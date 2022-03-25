
#!/usr/bin/env python3

import random
import serial, time
from time import localtime, strftime
from utils import connect_mqtt


port = serial.Serial("/dev/serial0", baudrate=9600, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=1.5)

broker = 'broker-url'
mqtt_port = 1883
topic = "rasp/pm10"
client_id = f'modulo-central-sensor-{random.randint(0, 1000)}'

def main():
    client = connect_mqtt(client_id, broker, mqtt_port)
    if port.isOpen():
        port.close()
    port.open()
    time.sleep(0.2)
    try:
        data = port.read(32);
        if ord(data[0]) == 66 and ord(data[1]) == 77:
            suma = 0
            for a in range(30):
                suma += ord(data[a])
            if suma == ord(data[30])*256+ord(data[31]):
                PM10 = int((ord(data[8])*256+ord(data[9]))/0.75)
                publish(client, f'PM10: {PM10} ug/m3', 2)
    except Exception as ex:
        print(ex)
    finally:
        port.close()

def publish(client, msg, tries=1):
    time.sleep(1)
    result = client.publish(topic, msg)
    # result: [0, 1]
    status = result[0]
    if (status == 0 and tries > 1):
        publish(client, msg, tries-1)


if __name__=="__main__":
    while(1):
        main()
        time.sleep(0.2)