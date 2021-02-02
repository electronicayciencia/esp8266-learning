#!/bin/bash

# Bridge from MQTT to InfluxDB when the data is just a variable:
# topic -> 16.5

TOPIC=/mqtemp/data
DATABASE=mqtemp
MEASURE=mqtemp
VARNAME=temp

while read -r line
do
        echo "$line"
        curl -s -q -XPOST "http://localhost:8086/write?db=$DATABASE" --data-binary "$MEASURE $VARNAME=$line" > /dev/null
done < <(mosquitto_sub -t $TOPIC)

