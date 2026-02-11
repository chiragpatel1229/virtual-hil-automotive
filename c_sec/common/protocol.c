/*
 * protocol.c
 *
 * Communication Protocol Implementation
 *
 * This file contains all encoding and decoding logic
 * for UART packets and CAN-like frames.
 *
 * Keeping this separate ensures:
 * - Reusability
 * - Clean architecture
 * - Easy debugging
 */

#include "protocol.h"
#include <string.h>

/*
 * Internal helper function to calculate checksum.
 * Checksum = sum of first 4 bytes masked to 8 bits.
 */
static uint8_t calculate_checksum(uint8_t *buffer)
{
    return (buffer[0] + buffer[1] + buffer[2] + buffer[3]) & 0xFF;
}

void protocol_encode_uart(sensor_data_t *data, uint8_t *buffer)
{
    buffer[0] = UART_START_BYTE;
    buffer[1] = (data->voltage_mv >> 8) & 0xFF;
    buffer[2] = data->voltage_mv & 0xFF;
    buffer[3] = data->temp_c;
    buffer[4] = calculate_checksum(buffer);
}

int protocol_decode_uart(uint8_t *buffer, sensor_data_t *data)
{
    if (buffer[0] != UART_START_BYTE)
        return 0;

    uint8_t expected_checksum = calculate_checksum(buffer);

    if (expected_checksum != buffer[4])
        return 0;

    data->voltage_mv = (buffer[1] << 8) | buffer[2];
    data->temp_c = buffer[3];

    return 1;
}

void protocol_prepare_can_frame(
    fake_can_frame_t *frame,
    sensor_data_t *data,
    uint8_t status
)
{
    frame->can_id = 0x100;
    frame->dlc = CAN_DLC;

    frame->data[0] = (data->voltage_mv >> 8) & 0xFF;
    frame->data[1] = data->voltage_mv & 0xFF;
    frame->data[2] = data->temp_c;
    frame->data[3] = status;

    /* Clear remaining bytes */
    memset(&frame->data[4], 0, 4);
}
