#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "frame.h"

#define SERVER_PORT 8080
#define MAX_BUFFER_SIZE 2500  // Larger than max frame size

// Initialize client MAC address (expected client MAC)
const uint8_t CLIENT_MAC[6] = {0x12, 0x45, 0xCC, 0xDD, 0xEE, 0x88};
// Initialize AP MAC address
const uint8_t AP_MAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD};

// Function to validate received frame
int validate_frame(udp_payload *payload) {
    // Check start and end frame identifiers
    if (payload->start_frame_id != START_FRAME_ID || payload->end_frame_id != END_FRAME_ID) {
        printf("Invalid frame identifiers\n");
        return 0;
    }
    
    // Calculate and verify FCS
    uint32_t calculated_fcs = getCheckSumValue(&payload->frame, sizeof(ieee80211_frame), 0, 4);
    if (calculated_fcs != payload->frame.fcs) {
        printf("FCS (Frame Check Sequence) Error\n");
        return 0;
    }
    
    return 1;  // Frame is valid
}

// Create Association Response frame
size_t create_association_response(uint8_t *buffer) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 0;           // Management frame
    frame.frame_control.subtype = 1;        // Association Response
    frame.frame_control.to_ds = 0;          // From AP
    frame.frame_control.from_ds = 1;        // To client
    frame.frame_control.more_frag = 0;
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID (association ID)
    frame.duration_id = 0xABCD;  // Any 2-byte hex value
    
    // Set addresses
    memcpy(frame.addr1, CLIENT_MAC, 6);     // Receiver (Client)
    memcpy(frame.addr2, AP_MAC, 6);         // Transmitter (AP)
    memcpy(frame.addr3, AP_MAC, 6);         // AP BSSID
    
    // Set sequence control
    frame.seq_ctrl = 0;
    
    // Set addr4 to all zeros
    memset(frame.addr4, 0, 6);
    
    // Calculate FCS
    frame.fcs = getCheckSumValue(&frame, sizeof(ieee80211_frame), 0, 4);
    
    // Create the complete UDP payload
    udp_payload payload;
    payload.start_frame_id = START_FRAME_ID;
    memcpy(&payload.frame, &frame, sizeof(ieee80211_frame));
    payload.end_frame_id = END_FRAME_ID;
    
    // Copy to output buffer
    memcpy(buffer, &payload, sizeof(udp_payload));
    
    return sizeof(udp_payload);
}

// Create Probe Response frame
size_t create_probe_response(uint8_t *buffer) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 0;           // Management frame
    frame.frame_control.subtype = 5;        // Probe Response
    frame.frame_control.to_ds = 0;          // From AP
    frame.frame_control.from_ds = 1;        // To client
    frame.frame_control.more_frag = 0;
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID (association ID)
    frame.duration_id = 0x1234;  // Any 2-byte hex value
    
    // Set addresses
    memcpy(frame.addr1, CLIENT_MAC, 6);     // Receiver (Client)
    memcpy(frame.addr2, AP_MAC, 6);         // Transmitter (AP)
    memcpy(frame.addr3, AP_MAC, 6);         // AP BSSID
    
    // Set sequence control
    frame.seq_ctrl = 0;
    
    // Set addr4 to all zeros
    memset(frame.addr4, 0, 6);
    
    // Calculate FCS
    frame.fcs = getCheckSumValue(&frame, sizeof(ieee80211_frame), 0, 4);
    
    // Create the complete UDP payload
    udp_payload payload;
    payload.start_frame_id = START_FRAME_ID;
    memcpy(&payload.frame, &frame, sizeof(ieee80211_frame));
    payload.end_frame_id = END_FRAME_ID;
    
    // Copy to output buffer
    memcpy(buffer, &payload, sizeof(udp_payload));
    
    return sizeof(udp_payload);
}

// Create CTS frame
size_t create_cts_frame(uint8_t *buffer, uint16_t duration_id) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 1;           // Control frame
    frame.frame_control.subtype = 12;       // CTS
    frame.frame_control.to_ds = 0;          // From AP
    frame.frame_control.from_ds = 1;        // To client
    frame.frame_control.more_frag = 0;
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID (one less than the RTS duration)
    frame.duration_id = duration_id - 1;
    
    // Set addresses
    memcpy(frame.addr1, CLIENT_MAC, 6);     // Receiver (Client)
    memcpy(frame.addr2, AP_MAC, 6);         // Transmitter (AP)
    memcpy(frame.addr3, AP_MAC, 6);         // AP BSSID
    
    // Set sequence control
    frame.seq_ctrl = 0;
    
    // Set addr4 to all zeros
    memset(frame.addr4, 0, 6);
    
    // Calculate FCS
    frame.fcs = getCheckSumValue(&frame, sizeof(ieee80211_frame), 0, 4);
    
    // Create the complete UDP payload
    udp_payload payload;
    payload.start_frame_id = START_FRAME_ID;
    memcpy(&payload.frame, &frame, sizeof(ieee80211_frame));
    payload.end_frame_id = END_FRAME_ID;
    
    // Copy to output buffer
    memcpy(buffer, &payload, sizeof(udp_payload));
    
    return sizeof(udp_payload);
}

// Create ACK frame
size_t create_ack_frame(uint8_t *buffer, uint16_t duration_id) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 1;           // Control frame
    frame.frame_control.subtype = 13;       // ACK
    frame.frame_control.to_ds = 0;          // From AP
    frame.frame_control.from_ds = 1;        // To client
    frame.frame_control.more_frag = 0;
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID (one less than the data frame duration)
    frame.duration_id = duration_id - 1;
    
    // Set addresses
    memcpy(frame.addr1, CLIENT_MAC, 6);     // Receiver (Client)
    memcpy(frame.addr2, AP_MAC, 6);         // Transmitter (AP)
    memcpy(frame.addr3, AP_MAC, 6);         // AP BSSID
    
    // Set sequence control
    frame.seq_ctrl = 0;
    
    // Set addr4 to all zeros
    memset(frame.addr4, 0, 6);
    
    // Calculate FCS
    frame.fcs = getCheckSumValue(&frame, sizeof(ieee80211_frame), 0, 4);
    
    // Create the complete UDP payload
    udp_payload payload;
    payload.start_frame_id = START_FRAME_ID;
    memcpy(&payload.frame, &frame, sizeof(ieee80211_frame));
    payload.end_frame_id = END_FRAME_ID;
    
    // Copy to output buffer
    memcpy(buffer, &payload, sizeof(udp_payload));
    
    return sizeof(udp_payload);
}

// Process received frame and send appropriate response
void process_frame(int socket_fd, uint8_t *recv_buffer, size_t recv_size, 
                  struct sockaddr_in *client_addr, socklen_t client_addr_len) {
    udp_payload *payload = (udp_payload *)recv_buffer;
    uint8_t send_buffer[MAX_BUFFER_SIZE];
    size_t response_size;
    
    // Check frame identifiers
    if (payload->start_frame_id != START_FRAME_ID || payload->end_frame_id != END_FRAME_ID) {
        printf("Invalid frame identifiers, ignoring packet\n");
        return;
    }
    
    // Get frame type and subtype
    uint8_t frame_type = payload->frame.frame_control.type;
    uint8_t frame_subtype = payload->frame.frame_control.subtype;
    
    uint32_t calculated_fcs = getCheckSumValue(&payload->frame, sizeof(ieee80211_frame), 0, 4);
    if (calculated_fcs != payload->frame.fcs) {
        printf("FCS ERROR: Received=0x%08X, Calculated=0x%08X\n", 
               payload->frame.fcs, calculated_fcs);
        return;  // Don't respond to frames with FCS errors
    }
    
    // Process frame based on type and subtype
    if (frame_type == 0) {  // Management frame
        if (frame_subtype == 0) {  // Association Request
            printf("Received Association Request\n");
            response_size = create_association_response(send_buffer);
            printf("Sending Association Response\n");
        } else if (frame_subtype == 4) {  // Probe Request
            printf("Received Probe Request\n");
            response_size = create_probe_response(send_buffer);
            printf("Sending Probe Response\n");
        } else {
            printf("Unsupported management frame subtype: %d\n", frame_subtype);
            return;
        }
    } else if (frame_type == 1) {  // Control frame
        if (frame_subtype == 11) {  // RTS
            printf("Received RTS, duration_id=%d\n", payload->frame.duration_id);
            response_size = create_cts_frame(send_buffer, payload->frame.duration_id);
            printf("Sending CTS, duration_id=%d\n", payload->frame.duration_id - 1);
        } else {
            printf("Unsupported control frame subtype: %d\n", frame_subtype);
            return;
        }
    } else if (frame_type == 2) {  // Data frame
        printf("Received Data Frame, duration_id=%d, more_fragments=%d, seq_ctrl=%d\n", 
               payload->frame.duration_id, 
               payload->frame.frame_control.more_frag,
               payload->frame.seq_ctrl);
        response_size = create_ack_frame(send_buffer, payload->frame.duration_id);
        printf("Sending ACK, duration_id=%d\n", payload->frame.duration_id - 1);
    } else {
        printf("Unsupported frame type: %d\n", frame_type);
        return;
    }
    
    // Send response
    if (sendto(socket_fd, send_buffer, response_size, 0, 
              (struct sockaddr *)client_addr, client_addr_len) < 0) {
        perror("sendto failed");
    }
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    uint8_t buffer[MAX_BUFFER_SIZE];
    ssize_t recv_len;
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Bind socket to server address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    printf("UDP Server (Access Point) started. Listening on port %d\n", SERVER_PORT);
    
    // Main loop to receive and process frames
    while(1) {
        // Receive a packet
        recv_len = recvfrom(server_socket, buffer, MAX_BUFFER_SIZE, 0, 
                          (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (recv_len < 0) {
            perror("recvfrom failed");
            continue;
        }
        
        printf("\nReceived packet from %s:%d (%ld bytes)\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recv_len);
        
        // Process the received frame
        process_frame(server_socket, buffer, recv_len, &client_addr, client_addr_len);
    }
    
    // Close socket (never reached in this example)
    close(server_socket);
    
    return 0;
}