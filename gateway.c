#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <arpa/inet.h>

#define WOKWI_IP "127.0.0.1"
#define WOKWI_PORT 4000
#define CAN_IFACE "vcan0"

// Status Codes
#define STATUS_OK 0x00
#define STATUS_WARN_LOW_VOLT 0x01
#define STATUS_CRIT_TEMP 0x02

int main() {
    int sock_tcp, sock_can;
    struct sockaddr_in server_addr;
    struct sockaddr_can addr_can;
    struct ifreq ifr;
    struct can_frame frame;
    
    // Buffer for UART packet: [HEAD, VOLT_H, VOLT_L, TEMP, CS]
    unsigned char buffer[5]; 

    printf("[Gateway] Initializing System...\n");

    // ------------------------------------------------
    // 1. Setup SocketCAN
    // ------------------------------------------------
    if ((sock_can = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("[Error] SocketCAN creation failed");
        return 1;
    }

    strcpy(ifr.ifr_name, CAN_IFACE);
    ioctl(sock_can, SIOCGIFINDEX, &ifr);

    addr_can.can_family = AF_CAN;
    addr_can.can_ifindex = ifr.ifr_ifindex;

    if (bind(sock_can, (struct sockaddr *)&addr_can, sizeof(addr_can)) < 0) {
        perror("[Error] SocketCAN bind failed");
        return 1;
    }
    printf("[Gateway] Connected to %s\n", CAN_IFACE);

    // ------------------------------------------------
    // 2. Setup TCP Connection to Wokwi
    // ------------------------------------------------
    if ((sock_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[Error] TCP Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(WOKWI_PORT);
    
    // Note: In WSL2, 127.0.0.1 connects to WSL's localhost. 
    // If Wokwi is on Windows, we might need the Windows Host IP.
    // Try 127.0.0.1 first if using standard forwarding.
    if (inet_pton(AF_INET, WOKWI_IP, &server_addr.sin_addr) <= 0) {
        perror("[Error] Invalid address");
        return 1;
    }

    printf("[Gateway] Connecting to Wokwi TCP (make sure simulation is running)...\n");
    if (connect(sock_tcp, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[Error] Connection failed. Is Wokwi Gateway running?");
        return 1;
    }
    printf("[Gateway] Connected to Wokwi!\n");

    // ------------------------------------------------
    // 3. Main Loop: Read UART -> Process -> Send CAN
    // ------------------------------------------------
    while (1) {
        // We attempt to read exactly 5 bytes
        int bytes_read = 0;
        while(bytes_read < 5) {
            int result = read(sock_tcp, buffer + bytes_read, 5 - bytes_read);
            if (result <= 0) {
                printf("[Gateway] TCP Connection closed or error.\n");
                close(sock_tcp);
                close(sock_can);
                return 1;
            }
            bytes_read += result;
        }

        // --- Parsing ---
        // Packet: [0xAA] [VH] [VL] [T] [CS]
        
        if (buffer[0] != 0xAA) {
            printf("[Warn] Sync byte error. Expected 0xAA, got 0x%02X\n", buffer[0]);
            // In a real system we would scan forward to find 0xAA, 
            // but here we just continue to keep it simple.
            continue; 
        }

        unsigned char volt_hi = buffer[1];
        unsigned char volt_lo = buffer[2];
        unsigned char temp = buffer[3];
        unsigned char received_cs = buffer[4];

        // Checksum Validation
        // CHECKSUM is (0xAA + VOLT_HI + VOLT_LO + TEMP) & 0xFF
        unsigned char calculated_cs = (0xAA + volt_hi + volt_lo + temp) & 0xFF;

        if (calculated_cs != received_cs) {
            printf("[Warn] Checksum mismatch! Calc: 0x%02X, Recv: 0x%02X\n", calculated_cs, received_cs);
            continue; 
        }

        // Reconstruct physical values
        uint16_t voltage_mv = (volt_hi << 8) | volt_lo;

        // --- Safety Logic ---
        uint8_t status = STATUS_OK;
        
        // Spec: Temp > 60 is Critical. Volt < 3100 is Error/Warn.
        if (temp > 60) {
            status = STATUS_CRIT_TEMP;
        } else if (voltage_mv < 3100) {
            status = STATUS_WARN_LOW_VOLT;
        }

        // --- Prepare CAN Frame ---
        frame.can_id = 0x100;
        frame.can_dlc = 8;
        frame.data[0] = volt_hi;
        frame.data[1] = volt_lo;
        frame.data[2] = temp;
        frame.data[3] = status;
        frame.data[4] = 0; // Padding
        frame.data[5] = 0;
        frame.data[6] = 0;
        frame.data[7] = 0;

        // --- Send CAN Frame ---
        if (write(sock_can, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            perror("[Error] CAN write error");
        } else {
            // Debug print to console so we see it's alive
            printf("TX CAN ID:0x100 | Volt:%dmV Temp:%dC Status:0x%02X\n", 
                   voltage_mv, temp, status);
        }
    }

    return 0;
}