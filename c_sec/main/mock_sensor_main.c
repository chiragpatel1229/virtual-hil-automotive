/*
 * mock_sensor_main.c
 *
 * Application Layer for Mock Sensor
 *
 * This file handles:
 * - TCP server setup
 * - Packet formatting
 * - Data transmission
 *
 * All battery simulation logic is inside mock_sensor.c.
 */

#include "mock_sensor.h"
#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 4000

int main(void)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    unsigned char buffer[5];

    printf("========================================\n");
    printf("[MOCK SENSOR] Simulated STM32 started\n");
    printf("========================================\n");

    /* TCP Server Setup */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Waiting for Gateway connection...\n");

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    printf("Gateway connected\n");

    mock_sensor_init();

    long sequence = 0;

    while (1)
    {
        sequence++;

        battery_data_t data = mock_sensor_read();

        /* Packet format:
           [0] 0xAA
           [1] Voltage high byte
           [2] Voltage low byte
           [3] Temperature
           [4] Checksum
        */

        buffer[0] = 0xAA;
        buffer[1] = (data.voltage_mv >> 8) & 0xFF;
        buffer[2] = data.voltage_mv & 0xFF;
        buffer[3] = data.temp_c;
        buffer[4] = (buffer[0] + buffer[1] + buffer[2] + buffer[3]) & 0xFF;

        send(new_socket, buffer, 5, 0);

        printf(
            "\r[MOCK SENSOR TX] Seq:%ld | Volt:%4dmV | Temp:%3dC",
            sequence,
            data.voltage_mv,
            data.temp_c
        );
        fflush(stdout);

        usleep(100000);
    }

    return 0;
}
