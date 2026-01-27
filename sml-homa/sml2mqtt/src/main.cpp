/*
 * Holger Mueller
 * 2018/11/09
 * Modified from Tobias Lorenz to send HomA framework MQTT messages
 * based on specific OBIS data.
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

/* C includes */
#include <unistd.h>
#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

/* C++ includes */
#include <mosquittopp.h>
#include <yaml-cpp/yaml.h>

#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

/* project internal includes */
#include "Logger.h"
#include "MqttClient.h"
#include "SML.h"

/** abort the main loop */
std::atomic<bool> abortLoop;

/** handle SIGTERM */
void signalHandler(int /*signum*/) {
    abortLoop = true;
}

/** main function */
int main(int argc, char** argv) {
    /* default parameters */
    std::string host = "localhost";
    int port = 1883;
    int qos = 1;
    bool verbose = false;
    std::string topic = "/devices/123456-energy/controls";
    std::string id = "sml2mqtt";
    std::string username = "";
    std::string password = "";
    std::string device = "/dev/vzir0";
    YAML::Node config;

    /* evaluate command line parameters */
    int c;
    while ((c = getopt(argc, argv, "c:h:p:q:t:i:u:P:d:v?")) != -1) {
        switch (c) {
        case 'c':
            config = YAML::LoadFile(optarg);
            if (config["host"]) {
                host = config["host"].as<std::string>();
                if (verbose) Logger::info("Using yaml config host: " + host);
            }
            if (config["port"]) {
                port = config["port"].as<int>();
                if (verbose) Logger::info("Using yaml config port: " + std::to_string(port));
            }
            if (config["qos"]) {
                qos = config["qos"].as<int>();
                if (verbose) Logger::info("Using yaml config qos: " + std::to_string(qos));
            }
            if (config["topic"]) {
                topic = config["topic"].as<std::string>();
                if (verbose) Logger::info("Using yaml config topic: " + topic);
            }
            if (config["id"]) {
                id = config["id"].as<std::string>();
                if (verbose) Logger::info("Using yaml config id: " + id);
            }
            if (config["username"]) {
                username = config["username"].as<std::string>();
                if (verbose) Logger::info("Using yaml config username: " + username);
            }
            if (config["password"]) {
                password = config["password"].as<std::string>();
                if (verbose) Logger::info("Using yaml config password: " + password);
            }
            if (config["device"]) {
                device = config["device"].as<std::string>();
                if (verbose) Logger::info("Using yaml config device: " + device);
            }
            break;
        case 'h':
            host = optarg;
            if (verbose) Logger::info("Using command line config host: " + host);
            break;
        case 'p':
            port = std::stoul(optarg);
            if (verbose) Logger::info("Using command line config port: " + std::to_string(port));
            break;
        case 'q':
            qos = std::stoul(optarg);
            if (verbose) Logger::info("Using command line config qos: " + std::to_string(qos));
            break;
        case 't':
            topic = optarg;
            if (verbose) Logger::info("Using command line config topic: " + topic);
            break;
        case 'i':
            id = optarg;
            if (verbose) Logger::info("Using command line config id: " + id);
            break;
        case 'u':
            username = optarg;
            if (verbose) Logger::info("Using command line config username: " + username);
            break;
        case 'P':
            password = optarg;
            if (verbose) Logger::info("Using command line config password: " + password);
            break;
        case 'd':
            device = optarg;
            if (verbose) Logger::info("Using command line config device: " + device);
            break;
        case 'v':
            verbose = true;
            break;
        default:
            std::cout << "Usage: sml2mqtt [-v] [-c config.yaml] [-h host] [-p port] [-q qos] [-t topic] [-i id] [-u username] [-P password] [-d device]" << std::endl
                      << "-v: Be verbose, use this first to get all verbose messages" << std::endl
                      << "-c: Use YAML config file <config.yaml> (can be combined with other options)" << std::endl
                      << "-h: hostname of broker" << std::endl
                      << "-p: port of broker" << std::endl
                      << "-q: QOS of messages" << std::endl
                      << "-t: MQTT topic to publish to (e.g. /devices/123456-energy/controls)" << std::endl
                      << "-i: ID of broker client (e.g. sml2mqtt)" << std::endl
                      << "-u: username" << std::endl
                      << "-p: password" << std::endl
                      << "-d: device to read sml messages from (e.g. /dev/vzir0)" << std::endl;
            return EXIT_FAILURE;
        }
    }

    /* register signal handler */
    abortLoop = false;
    signal(SIGTERM, signalHandler);

    /* mosquitto constructor */
    if (mosqpp::lib_init() != MOSQ_ERR_SUCCESS) {
        Logger::error("main: lib_init failed");
        return EXIT_FAILURE;
    }

    /* start MqttClient */
    mqttClient() = new MqttClient(host.c_str(), port, qos, topic.c_str(), id.c_str(), username.c_str(), password.c_str(), verbose);

    // check if MQTT client is available
    if (!mqttClient()) {
        return EXIT_FAILURE;
    }

    // setup HomA meta data
    /*
        {'obis': '1-0:16.7.0*255', 'scale': 1, 'unit': ' W', 'topic': 'Current Power'},
        {'obis': '1-0:1.8.0*255', 'scale': 1000, 'unit': ' kWh', 'topic': 'Total Energy'}
        {'obis': '1-0:2.8.0*255', 'scale': 1000, 'unit': ' kWh', 'topic': 'Total Energy Feed-in'}
    */
    mqttClient()->publishOnChange(TOPIC_POWER "/meta/type", "text");
    mqttClient()->publishOnChange(TOPIC_POWER "/meta/unit", " W");
    mqttClient()->publishOnChange(TOPIC_POWER "/meta/order", "1");
    mqttClient()->publishOnChange(TOPIC_ENERGY "/meta/type", "text");
    mqttClient()->publishOnChange(TOPIC_ENERGY "/meta/unit", " kWh");
    mqttClient()->publishOnChange(TOPIC_ENERGY "/meta/order", "2");
    mqttClient()->publishOnChange(TOPIC_FEED_IN "/meta/type", "text");
    mqttClient()->publishOnChange(TOPIC_FEED_IN "/meta/unit", " kWh");
    mqttClient()->publishOnChange(TOPIC_FEED_IN "/meta/order", "2");

#ifdef WITH_SYSTEMD
    /* systemd notify */
    sd_notify(0, "READY=1");
#endif

    /* start publish loop */
    while (!abortLoop) {
#ifdef WITH_SYSTEMD
        /* systemd notify */
        sd_notify(0, "WATCHDOG=1");
#endif
        // init serial channel
        SML sml(device);
        if (!sml.is_open()) {
            Logger::error("main: sml.is_open() failed. Exit.");
            return EXIT_FAILURE;
        }

        // listen on the serial device read channels and publish via MQTT, this call is blocking
        sml.transport_listen();
        if (!abortLoop) {
            Logger::error("main: sml.transport_listen() ended unexpectedly, retrying in 20s ...");
            std::this_thread::sleep_for(std::chrono::seconds(20));
        }
    }

    /* delete resources */
    delete mqttClient();

    /* mosquitto destructor */
    if (mosqpp::lib_cleanup() != MOSQ_ERR_SUCCESS) {
        Logger::error("main: lib_cleanup failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
