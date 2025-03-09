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
#define MAX_PAYLOAD_SIZE 2312
#define MAX_FRAME_SIZE 2346

// Frame Control Field bit positions
#define FC_PROTOCOL_VERSION_POS 0
#define FC_TYPE_POS 2
#define FC_SUBTYPE_POS 4
#define FC_TO_DS_POS 8
#define FC_FROM_DS_POS 9
#define FC_MORE_FRAG_POS 10
#define FC_RETRY_POS 11
#define FC_POWER_MGMT_POS 12
#define FC_MORE_DATA_POS 13
#define FC_WEP_POS 14
#define FC_ORDER_POS 15

// MAC Address length in bytes
#define MAC_ADDR_LEN 6

// Structure for Frame Control field (2 bytes)
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
} FrameControl;

// Structure for Sequence Control field (2 bytes)
typedef struct __attribute__((packed)) {
    uint16_t fragment_number : 4;
    uint16_t sequence_number : 12;
} SequenceControl;

// Structure for IEEE 802.11 frame header
typedef struct __attribute__((packed)) {
    FrameControl frame_control;    // 2 bytes
    uint16_t duration_id;          // 2 bytes
    uint8_t addr1[MAC_ADDR_LEN];   // 6 bytes: Destination address
    uint8_t addr2[MAC_ADDR_LEN];   // 6 bytes: Source address
    uint8_t addr3[MAC_ADDR_LEN];   // 6 bytes: BSSID or other use
    SequenceControl seq_ctrl;      // 2 bytes
    uint8_t addr4[MAC_ADDR_LEN];   // 6 bytes: Used in ad-hoc or special cases
} IEEE80211Header;

// Structure for complete IEEE 802.11 frame
typedef struct __attribute__((packed)) {
    IEEE80211Header header;           // 30 bytes
    uint8_t payload[MAX_PAYLOAD_SIZE]; // Variable length (0-2312 bytes)
    uint32_t fcs;                     // 4 bytes: Frame Check Sequence
} IEEE80211Frame;

// Structure for UDP payload containing IEEE 802.11 frame
typedef struct __attribute__((packed)) {
    uint16_t start_frame_id;         // 2 bytes: Start of frame identifier (0xFFFF)
    IEEE80211Frame frame;            // Variable length IEEE 802.11 frame
    uint16_t end_frame_id;           // 2 bytes: End of frame identifier (0xFFFF)
} UDPPayload;

// Function declarations for frame operations
void init_frame_control(FrameControl *fc, uint8_t type, uint8_t subtype);
void set_address(uint8_t *addr, const char *mac_str);
void print_mac_address(const uint8_t *addr);
void print_frame_info(const IEEE80211Frame *frame);

#endif /* FRAME_H */