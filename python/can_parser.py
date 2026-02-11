# can_parser.py
# This file knows how to understand the data packets coming from the gateway
# The gateway sends 13 bytes that look like a CAN message (but fake/simple)

import struct

def parse_can_frame(data):
    """
    Take 13 bytes from the gateway and turn them into useful numbers.
    Returns: can_id, voltage (in mV), temperature (°C), status
    """
    # unpack the 13 bytes using this format:
    # < = little endian, I = 4-byte unsigned int, B = 1-byte unsigned
    # 8s = 8 bytes of payload
    can_id, dlc, payload = struct.unpack("<IB8s", data)

    # voltage is stored in two bytes (high byte and low byte)
    voltage_mv = (payload[0] << 8) | payload[1]     # combine to 16-bit number

    temperature = payload[2]                        # 1 byte = temperature
    status = payload[3]                             # 1 byte = status code

    return can_id, voltage_mv, temperature, status