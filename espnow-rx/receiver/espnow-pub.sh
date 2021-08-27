#!/usr/bin/bash

# Generic ESPNOW MQTT publisher
# Message published format is:
# MAC DATENS MSG
#  MAC: originating device
#  DATENS: timestamp with ns
#  MSG: incoming message

SERIAL=/dev/ttyS0
BAUDRATE=115200
TOPIC=espnow-pub

stty -F $SERIAL $BAUDRATE raw
exec 3<$SERIAL

echo "ESPNOW Publisher. Started at $(date)."
echo "Topic: $TOPIC"

while read -u 3 mark mac len msg
do 
	datens=$(date +%s%N)
	pubmsg="$mac $datens $msg"

  mosquitto_pub -i espnowpub -t $TOPIC -q 2 -m "$pubmsg"
  echo -n .
done

