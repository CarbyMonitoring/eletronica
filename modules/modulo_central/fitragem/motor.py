import random
import board
import busio

import adafruit_mcp4725
from simple_pid import PID

from utils import connect_mqtt

broker = 'broker-url'
port = 1883
topic = "esp32/co2"
client_id = f'modulo-central-filtragem-{random.randint(0, 1000)}'

i2c = busio.I2C(board.SCL, board.SDA)
dac = adafruit_mcp4725.MCP4725(i2c)

def on_message(client, userdata, message):
  concentracaoCo2 = int(str(message.payload.decode("utf-8")))
  tensaoDac = pid(concentracaoCo2) 
  dac.write(tensaoDac)

pid = PID(1, 0.1, 0)
pid.output_limits = (0, 4095)

client = connect_mqtt(client_id, broker, port)

client.subscribe(topic)

client.on_message = on_message

client.loop_forever(1)


  