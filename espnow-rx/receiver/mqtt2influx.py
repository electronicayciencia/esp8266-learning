#!/usr/bin/env python3

import os
import logging
import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient
from influxdb_client.client.write_api import SYNCHRONOUS

FLUX_BUCKET="data"
FLUX_MEASURE="espnow"
FLUX_ORG="electronicayciencia@gmail.com"
FLUX_URL="https://us-east-1-1.aws.cloud2.influxdata.com"
FLUX_TOKEN=os.environ['INFLUXDB_W_TOKEN']

MQTT_SERVER="localhost"
MQTT_TOPIC="espnow-pub"
MQTT_CLIENTID="m2flux-py"

REJECTFILE="rejected.dat"
LOGLEVEL="INFO"


def on_connect(client, userdata, flags, rc):
    client.subscribe(MQTT_TOPIC, 2)
    logging.info("Connected. Id: {}. Topic: {}.".format(MQTT_CLIENTID, MQTT_TOPIC))


def on_message(client, userdata, msg):
    """Receive and process new messages."""
    text = msg.payload.decode("utf-8") # payload is a byte array
    logging.debug("Received message ({}): {}".format(msg.topic, text))

    try:
        line = lineformat(text)
    except:
        logging.warning("Unexpected message format!: " + text)
        return

    try:
        send_to_influx([line])
        logging.info("Message sent to influxDB: " + text)
    except:
        store_as_rejected(line)
        logging.warning("Message stored as rejected: " + text)


def lineformat(text):
    """Process one message into lineformat."""
    (mac, ns, msg) = text.split(" ", 2)
    #TODO: check if msg is valid for influxdb
    line = "{},mac={} {} {}".format(FLUX_MEASURE, mac, msg, ns)
    return line


def send_to_influx(lines):
    """Send content to influx. Content maybe one or more lines. Rise exception on failure."""
    if len(lines) == 1:
        logging.debug("Sending: " + lines[0])
    else:
        logging.debug("Sending {} lines".format(len(lines)))

    flux_client = InfluxDBClient(url=FLUX_URL, token=FLUX_TOKEN, org=FLUX_ORG)
    write_api = flux_client.write_api(write_options=SYNCHRONOUS)
    write_api.write(FLUX_BUCKET, FLUX_ORG, lines)


def store_as_rejected(line):
    """Store rejected line into a file."""
    logging.debug("Storing as rejected: " + line)
    with open(REJECTFILE, 'a') as f1:
        f1.write(line + os.linesep)


def send_rejecteds():
    """Try to send again rejected lineformats."""
    if not os.path.exists(REJECTFILE) or os.path.getsize(REJECTFILE) <= 0:
        logging.debug("No rejected messages found.")
        return

    logging.info("Trying to re-send rejected messages.")
    
    with open(REJECTFILE, 'r') as f1:
         lines = [i.rstrip() for i in f1.readlines()]

    try:
        send_to_influx(lines)
    except Exception as err:
        logging.warning("Re-sending rejected messages failed: {}".format(err))
        return

    os.remove(REJECTFILE)
    logging.info("Rejected messages sent.")


def init_mqtt():
    client = mqtt.Client(client_id=MQTT_CLIENTID, clean_session=False)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_SERVER)
    return client


logging.basicConfig(
        format='%(asctime)s [%(levelname)s] %(message)s',
        level=LOGLEVEL)
mqtt_client = init_mqtt()

while True:
    mqtt_client.loop(10)
    send_rejecteds()

