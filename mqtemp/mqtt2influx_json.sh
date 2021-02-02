#!/bin/bash

# Bridge from MQTT to InfluxDB when the data is json (needs jq):
# topic -> {"ontime":517,"temp":15.938}

TOPIC=/mqtemp/data
DATABASE=mqtemp
MEASURE=mqtemp

while read -r line
do
    echo "$line"

    ontime=`echo "$line" | jq ".ontime"`
    temp=`echo "$line" | jq ".temp"`

    curl "http://localhost:8086/write?db=$DATABASE" \
        -s \
        -q \
        -XPOST \
        --data-binary "$MEASURE ontime=$ontime,temp=$temp" > /dev/null

done < <(mosquitto_sub -t $TOPIC)
