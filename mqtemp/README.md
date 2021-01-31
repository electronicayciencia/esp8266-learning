# MQTEMP

Read the temperature from DS18B20 and report it via MQTT.

29/01/2021

To install influxdb in Raspberry see for example: 
https://www.circuits.dk/install-grafana-influxdb-raspberry/


    ### NOT starting on installation, please execute the following statements to configure grafana to start automatically using systemd
     sudo /bin/systemctl daemon-reload
     sudo /bin/systemctl enable grafana-server
    ### You can start grafana-server by executing
     sudo /bin/systemctl start grafana-server


Create database:

    $ influxdb
    create database mqtemp
    create user telegraf with password 'xxxxxx'
    create user grafana with password 'xxxxxx'
    grant all on mqtemp to telegraf
    grant all on mqtemp to grafana


Configure grafana datasource.

    $ apt-get install telegraf


```conf
[[inputs.mqtt_consumer]]
  servers = ["tcp://127.0.0.1:1883"]

  topics = [
     "mqtemp/data",
   ]
  data_format = "influx"

...

[[outputs.influxdb]]
  ## The full HTTP or UDP URL for your InfluxDB instance.
  urls = ["http://127.0.0.1:8086"]

  database = "mqtemp"
  skip_database_creation = true
  username = "telegraf"
  password = "xxxx"

```

Restart telegraf: `service telegraf status`

Check the status using `service telegraf status`.

If errors `grep telegraf /var/log/syslog | tail`.

```
> show measurements
name: measurements
name
----
cpu
disk
diskio
kernel
mem
processes
swap
system
```

mqtemp is not there, but a lot of garbage. Removing telegraf right now.

Testing https://github.com/Nilhcem/home-monitoring-grafana/blob/master/02-bridge/main.py

Too complicated, use bash *process substitution* to make a bridge:

```bash
#!/bin/bash
TOPIC=/mqtemp/data
DATABASE=mqtemp

while read -r line
do
    echo "$line"
    curl -s -q -XPOST "http://localhost:8086/write?db=$DATABASE" --data-binary "$line" > /dev/null
done < <(mosquitto_sub -t $TOPIC)
```

Inside influx:

```
pi@raspberrypi:~/influxdb$ influx
Connected to http://localhost:8086 version 1.8.3
InfluxDB shell version: 1.8.3
> use mqtemp
Using database mqtemp
> show measurements
name: measurements
name
----
mqtemp
> select * from mqtemp
name: mqtemp
time                ontime temp
----                ------ ----
1612118997004359120 3178   19.375
1612119179972478267 3180   19.375
1612119188490909254 3180   19.375
```
