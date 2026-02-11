/*
 * protocol.h
 *
 * Shared Communication Protocol Layer
 *
 * This module defines all data formats used between:
 * - Mock Sensor
 * - Gateway
 *
 * It contains:
 * - UART packet definitions
 * - CAN-like frame definitions
 * - Encoding and decoding function prototypes
 *
 * The purpose of this layer is to centralize all
 * communication-related logic in one place.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

/* -----------------------------------------------------------
 * UART Sensor Packet Definitions
 * -----------------------------------------------------------
 * Format:
 * [0] Start byte (0xAA)
 * [1] Voltage high byte
 * [2] Voltage low byte
 * [3] Temperature
 * [4] Checksum
 */

#define UART_PACKET_SIZE 5
#define UART_START_BYTE  0xAA

typedef struct
{
    uint16_t voltage_mv;
    uint8_t  temp_c;
} sensor_data_t;


/* -----------------------------------------------------------
 * Fake CAN Frame Definition
 * -----------------------------------------------------------
 * This is not real CAN, but shaped similarly
 * for learning and simulation purposes.
 */

#define CAN_DLC 8

typedef struct __attribute__((packed))
{
    uint32_t can_id;
    uint8_t  dlc;
    uint8_t  data[8];
} fake_can_frame_t;


/* -----------------------------------------------------------
 * UART Encoding / Decoding
 * -----------------------------------------------------------
 */

/*
 * Encodes sensor data into a UART packet buffer.
 * Buffer must be at least UART_PACKET_SIZE bytes.
 */
void protocol_encode_uart(sensor_data_t *data, uint8_t *buffer);

/*
 * Decodes UART packet into sensor_data_t.
 * Returns 1 if valid, 0 if invalid.
 */
int protocol_decode_uart(uint8_t *buffer, sensor_data_t *data);


/* -----------------------------------------------------------
 * CAN Frame Preparation
 * -----------------------------------------------------------
 */

/*
 * Fills a CAN-like frame using sensor data and status.
 */
void protocol_prepare_can_frame(
    fake_can_frame_t *frame,
    sensor_data_t *data,
    uint8_t status
);

#endif
