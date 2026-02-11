# config.py
# This file contains all the important numbers and settings
# so we can easily change them later without touching the main code

# UDP settings - where we listen for data from the gateway
UDP_IP = "127.0.0.1"        # localhost
UDP_PORT = 5000             # port number we chose

# How much data we collect for learning "normal" behavior
TRAINING_SAMPLES = 200      # number of good examples to learn from

# How many recent voltage values we use to calculate noise
WINDOW_SIZE = 20

# How much "strange" data we expect (small number = sensitive)
CONTAMINATION = 0.02        # 2% is a common starting value

# How many anomalies in a row we need to raise a real alert
ANOMALY_WINDOW = 10         # look at last 10 measurements
ANOMALY_THRESHOLD = 3       # need at least 3 anomalies to alert

# How long we monitor and record in live mode (in seconds)
COLLECTION_TIME = 3 * 60   # 3 minutes = 180 seconds