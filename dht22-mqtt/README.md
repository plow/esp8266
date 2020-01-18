# DHT22 to MQTT

Periodically measures temperature and humidity and send the results to an MQTT topic. Between two measurements the ESP8266 goes to deepsleep.

## Build

### Create Config
Copy and rename all `settings-*.h.default` files to `settings-*.h` and adjust the settings acording to your setup. Since `*/settings-*.h` files are ignored by `git` you don't run the risk of pushing secrets to the repo.

### Library Requirements
Make sure the following libraries are installed (including all their dependencies):

* `DHT sensor library` by `Adafruit`: Arduino library for DHT11, DHT22, etc Temperature & Humidity Sensors
* `PubSubClient` by `Nick O'Leary`: Arduino client for MQTT

