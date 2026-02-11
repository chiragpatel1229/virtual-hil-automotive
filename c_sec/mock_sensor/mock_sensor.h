/*
 * mock_sensor.h
 *
 * Mock Sensor Module Interface
 *
 * This file defines the public interface of the mock battery sensor.
 * The goal is to separate the sensor simulation logic from the
 * application layer (main function).
 *
 * In real embedded systems, this would represent the driver layer
 * that provides measured data to the rest of the ECU software.
 */

#ifndef MOCK_SENSOR_H
#define MOCK_SENSOR_H

#include <stdint.h>

/*
 * Battery measurement structure
 * This represents one sensor reading.
 */
typedef struct
{
    uint16_t voltage_mv;   // Battery voltage in millivolts
    uint8_t  temp_c;       // Temperature in degree Celsius
} battery_data_t;

/*
 * Initializes internal simulation variables.
 * Must be called once before using mock_sensor_read().
 */
void mock_sensor_init(void);

/*
 * Generates one simulated battery measurement.
 * Returns a battery_data_t structure.
 */
battery_data_t mock_sensor_read(void);

#endif
