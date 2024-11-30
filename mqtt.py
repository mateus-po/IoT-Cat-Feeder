import paho.mqtt.client as mqtt
import tkinter as tk
import threading

mqtt_broker = "" # Broker's IP address
mqtt_port = 1883
mqtt_user = "" # MQTT user which is used for authentication
mqtt_password = "" # MQTT password used for authenticating aforementioned user
mqtt_topic_weight = "" # Weight topic for specific ESP32 client
mqtt_topic_pressure = "" # Pressure topic for specific ESP32 client

root = tk.Tk()
root.title("MQTT Sensor Viewer")

weight_label = tk.Label(root, text="Waiting for weight data...", font=("Helvetica", 16), padx=20, pady=10)
weight_label.pack(padx=20, pady=10)

pressure_label = tk.Label(root, text="Waiting for pressure data...", font=("Helvetica", 16), padx=20, pady=10)
pressure_label.pack(padx=20, pady=10)

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")
    client.subscribe([(mqtt_topic_weight, 0), (mqtt_topic_pressure, 0)])

def on_message(client, userdata, msg):
    message = msg.payload.decode()
    
    if msg.topic == mqtt_topic_weight:
        print(f"Received weight: {message} kg")
        weight_label.config(text=f"Last Weight: {message} kg")
    elif msg.topic == mqtt_topic_pressure:
        print(f"Received pressure: {message} Pa")
        pressure_label.config(text=f"Last Pressure: {message} Pa")

client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_password)
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_broker, mqtt_port, 60)

def run_mqtt():
    client.loop_forever()

mqtt_thread = threading.Thread(target=run_mqtt, daemon=True)
mqtt_thread.start()

root.mainloop()
