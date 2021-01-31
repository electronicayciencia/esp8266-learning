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
    create user grafana with password 'xxxxxx'
    grant all on mqtemp to grafana


Configure grafana datasource. http://raspberrypi.local:3000/

Use bash *process substitution* to make a bridge between mosquitto and influxdb:

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
