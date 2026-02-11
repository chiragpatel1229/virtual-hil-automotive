/*
 * gateway.h
 *
 * Gateway Module Interface
 *
 * This module receives validated sensor data and
 * prepares CAN-like frames for transmission.
 */

#ifndef GATEWAY_H
#define GATEWAY_H

#include <stdint.h>

#define STATUS_OK             0x00
#define STATUS_WARN_LOW_VOLT  0x01
#define STATUS_CRIT_TEMP      0x02

uint8_t gateway_evaluate_status(uint16_t voltage_mv, uint8_t temp);

#endif
