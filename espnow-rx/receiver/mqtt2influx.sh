#!/usr/bin/bash

TOPIC=espnow-pub

BUCKET="data"
ORG="electronicayciencia@gmail.com"
URL="https://us-east-1-1.aws.cloud2.influxdata.com"
TOKEN="$INFLUXDB_W_TOKEN"

ID=influx

#set -e
set -o pipefail

echo "InfluxDB logger. Starting at $(date)."
echo "Topic: $TOPIC, bucket: $BUCKET"

mosquitto_sub -i "$ID" -t "$TOPIC" -q 2 -c | \
while read mac ns msg
do
  lineformat="espnow,mac=$mac $msg $datens"

  curl -sS "$URL/api/v2/write?bucket=$BUCKET&org=$ORG" \
    --fail \
    -H "Authorization: Token $TOKEN" \
    -H 'Content-Type: text/plain' \
    --data "$lineformat" \
    || killall mosquitto_sub # ignores sigpipe
  echo -n .
done

