/*
 * gateway.c
 *
 * Safety Logic Layer
 *
 * This file contains deterministic safety rules.
 * The AI is not allowed to modify these decisions.
 */

#include "gateway.h"

uint8_t gateway_evaluate_status(uint16_t voltage_mv, uint8_t temp)
{
    if (temp > 60)
    {
        return STATUS_CRIT_TEMP;
    }

    if (voltage_mv < 3100)
    {
        return STATUS_WARN_LOW_VOLT;
    }

    return STATUS_OK;
}
