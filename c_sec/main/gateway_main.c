/*
 * gateway_main.c
 *
 * Gateway Application Layer
 *
 * Responsibilities:
 * - Connect to mock sensor (TCP)
 * - Validate packets
 * - Apply safety logic
 * - Forward CAN-like frame via UDP
 */

#include "gateway.h"
#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SENSOR_IP   "127.0.0.1"
#define SENSOR_PORT 4000

#define UDP_IP      "127.0.0.1"
#define UDP_PORT    5000

// typedef struct __attribute__((packed))
// {
//     uint32_t can_id;
//     uint8_t  dlc;
//     uint8_t  data[8];
// } fake_can_frame_t;

int main(void)
{
    int sock_tcp, sock_udp;
    struct sockaddr_in sensor_addr, udp_addr;
    unsigned char buffer[5];
    fake_can_frame_t frame;

    printf("[GATEWAY] Starting ECU Gateway\n");

    sock_udp = socket(AF_INET, SOCK_DGRAM, 0);

    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, UDP_IP, &udp_addr.sin_addr);

    sock_tcp = socket(AF_INET, SOCK_STREAM, 0);

    sensor_addr.sin_family = AF_INET;
    sensor_addr.sin_port = htons(SENSOR_PORT);
    inet_pton(AF_INET, SENSOR_IP, &sensor_addr.sin_addr);

    while (connect(sock_tcp, (struct sockaddr *)&sensor_addr, sizeof(sensor_addr)) < 0)
    {
        sleep(2);
    }

    printf("[GATEWAY] Connected to Mock Sensor\n");

    while (1)
    {
        int bytes_read = 0;

        while (bytes_read < 5)
        {
            int result = recv(sock_tcp, buffer + bytes_read, 5 - bytes_read, 0);
            if (result <= 0)
                return 1;

            bytes_read += result;
        }

        if (buffer[0] != 0xAA)
            continue;

        uint16_t voltage_mv = (buffer[1] << 8) | buffer[2];
        uint8_t temp = buffer[3];

        uint8_t status = gateway_evaluate_status(voltage_mv, temp);

        frame.can_id = 0x100;
        frame.dlc = 8;

        frame.data[0] = buffer[1];
        frame.data[1] = buffer[2];
        frame.data[2] = temp;
        frame.data[3] = status;

        memset(&frame.data[4], 0, 4);

        sendto(sock_udp, &frame, sizeof(frame), 0,
               (struct sockaddr *)&udp_addr, sizeof(udp_addr));

        printf(
            "\r[GATEWAY RX->TX] Volt:%4dmV | Temp:%3dC | Status:0x%02X",
            voltage_mv,
            temp,
            status
        );
        fflush(stdout);
    }

    return 0;
}
