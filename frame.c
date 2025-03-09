#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "frame.h"

// Implementation of FCS (Frame Check Sequence) calculation
uint32_t getCheckSumValue(void *buffer, size_t size, size_t start, size_t len) {
    uint32_t checksum = 0;
    uint8_t *data = (uint8_t *)buffer;
    
    // Simple checksum calculation - XOR all bytes except the FCS field
    for (size_t i = 0; i < size - len; i++) {
        checksum ^= data[i];
    }
    
    // For a more robust checksum, you might want to use CRC-32 algorithm
    // But this simple XOR implementation works for simulation purposes
    return checksum;
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