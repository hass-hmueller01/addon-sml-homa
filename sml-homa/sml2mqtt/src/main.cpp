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
#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mosquittopp.h>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <yaml-cpp/yaml.h>

/* project internal includes */
#include "SML.h"
#include "MqttClient.h"

/** abort the main loop */
std::atomic<bool> abortLoop;

/** handle SIGTERM */
void signalHandler(int /*signum*/)
{
    abortLoop = true;
}

/** main function */
int main(int argc, char ** argv)
{
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
                if (verbose) std::cout << "Using yaml config host: " << host << std::endl;
            }
            if (config["port"]) {
                port = config["port"].as<int>();
                if (verbose) std::cout << "Using yaml config port: " << port << std::endl;
            }
            if (config["qos"]) {
                qos = config["qos"].as<int>();
                if (verbose) std::cout << "Using yaml config qos: " << qos << std::endl;
            }
            if (config["topic"]) {
                topic = config["topic"].as<std::string>();
                if (verbose) std::cout << "Using yaml config topic: " << topic << std::endl;
            }
            if (config["id"]) {
                id = config["id"].as<std::string>();
                if (verbose) std::cout << "Using yaml config id: " << id << std::endl;
            }
            if (config["username"]) {
                username = config["username"].as<std::string>();
                if (verbose) std::cout << "Using yaml config username: " << username << std::endl;
            }
            if (config["password"]) {
                password = config["password"].as<std::string>();
                if (verbose) std::cout << "Using yaml config password: " << password << std::endl;
            }
            if (config["device"]) {
                device = config["device"].as<std::string>();
                if (verbose) std::cout << "Using yaml config device: " << device << std::endl;
            }
            break;
        case 'h':
            host = optarg;
            if (verbose) std::cout << "Using command line config host: " << host << std::endl;
            break;
        case 'p':
            port = std::stoul(optarg);
            if (verbose) std::cout << "Using command line config port: " << port << std::endl;
            break;
        case 'q':
            qos = std::stoul(optarg);
            if (verbose) std::cout << "Using command line config qos: " << qos << std::endl;
            break;
        case 't':
            topic = optarg;
            if (verbose) std::cout << "Using command line config topic: " << topic << std::endl;
            break;
        case 'i':
            id = optarg;
            if (verbose) std::cout << "Using command line config id: " << id << std::endl;
            break;
        case 'u':
            username = optarg;
            if (verbose) std::cout << "Using command line config username: " << username << std::endl;
            break;
        case 'P':
            password = optarg;
            if (verbose) std::cout << "Using command line config password: " << password << std::endl;
            break;
        case 'd':
            device = optarg;
            if (verbose) std::cout << "Using command line config device: " << device << std::endl;
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
        std::cerr << "main: lib_init failed" << std::endl;
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
    */
	mqttClient()->setTopic("Current Power/meta/type", "text");
	mqttClient()->setTopic("Current Power/meta/unit", " W");
	mqttClient()->setTopic("Current Power/meta/order", "1");
	mqttClient()->setTopic("Total Energy/meta/type", "text");
	mqttClient()->setTopic("Total Energy/meta/unit", " kWh");
	mqttClient()->setTopic("Total Energy/meta/order", "2");

    /* init all channels */
    SML sml(device);
    if (!sml.is_open()) {
        return -1;
    }

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

        /* read channels and publish via MQTT */
        // listen on the serial device, this call is blocking.
        sml.transport_listen();
    }

    /* delete resources */
    delete mqttClient();

    /* mosquitto destructor */
    if (mosqpp::lib_cleanup() != MOSQ_ERR_SUCCESS) {
        std::cerr << "main: lib_cleanup failed" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
