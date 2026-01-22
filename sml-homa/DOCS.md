# Home Assistant add-on: SML to HomA MQTT

This add-on uses an IR connector (e.g. IR-Kopf by [volkszaehler.org](http://wiki.volkszaehler.org/)) to read the current power and total energy from a SML (Smart Message Language) meter and creates HomA framework's MQTT messages.

## How to use

Once installed, the add-on fetches data from your SML meter and pushes it to the HomA MQTT topics `/devices/<systemID>/controls/<topic>`.

The following [OBIS](https://de.wikipedia.org/wiki/OBIS-Kennzahlen) messages are supported:
| OBIS           | unit | topic                |
| :------------- | :--- | :------------------- |
| 1-0:16.7.0*255 | W    | Current Power        |
| 1-0:1.8.0*255  | kWh  | Total Energy         |
| 1-0:2.8.0*255  | kWh  | Total Energy Feed-in |

To get the data it uses the [libSML](https://github.com/volkszaehler/libsml) and the [SML2MQTT](https://github.com/hass-hmueller01/addon-sml-homa/tree/main/sml-homa/sml2mqtt) application.

## Configuration

In the configuration section, you can configure the USB/TTY device (e.g. `/dev/ttyUSB0`) or the IR devices UART serial number to detect the TTY automatically. If TTY is set, auto detect by serial number is deactivated. If you like to use the auto detect you have to remove the TTY setting.

At start it will scan all UART devices and output the information in the log.

```
INFO: Found UART device: 10c4:ea60 - CP2104 USB to UART Bridge Controller, S/N 12345678 at /dev/ttyUSB0
```

Here you can copy paste the serial number (remember to remove the TTY device). That way the add-on will find the correct device at startup no matter what `/dev/ttyUSBx` it gets.

The add-on supports the internal (Home Assistant) or an external MQTT broker. To configure the external MQTT broker you have to activate the _"Show unused optional configuration options"_.

You can modify the topic's `<systemID>` by setting _"HomA System ID"_.
