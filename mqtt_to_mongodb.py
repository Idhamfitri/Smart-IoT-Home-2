import json
import pymongo
import paho.mqtt.client as mqtt
from datetime import datetime, timezone

# ================= MONGODB CONFIG =================
mongo_client = pymongo.MongoClient("mongodb://localhost:27017/")
db = mongo_client["smarthome"]
collection = db["iot_data"]

# ================= MQTT CONFIG =================
MQTT_BROKER = "localhost"
MQTT_PORT   = 1883
MQTT_TOPIC  = "smarthome/data"

# ================= MQTT CALLBACKS =================
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Connected successfully")
        client.subscribe(MQTT_TOPIC)
    else:
        print("[MQTT] Connection failed, code:", rc)

def on_message(client, userdata, msg):
    print("[MQTT] Message received:", msg.payload.decode())

    try:
        data = json.loads(msg.payload.decode())

        document = {
            "timestamp": datetime.now(timezone.utc),
            "presence": bool(data.get("presence")),
            "temperature": float(data.get("temperature")),
            "air_quality": int(data.get("air_quality")),
            "fan_status": data.get("fan_status")
        }

        collection.insert_one(document)
        print("[MongoDB] Data inserted")

    except Exception as e:
        print("[ERROR]", e)

# ================= MAIN =================
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)

try:
    client.loop_forever()
except KeyboardInterrupt:
    print("Stopping subscriber...")
    client.disconnect()
