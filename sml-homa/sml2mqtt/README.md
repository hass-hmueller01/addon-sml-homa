# SML2MQTT
This application reads the current power and total energy by an IR connector (e.g. IR-Kopf by [volkszaehler.org](http://wiki.volkszaehler.org/)) and creates HomA framework's MQTT messages.

This is based on the work of [Tobias Lorenz](https://bitbucket.org/tobylorenz/sml2mqtt/overview).

## Installation
Install the required dependencies
```bash
$ apt-get install libmosquittopp-dev libyaml-cpp-dev
$ git clone https://github.com/hmueller01/libsml.git
$ cd libsml
$ make
$ sudo make install
$ cd ..
```
Build and install `sml2mqtt`
```bash
$ mkdir build; cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
$ make
$ sudo make install
```

## Usage
Start the application manually
```none
sml2mqtt [-v] [-c config.yaml] [-h host] [-p port] [-q qos] [-t topic] [-i id] [-u username] [-P password] [-d device]
```
You can eighter use the command line parameter or define some or all options in an `config.yaml`:
```yaml
# Config file of sml2mqtt
host: localhost
port: 1883
qos: 1
topic: /devices/123456-energy/controls
id: sml2mqtt
username: user
password: pass
device: /dev/vzir0
```

## Systemd
If your system supports it, you can start the application as a daemon from systemd by using the provided template.

Modify `/usr/local/lib/systemd/system/sml2mqtt.service` to your needs.
```bash
$ sudo systemctl --system daemon-reload
$ sudo systemctl stop sml2mqtt.service # optional
$ sudo systemctl start sml2mqtt.service
```

Logs are then available in the systemd journal and other commands
```bash
$ sudo journalctl -u sml2mqtt.service -n 100 -f
$ sudo systemctl status sml2mqtt.service
```
