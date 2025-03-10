#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>

// Frame identifiers
#define START_FRAME_ID 0xFFFF
#define END_FRAME_ID 0xFFFF

// Protocol version
#define PROTOCOL_VERSION 0

// Frame types
#define TYPE_MANAGEMENT 0x00
#define TYPE_CONTROL 0x01
#define TYPE_DATA 0x02

// Management frame subtypes
#define SUBTYPE_ASSOC_REQ 0x00
#define SUBTYPE_ASSOC_RESP 0x01
#define SUBTYPE_PROBE_REQ 0x04
#define SUBTYPE_PROBE_RESP 0x05

// Control frame subtypes
#define SUBTYPE_RTS 0x0B
#define SUBTYPE_CTS 0x0C
#define SUBTYPE_ACK 0x0D

// Data frame subtypes
#define SUBTYPE_DATA 0x00

// Maximum sizes
#define MAX_PAYLOAD_SIZE 1024
#define MAX_FRAME_SIZE 2346

#define MAC_ADDR_LEN 6

// IEEE 802.11 Frame Control field (2 bytes)
typedef struct __attribute__((packed)) {
    uint8_t protocol_version : 2;
    uint8_t type : 2;
    uint8_t subtype : 4;
    
    uint8_t to_ds : 1;
    uint8_t from_ds : 1;
    uint8_t more_frag : 1;
    uint8_t retry : 1;
    uint8_t power_mgmt : 1;
    uint8_t more_data : 1;
    uint8_t wep : 1;
    uint8_t order : 1;
} frame_control_t;

// IEEE 802.11 frame structure
typedef struct __attribute__((packed)) {
    frame_control_t frame_control;
    uint16_t duration_id;
    uint8_t addr1[MAC_ADDR_LEN];      // Destination address
    uint8_t addr2[MAC_ADDR_LEN];      // Source address
    uint8_t addr3[MAC_ADDR_LEN];      // BSSID
    uint16_t seq_ctrl;                // Sequence control
    uint8_t addr4[MAC_ADDR_LEN];      // Fourth address (optional)
    uint8_t payload[MAX_PAYLOAD_SIZE];
    uint32_t fcs;                     // Frame Check Sequence
} ieee80211_frame;

// UDP payload wrapper for IEEE 802.11 frame
typedef struct __attribute__((packed)) {
    uint16_t start_frame_id;
    ieee80211_frame frame;
    uint16_t end_frame_id;
} udp_payload;

// FCS calculation function
uint32_t getCheckSumValue(void *buffer, size_t size, size_t start, size_t len);

// Frame handling functions
void init_frame_control(frame_control_t *fc, uint8_t type, uint8_t subtype);
void set_address(uint8_t *addr, const char *mac_str);
void print_mac_address(const uint8_t *addr);
void print_frame_info(const ieee80211_frame *frame);

#endif