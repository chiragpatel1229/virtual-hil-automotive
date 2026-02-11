/*
 * mock_sensor.c
 *
 * Mock Sensor Simulation Logic
 *
 * This file contains the internal battery behavior model.
 * It simulates:
 * - Normal voltage variation
 * - Gradual aging
 * - Noise increase
 * - Slow degradation
 * - Sudden fault injection
 *
 * The goal is to generate realistic signals for testing.
 */

#include "mock_sensor.h"
#include <stdlib.h>
#include <time.h>

/* 
 * Internal persistent simulation variables.
 * These are static so they are not visible outside this file.
 */
static uint16_t voltage_mv = 3300;
static uint8_t  temp_c = 45;

static float noise_amplitude = 2.0f;
static long degradation_counter = 0;
static long total_packets_sent = 0;

void mock_sensor_init(void)
{
    srand(time(NULL));
}

battery_data_t mock_sensor_read(void)
{
    battery_data_t data;

    total_packets_sent++;
    degradation_counter++;

    /* Normal sawtooth behavior */
    voltage_mv += 10;
    if (voltage_mv > 4000)
    {
        voltage_mv = 3000;
    }

    /* Gradual noise increase over time */
    if (degradation_counter % 100 == 0)
    {
        noise_amplitude += 0.5f;
    }

    int noise = (rand() % (int)(noise_amplitude * 2)) - (int)noise_amplitude;
    voltage_mv += noise;

    /* Slow aging voltage sag */
    if (degradation_counter > 600)
    {
        if (voltage_mv > 200)
        {
            voltage_mv -= 1;
        }
    }

    /* Hard fault injection */
    if (total_packets_sent > 300)
    {
        int chance = (rand() % 100) + 1;
        if (chance <= 2)
        {
            voltage_mv = 100;
        }
    }

    data.voltage_mv = voltage_mv;
    data.temp_c = temp_c;

    return data;
}
