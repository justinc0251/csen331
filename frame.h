#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>

// Frame identifiers - updated to 32-bit values as used in client.c
#define START_FRAME_ID 0xABCDEF12
#define END_FRAME_ID 0x12EFCDAB

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

// MAC Address length in bytes
#define MAC_ADDR_LEN 6

// Structure for Frame Control field (2 bytes) - renamed to match client.c
typedef struct __attribute__((packed)) {
    // First byte
    uint8_t protocol_version : 2;
    uint8_t type : 2;
    uint8_t subtype : 4;
    
    // Second byte
    uint8_t to_ds : 1;
    uint8_t from_ds : 1;
    uint8_t more_frag : 1;
    uint8_t retry : 1;
    uint8_t power_mgmt : 1;
    uint8_t more_data : 1;
    uint8_t wep : 1;
    uint8_t order : 1;
} frame_control_t;

// Structure for IEEE 802.11 frame - consolidated to match client.c usage
typedef struct __attribute__((packed)) {
    frame_control_t frame_control;    // 2 bytes
    uint16_t duration_id;             // 2 bytes
    uint8_t addr1[MAC_ADDR_LEN];      // 6 bytes: Destination address
    uint8_t addr2[MAC_ADDR_LEN];      // 6 bytes: Source address
    uint8_t addr3[MAC_ADDR_LEN];      // 6 bytes: BSSID
    uint16_t seq_ctrl;                // 2 bytes: Sequence control (combined field)
    uint8_t addr4[MAC_ADDR_LEN];      // 6 bytes: Optional fourth address
    uint8_t payload[MAX_PAYLOAD_SIZE]; // Variable length payload
    uint32_t fcs;                     // 4 bytes: Frame Check Sequence
} ieee80211_frame;

// Structure for UDP payload containing IEEE 802.11 frame - renamed to match client.c
typedef struct __attribute__((packed)) {
    uint32_t start_frame_id;         // 4 bytes: Start of frame identifier
    ieee80211_frame frame;           // Variable length IEEE 802.11 frame
    uint32_t end_frame_id;           // 4 bytes: End of frame identifier
} udp_payload;

// Function to calculate checksum for FCS - required by client.c
uint32_t getCheckSumValue(void *buffer, size_t size, size_t start, size_t len);

// Function declarations for frame operations
void init_frame_control(frame_control_t *fc, uint8_t type, uint8_t subtype);
void set_address(uint8_t *addr, const char *mac_str);
void print_mac_address(const uint8_t *addr);
void print_frame_info(const ieee80211_frame *frame);

#endif /* FRAME_H */