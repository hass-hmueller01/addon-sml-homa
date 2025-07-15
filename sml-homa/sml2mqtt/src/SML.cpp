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

#include "SML.h"

/* C includes */
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

/* C++ includes */
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

/* SML library */
#include <sml/sml_file.h>
#include <sml/sml_transport.h>
#include <sml/sml_value.h>

/* project internal includes */
#include "MqttClient.h"

/** units */
static const std::map<uint8_t, std::string> units = {
    // code, unit           // Quantity                                     Unit name               SI definition (comment)
    //=====================================================================================================================
    {1, "a"},               // time                                         year                    52*7*24*60*60 s
    {2, "mo"},              // time                                         month                   31*24*60*60 s
    {3, "wk"},              // time                                         week                    7*24*60*60 s
    {4, "d"},               // time                                         day                     24*60*60 s
    {5, "h"},               // time                                         hour                    60*60 s
    {6, "min."},            // time                                         min                     60 s
    {7, "s"},               // time (t)                                     second                  s
    {8, "°"},               // (phase) angle                                degree                  rad*180/π
    {9, "°C"},              // temperature (T)                              degree celsius          K-273.15
    {10, "currency"},       // (local) currency
    {11, "m"},              // length (l)                                   metre                   m
    {12, "m/s"},            // speed (v)                                    metre per second        m/s
    {13, "m³"},             // volume (V)                                   cubic metre             m³
    {14, "m³"},             // corrected volume                             cubic metre             m³
    {15, "m³/h"},           // volume flux                                  cubic metre per hour    m³/(60*60s)
    {16, "m³/h"},           // corrected volume flux                        cubic metre per hour    m³/(60*60s)
    {17, "m³/d"},           // volume flux                                                          m³/(24*60*60s)
    {18, "m³/d"},           // corrected volume flux                                                m³/(24*60*60s)
    {19, "l"},              // volume                                       litre                   10-3 m³
    {20, "kg"},             // mass (m)                                     kilogram
    {21, "N"},              // force (F)                                    newton
    {22, "Nm"},             // energy                                       newtonmeter             J = Nm = Ws
    {23, "Pa"},             // pressure (p)                                 pascal                  N/m²
    {24, "bar"},            // pressure (p)                                 bar                     10⁵ N/m²
    {25, "J"},              // energy                                       joule                   J = Nm = Ws
    {26, "J/h"},            // thermal power                                joule per hour          J/(60*60s)
    {27, "W"},              // active power (P)                             watt                    W = J/s
    {28, "VA"},             // apparent power (S)                           volt-ampere
    {29, "var"},            // reactive power (Q)                           var
    {30, "Wh"},             // active energy                                watt-hour               W*(60*60s)
    {31, "VAh"},            // apparent energy                              volt-ampere-hour        VA*(60*60s)
    {32, "varh"},           // reactive energy                              var-hour                var*(60*60s)
    {33, "A"},              // current (I)                                  ampere                  A
    {34, "C"},              // electrical charge (Q)                        coulomb                 C = As
    {35, "V"},              // voltage (U)                                  volt                    V
    {36, "V/m"},            // electr. field strength (E)                   volt per metre
    {37, "F"},              // capacitance (C)                              farad                   C/V = As/V
    {38, "Ω"},              // resistance (R)                               ohm                     Ω = V/A
    {39, "Ωm²/m"},          // resistivity (ρ)                              Ωm
    {40, "Wb"},             // magnetic flux (Φ)                            weber                   Wb = Vs
    {41, "T"},              // magnetic flux density (B)                    tesla                   Wb/m2
    {42, "A/m"},            // magnetic field strength (H)                  ampere per metre        A/m
    {43, "H"},              // inductance (L)                               henry                   H = Wb/A
    {44, "Hz"},             // frequency (f, ω)                             hertz                   1/s
    {45, "1/(Wh)"},         // R_W                                                                  (Active energy meter constant or pulse value)
    {46, "1/(varh)"},       // R_B                                                                  (reactive energy meter constant or pulse value)
    {47, "1/(VAh)"},        // R_S                                                                  (apparent energy meter constant or pulse value)
    {48, "V²h"},            // volt-squared hour                            volt-squaredhours       V²(60*60s)
    {49, "A²h"},            // ampere-squared hour                          ampere-squaredhours     A²(60*60s)
    {50, "kg/s"},           // mass flux                                    kilogram per second     kg/s
    {51, "S, mho"},         // conductance siemens                                                  1/Ω
    {52, "K"},              // temperature (T)                              kelvin
    {53, "1/(V²h)"},        // R_U²h                                                                (Volt-squared hour meter constant or pulse value)
    {54, "1/(A²h)"},        // R_I²h                                                                (Ampere-squared hour meter constant or pulse value)
    {55, "1/m³"},           // R_V, meter constant or pulse value (volume)
    {56, "%"},              // percentage                                   %
    {57, "Ah"},             // ampere-hours                                 ampere-hour
    {60, "Wh/m³"},          // energy per volume                                                    3,6*103 J/m³
    {61, "J/m³"},           // calorific value, wobbe
    {62, "Mol %"},          // molar fraction of gas composition            mole percent            (Basic gas composition unit)
    {63, "g/m³"},           // mass density, quantity of material                                   (Gas analysis, accompanying elements)
    {64, "Pa s"},           // dynamic viscosity pascal second                                      (Characteristic of gas stream)
    {253, "(reserved)"},    // reserved
    {254, "(other)"},       // other unit
    {255, "(unitless)"}
};

SML::SML(std::string device) :
    m_device(device),
    m_fd(-1)
{
    int bits;
    struct termios config;
    memset(&config, 0, sizeof(config));

    m_fd = open(m_device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_fd < 0) {
        std::cerr << "open(" << device << "): " << strerror(errno) << std::endl;
    }

    // set RTS
    ioctl(m_fd, TIOCMGET, &bits);
    bits |= TIOCM_RTS;
    ioctl(m_fd, TIOCMSET, &bits);

    tcgetattr(m_fd, &config);

    // set 8-N-1
    config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    config.c_oflag &= ~OPOST;
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    config.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB);
    config.c_cflag |= CS8;

    // set speed to 9600 baud
    cfsetispeed(&config, B9600);
    cfsetospeed(&config, B9600);

    tcsetattr(m_fd, TCSANOW, &config);
}

SML::~SML()
{
    close(m_fd);
}

bool SML::is_open() const
{
    return (m_fd > 0);
}

void SML::transport_listen()
{
    sml_transport_listen(m_fd, [](unsigned char * buffer, size_t buffer_len) {
        /* check if MQTT client is available */
        if (!mqttClient()) {
            return;
        }

        /* the buffer contains the whole message and strip transport escape sequences */
        sml_file *file = sml_file_parse(buffer + 8, buffer_len - 16);

        /* read OBIS data */
        for (int i = 0; i < file->messages_len; i++) {
            sml_message *message = file->messages[i];
            if (*message->message_body->tag == SML_MESSAGE_GET_LIST_RESPONSE) {
                sml_list *entry;
                sml_get_list_response *body;
                body = (sml_get_list_response *) message->message_body->data;
                for (entry = body->val_list; entry != NULL; entry = entry->next) {
                    /* check if valid */
                    if (!entry->value) {
                        std::cerr << "Error in data stream. entry->value should not be NULL. Skipping this." << std::endl;
                        continue;
                    }

                    /* set OBIS string */
                    std::ostringstream obis;
                    obis
                            << static_cast<int>(entry->obj_name->str[0])
                            << "-"
                            << static_cast<int>(entry->obj_name->str[1])
                            << ":"
                            << static_cast<int>(entry->obj_name->str[2])
                            << "."
                            << static_cast<int>(entry->obj_name->str[3])
                            << "."
                            << static_cast<int>(entry->obj_name->str[4])
                            << "*"
                            << static_cast<int>(entry->obj_name->str[5]);

                    /* set MQTT value based on type */
                    if (entry->value->type == SML_TYPE_OCTET_STRING) {
                        char *str;
                        //mqttClient()->setTopic(topic.str(), sml_value_to_strhex(entry->value, &str, true));
                        free(str);
                    } else
                    if (entry->value->type == SML_TYPE_BOOLEAN) {
                        //mqttClient()->setTopic(topic.str(), (entry->value->data.boolean ? "1" : "0"));
                    } else
                    if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
                               ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED)) {
                        double value = sml_value_to_double(entry->value);
                        int scaler = (entry->scaler) ? *entry->scaler : 0;
                        value = value * pow(10, scaler);
                        /*
                            {'obis': '1-0:16.7.0*255', 'scale': 1, 'unit': ' W', 'topic': 'Current Power'},
                        	{'obis': '1-0:1.8.0*255', 'scale': 1000, 'unit': ' kWh', 'topic': 'Total Energy'}
                        */
                        if (obis.str() == "1-0:16.7.0*255") {
                            std::ostringstream valuestr;
                            valuestr << std::fixed << std::setprecision(1) << value;
                        	mqttClient()->setTopic("Current Power", valuestr.str());
                        } else if (obis.str() == "1-0:1.8.0*255") {
                            value = value / 1000;
                            std::ostringstream valuestr;
                            valuestr << std::fixed << std::setprecision(1) << value;
                        	mqttClient()->setTopic("Total Energy", valuestr.str());
                        }

                        /* unit is optional */
                        if (entry->unit) {
                            uint8_t code = (uint8_t) * entry->unit;
                            if (units.count(code)) {
                                //mqttClient()->setTopic(topic.str() + "/$unit", units.at(code));
                            }
                        }
                    }
                }
            }
        }

        /* free memory */
        sml_file_free(file);
    });
}
