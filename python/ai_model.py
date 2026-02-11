# ai_model.py
# This file creates and trains our anomaly detection model
# We use Isolation Forest because it's good at finding unusual points
# and it doesn't need us to tell it exactly what is bad

import pandas as pd
from sklearn.ensemble import IsolationForest
import numpy as np

def train_ai_model(training_features, contamination=0.02):
    # Convert our list of measurements into a nice table (DataFrame)
    df = pd.DataFrame(
        training_features,
        columns=["Voltage", "DeltaVoltage", "NoiseStd", "Temperature"]
    )

    # Create the anomaly detection model
    # n_estimators = how many small decision trees we use (more = better but slower)
    # contamination = how much weird data we expect (0.02 = 2%)
    model = IsolationForest(
        n_estimators=200,
        contamination=contamination,
        random_state=42         # same random seed = same results every time
    )

    # Teach the model what "normal" looks like
    model.fit(df)

    # Save some important numbers from the training data
    # We will use these later to explain why something is strange
    baseline = {
        "mean_delta": df["DeltaVoltage"].mean(),
        "std_delta": df["DeltaVoltage"].std(),
        "mean_noise": df["NoiseStd"].mean(),
        "std_noise": df["NoiseStd"].std(),
        "min_voltage": df["Voltage"].min(),
        "max_voltage": df["Voltage"].max(),
        "min_temp": df["Temperature"].min(),
        "max_temp": df["Temperature"].max()
    }

    return model, baseline, df