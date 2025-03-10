#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include "frame.h"

/*
* Takes in an input string and generates a 32-bit checksum hash value of type uint32_t
* This function is internally called by the function getCheckSumValue(); so not to be called directly by
developer
* Input type: String
* Output type: uint32_t
*/
uint32_t generate32bitChecksum(const char* valueToConvert) {
    uint32_t checksum = 0;
    while (*valueToConvert) {
        checksum += *valueToConvert++;
        checksum += (checksum << 10);
        checksum ^= (checksum >> 6);
    }
    checksum += (checksum << 3);
    checksum ^= (checksum >> 11);
    checksum += (checksum << 15);
    return checksum;
}

/*
* This function can be called by the developer to generate a 32-bit checksum directly from the pointer to your
frame structure
* The function is independent of the contents/data types used in your frame structure
* It works based on the bits in your structure
* IMPORTANT NOTE & Hint: For accurate results, you must use __attribute__((packed)) while creating your
Frame structure
* to avoid additional padding bytes which occur in C language structures
* Input: Pointer to the frame structure, the size of the frame structure, number of bytes to skip from the start
and end (for crc calculation)
* Output: uint 32 bit final Check Sum value
*/
uint32_t getCheckSumValue(void *ptr, size_t size, size_t bytesToSkipFromStart, size_t bytesToSkipFromEnd) {
    const unsigned char *byte = (const unsigned char *)ptr;
    // binaryString[] is a logical representation of 1 byte. Each character in it represents 1 bit.
    char binaryString[9]; // One additional character for the null terminator
    binaryString[8] = '\0'; // Null terminator definition

    char *buffer = malloc(1); // Allocates space for an empty string (1 byte for the null terminator)
    buffer[0] = '\0'; // Initializes an empty string
    for (size_t i = 1; i <= size; i++) {
        for (int j = 7; j >= 0; j--) {
            int bit = (byte[i - 1] >> j) & 1;
            binaryString[7 - j] = bit + '0'; // Converts bit to character '0' or '1'
        }
        buffer = realloc(buffer, strlen(buffer) + strlen(binaryString) + 1); // Resizes buffer to fit the concatenated
        strcat(buffer, binaryString);
    }
    buffer[strlen(buffer)-(bytesToSkipFromEnd*8)] = '\0';
    memmove(buffer, buffer + (bytesToSkipFromStart*8), strlen(buffer) - (bytesToSkipFromStart*8) + 1); //+1
    
    uint32_t checkSumValue = generate32bitChecksum(buffer);
    free(buffer); // Freeing memory allocated by malloc.
    return checkSumValue;
}

// Initialize frame control field with specified type and subtype
void init_frame_control(frame_control_t *fc, uint8_t type, uint8_t subtype) {
    if (!fc) return;
    
    memset(fc, 0, sizeof(frame_control_t));
    fc->protocol_version = PROTOCOL_VERSION;
    fc->type = type;
    fc->subtype = subtype;
}

// Set MAC address from string representation (e.g., "AA:BB:CC:DD:EE:FF")
void set_address(uint8_t *addr, const char *mac_str) {
    if (!addr || !mac_str) return;
    
    unsigned int values[MAC_ADDR_LEN];
    int result = sscanf(mac_str, "%x:%x:%x:%x:%x:%x", 
                      &values[0], &values[1], &values[2], 
                      &values[3], &values[4], &values[5]);
    
    if (result == MAC_ADDR_LEN) {
        for (int i = 0; i < MAC_ADDR_LEN; i++) {
            addr[i] = (uint8_t)values[i];
        }
    } else {
        // Default to zeros if invalid format
        memset(addr, 0, MAC_ADDR_LEN);
    }
}

// Print MAC address in human-readable format
void print_mac_address(const uint8_t *addr) {
    if (!addr) return;
    
    printf("%02X:%02X:%02X:%02X:%02X:%02X", 
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

// Print detailed information about an IEEE 802.11 frame
void print_frame_info(const ieee80211_frame *frame) {
    if (!frame) return;
    
    // Print frame control information
    printf("Frame Control:\n");
    printf("  Protocol Version: %d\n", frame->frame_control.protocol_version);
    
    // Print frame type
    printf("  Type: ");
    switch (frame->frame_control.type) {
        case TYPE_MANAGEMENT:
            printf("Management (0)\n");
            break;
        case TYPE_CONTROL:
            printf("Control (1)\n");
            break;
        case TYPE_DATA:
            printf("Data (2)\n");
            break;
        default:
            printf("Unknown (%d)\n", frame->frame_control.type);
    }
    
    // Print frame subtype based on type
    printf("  Subtype: ");
    if (frame->frame_control.type == TYPE_MANAGEMENT) {
        switch (frame->frame_control.subtype) {
            case SUBTYPE_ASSOC_REQ:
                printf("Association Request (0)\n");
                break;
            case SUBTYPE_ASSOC_RESP:
                printf("Association Response (1)\n");
                break;
            case SUBTYPE_PROBE_REQ:
                printf("Probe Request (4)\n");
                break;
            case SUBTYPE_PROBE_RESP:
                printf("Probe Response (5)\n");
                break;
            default:
                printf("Unknown (%d)\n", frame->frame_control.subtype);
        }
    } else if (frame->frame_control.type == TYPE_CONTROL) {
        switch (frame->frame_control.subtype) {
            case SUBTYPE_RTS:
                printf("RTS (11)\n");
                break;
            case SUBTYPE_CTS:
                printf("CTS (12)\n");
                break;
            case SUBTYPE_ACK:
                printf("ACK (13)\n");
                break;
            default:
                printf("Unknown (%d)\n", frame->frame_control.subtype);
        }
    } else if (frame->frame_control.type == TYPE_DATA) {
        switch (frame->frame_control.subtype) {
            case SUBTYPE_DATA:
                printf("Data (0)\n");
                break;
            default:
                printf("Unknown (%d)\n", frame->frame_control.subtype);
        }
    }
    
    // Print flags
    printf("  Flags:\n");
    printf("    To DS: %d\n", frame->frame_control.to_ds);
    printf("    From DS: %d\n", frame->frame_control.from_ds);
    printf("    More Fragments: %d\n", frame->frame_control.more_frag);
    printf("    Retry: %d\n", frame->frame_control.retry);
    printf("    Power Management: %d\n", frame->frame_control.power_mgmt);
    printf("    More Data: %d\n", frame->frame_control.more_data);
    printf("    WEP: %d\n", frame->frame_control.wep);
    printf("    Order: %d\n", frame->frame_control.order);
    
    // Print duration/ID
    printf("Duration/ID: 0x%04X\n", frame->duration_id);
    
    // Print addresses
    printf("Address 1 (Destination): ");
    print_mac_address(frame->addr1);
    printf("\n");
    
    printf("Address 2 (Source): ");
    print_mac_address(frame->addr2);
    printf("\n");
    
    printf("Address 3 (BSSID): ");
    print_mac_address(frame->addr3);
    printf("\n");
    
    printf("Address 4: ");
    print_mac_address(frame->addr4);
    printf("\n");
    
    // Print sequence control
    printf("Sequence Control: 0x%04X\n", frame->seq_ctrl);
    
    // Print FCS
    printf("FCS: 0x%08X\n", frame->fcs);
}