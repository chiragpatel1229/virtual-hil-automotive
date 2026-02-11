# monitor.py
# This file contains helper functions that help explain
# why the AI thinks something is strange
# and what we should do about it

def explain_anomaly(voltage, delta_v, noise_std, temp, baseline):
    """
    Try to give a human-readable reason why this point looks strange.
    Uses simple rules based on what we learned during training.
    """
    reasons = []

    # Did the voltage jump a lot suddenly?
    if abs(delta_v) > 3 * baseline["std_delta"]:
        reasons.append("Sudden voltage change")

    # Is the signal becoming noisier than normal?
    if noise_std > baseline["mean_noise"] + 3 * baseline["std_noise"]:
        reasons.append("Noise growth detected")

    # Is voltage lower than anything we saw during training?
    if voltage < baseline["min_voltage"]:
        reasons.append("Voltage below learned normal range")

    # Is temperature outside what we learned?
    if temp < baseline["min_temp"] or temp > baseline["max_temp"]:
        reasons.append("Temperature out of normal range")

    # If we have no specific reason, just say it's unusual
    if not reasons:
        reasons.append("Behavioral outlier")

    return " + ".join(reasons)


def ai_recommendation(reason):
    """
    Give a simple suggestion what to do.
    Important: AI only suggests — never decides!
    """
    if "Noise" in reason:
        return "Recommend derating (reduce power)"
    if "Voltage below" in reason:
        return "Recommend safe mode / stop high load"
    if "Temperature" in reason:
        return "Check cooling / thermal system"
    return "Monitor only - no clear action yet"