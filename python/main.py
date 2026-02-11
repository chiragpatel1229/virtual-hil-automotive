# main.py
# This is the main program that runs everything:
# 1. listen to UDP data from gateway
# 2. learn what normal looks like
# 3. watch live and look for problems
# 4. save everything so we can look at it later

import socket
import time
import warnings
import numpy as np
import pandas as pd

# Our own modules
from config import *
from can_parser import parse_can_frame
from ai_model import train_ai_model
from monitor import explain_anomaly, ai_recommendation

# Hide some boring warnings from libraries
warnings.filterwarnings("ignore")

# Create UDP socket to listen for data from gateway
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
print(f"AI Monitor started - listening on {UDP_IP}:{UDP_PORT}")

# =============================================
# Phase 1: Learn normal behavior (training)
# =============================================
print("\nPhase 1: Collecting normal data to learn from...")
print(f"Need {TRAINING_SAMPLES} good measurements...")

training_features = []
voltage_window = []         # will keep last few voltages to calculate noise
prev_voltage = None

while len(training_features) < TRAINING_SAMPLES:
    data, addr = sock.recvfrom(1024)
    
    if len(data) != 13:
        continue  # skip bad packets

    # read the values from the packet
    _, voltage, temp, _ = parse_can_frame(data)

    if prev_voltage is not None:
        delta_v = voltage - prev_voltage
        
        # keep only the last WINDOW_SIZE voltages
        voltage_window.append(voltage)
        if len(voltage_window) > WINDOW_SIZE:
            voltage_window.pop(0)
        
        # calculate how noisy the signal is right now
        noise_std = np.std(voltage_window) if len(voltage_window) > 1 else 0.0
        
        # save this example for training
        training_features.append([voltage, delta_v, noise_std, temp])

    prev_voltage = voltage

print(f"Collected {len(training_features)} examples → training done")

# =============================================
# Phase 2: Train the AI model
# =============================================
print("\nPhase 2: Training AI model...")
print("\n... Please wait for few second: ~20 Sec. - Collecting 200 Samples")
model, baseline, df_training = train_ai_model(training_features, CONTAMINATION)
df_training.to_csv("training_data.csv", index=False)
print("Model ready! Normal behavior learned.")

# =============================================
# Phase 3: Live monitoring & anomaly detection
# =============================================
print("\nPhase 3: Starting live monitoring...")
print("Press Ctrl+C to stop early")

data_log = []
voltage_window = []         # reset window for live phase
prev_voltage = None
anomaly_history = []        # remember last few anomaly decisions

start_time = time.time()

try:
    while time.time() - start_time < COLLECTION_TIME:
        data, _ = sock.recvfrom(1024)
        if len(data) != 13:
            continue

        _, voltage, temp, _ = parse_can_frame(data)

        if prev_voltage is not None:
            delta_v = voltage - prev_voltage

            voltage_window.append(voltage)
            if len(voltage_window) > WINDOW_SIZE:
                voltage_window.pop(0)

            noise_std = np.std(voltage_window) if len(voltage_window) > 1 else 0.0

            # ask the model: is this normal or strange?
            sample = [[voltage, delta_v, noise_std, temp]]
            is_anomaly = model.predict(sample)[0] == -1   # -1 means anomaly

            # keep history to avoid single false alarms
            anomaly_history.append(is_anomaly)
            if len(anomaly_history) > ANOMALY_WINDOW:
                anomaly_history.pop(0)

            # only alert if anomaly appears multiple times
            if sum(anomaly_history) >= ANOMALY_THRESHOLD:
                reason = explain_anomaly(voltage, delta_v, noise_std, temp, baseline)
                action = ai_recommendation(reason)
                print(f"[ALERT!] {reason} → {action}")
                print(f"   Voltage = {voltage} mV  Noise={noise_std:5.2f}  Temp = {temp} °C")
            else:
                print(f"OK - V={voltage:4d} mV  ΔV={delta_v:+4d}  Noise={noise_std:5.2f}")

            # save everything for later plotting / analysis
            data_log.append({
                "Time": time.time() - start_time,
                "Voltage": voltage,
                "DeltaVoltage": delta_v,
                "NoiseStd": noise_std,
                "Temperature": temp,
                "Anomaly": is_anomaly
            })

        prev_voltage = voltage

        # small sleep so we don't use 100% CPU
        time.sleep(0.01)

except KeyboardInterrupt:
    print("\nStopped by user (Ctrl+C)")

# Save all measurements to a file
if data_log:
    pd.DataFrame(data_log).to_csv("live_monitoring_log.csv", index=False)
    print(f"\nSaved {len(data_log)} measurements to live_monitoring_log.csv")

print("Program finished.")
sock.close()