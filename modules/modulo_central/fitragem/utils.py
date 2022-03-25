from paho.mqtt import client as mqtt_client


def connect_mqtt(_client_id, _broker, _port):
  client = mqtt_client.Client(_client_id)
  client.connect(_broker, _port)
  return client