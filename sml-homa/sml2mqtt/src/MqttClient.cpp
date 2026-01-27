/*
 * Holger Mueller
 * 2018/11/09
 * Modified from Tobias Lorenz to send HomA framework MQTT messages
 *
 * Copyright (C) 2018 Tobias Lorenz.
 * Contact: tobias.lorenz@gmx.net
 *
 * This file is part of Tobias Lorenz's Toolkit.
 *
 * Commercial License Usage
 * Licensees holding valid commercial licenses may use this file in
 * accordance with the commercial license agreement provided with the
 * Software or, alternatively, in accordance with the terms contained in
 * a written agreement between you and Tobias Lorenz.
 *
 * GNU General Public License 3.0 Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl.html.
 */

#include "MqttClient.h"
#include "Logger.h"

/* C++ includes */
#include <iostream>
#include <string>

MqttClient::MqttClient(const char *host, int port,
                       int qos, const char *baseTopic, const char *id,
                       const char *username, const char *password,
                       bool verbose) : mosqpp::mosquittopp(id),
                                       m_verbose(verbose),
                                       m_qos(qos),
                                       m_baseTopic(baseTopic),
                                       m_topicPayloads(),
                                       m_topicPayloadsMutex() {
    /* set last will */
    /*
    std::string topic = m_baseTopic + "/$state";
    std::string payload = "lost";
    if (will_set(topic.c_str(), payload.length(), payload.c_str(), m_qos, true) != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::MqttClient: will_set failed");
    }
    */

    /* username/password */
    if (username_pw_set(username, password) != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::MqttClient: username_pw_set failed");
    }

    /* connect */
    int rc = connect_async(host, port);
    if (rc != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::MqttClient: connect_async failed (" + std::string(mosqpp::strerror(rc)) + ")");
    }
    if (loop_start() != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::MqttClient: loop_start failed");
    }
}

MqttClient::~MqttClient() {
    /* disconnect */
    /*
    std::string topic = m_baseTopic + "/$state";
    std::string payload = "disconnected";
    if (publish(nullptr, topic.c_str(), payload.length(), payload.c_str(), m_qos, true) != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::~MqttClient: publish failed");
    }
    */
    if (disconnect() != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::~MqttClient: disconnect failed");
    }
    if (loop_stop() != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::~MqttClient: loop_stop failed");
    }
}

void MqttClient::publishOnChange(std::string topic, std::string payload, bool retain) {
    std::lock_guard<std::mutex> lock(m_topicPayloadsMutex);

    /* check if value has changed */
    if (m_topicPayloads[topic] == payload) {
        return;
    }
    m_topicPayloads[topic] = payload;

    /* publish */
    topic = m_baseTopic + "/" + topic;
    if (m_verbose) {
        Logger::info(topic + " set to " + payload);
    }
    int rc = publish(nullptr, topic.c_str(), payload.length(), payload.c_str(), m_qos, retain);
    if (rc != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::publishOnChange: publish " + topic + " failed rc=" + std::to_string(rc) + " (" + mosqpp::strerror(rc) + ")");
    }
}

std::string MqttClient::getTopic(std::string topic, std::string defaultValue) const {
    std::lock_guard<std::mutex> lock(m_topicPayloadsMutex);

    try {
        return m_topicPayloads.at(topic);
    } catch (std::out_of_range &e) {
        return defaultValue;
    }
}

void MqttClient::on_connect(int rc) {
    std::string topic;
    std::string payload;

    if (rc != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::on_connect(" + std::to_string(rc) + ")");
    } else {
        /* publish $state = init */
        /* not used
        topic = m_baseTopic + "/$state";
        payload = "init";
        if (publish(nullptr, topic.c_str(), payload.length(), payload.c_str(), m_qos, true) != MOSQ_ERR_SUCCESS) {
            Logger::error("MqttClient::on_connect: publish('" + topic + "', '" + payload + "') failed");
        }
        */

        /* publish $name */
        /* not used by HomA
        topic = m_baseTopic + "/$name";
        payload = "SML";
        if (publish(nullptr, topic.c_str(), payload.length(), payload.c_str(), m_qos, true) != MOSQ_ERR_SUCCESS) {
            Logger::error("MqttClient::on_connect: publish('" + topic + "', '" + payload + "') failed");
        }
        */

        /* subscribe */
        /* not used by HomA
        topic = m_baseTopic + "/" + m_subscribeTopic;
        if (subscribe(nullptr, topic.c_str(), m_qos) != MOSQ_ERR_SUCCESS) {
            Logger::error("MqttClient::on_connect: subscribe failed");
        }
        */

        /* publish $state = ready */
        /* not used
        topic = m_baseTopic + "/$state";
        payload = "ready";
        if (publish(nullptr, topic.c_str(), payload.length(), payload.c_str(), m_qos, true) != MOSQ_ERR_SUCCESS) {
            Logger::error("MqttClient::on_connect: publish('" + topic + "', '" + payload + "') failed");
        }
        */
    }
}

void MqttClient::on_message(const struct mosquitto_message *message) {
    std::lock_guard<std::mutex> lock(m_topicPayloadsMutex);

    /* remove basetopic from topic */
    std::string topic = message->topic;
    topic.erase(0, m_baseTopic.length() + 1);

    /* save it */
    std::string payload(static_cast<const char *>(message->payload), message->payloadlen);
    m_topicPayloads[topic] = payload;
}

void MqttClient::on_disconnect(int rc) {
    if (rc != MOSQ_ERR_SUCCESS) {
        Logger::error("MqttClient::on_disconnect: Unexpected disconnect from broker. Reason code: " + std::to_string(rc));
    } else if (m_verbose) {
        Logger::info("MqttClient::on_disconnect: disconnected");
    }
}

MqttClient *&mqttClient() {
    static MqttClient *mqttClient = nullptr;
    return mqttClient;
}
