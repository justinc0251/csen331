#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include "frame.h"

#define SERVER_IP "127.0.0.1"  // Change this to the server's IP if needed
#define SERVER_PORT 8080
#define CLIENT_PORT 8081
#define MAX_BUFFER_SIZE 2500  // Larger than max frame size to ensure enough space
#define ACK_TIMEOUT 3         // Timeout in seconds
#define MAX_RETRIES 3         // Maximum number of retries

// Global variables
int client_socket;
struct sockaddr_in server_addr;
socklen_t server_addr_len = sizeof(struct sockaddr_in);
volatile int waiting_for_response = 0;
volatile int response_received = 0;

// Signal handler for timeout
void timeout_handler(int signum) {
    if (waiting_for_response) {
        waiting_for_response = 0;
        response_received = 0;
    }
}

// Function to set up a timer
void setup_timer(int seconds) {
    struct sigaction sa;
    struct itimerval timer;

    // Install signal handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timeout_handler;
    sigaction(SIGALRM, &sa, NULL);

    // Configure timer
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;  // Non-recurring timer
    timer.it_interval.tv_usec = 0;

    // Start timer
    setitimer(ITIMER_REAL, &timer, NULL);
}

// Function to cancel timer
void cancel_timer() {
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

// Initialize client MAC address
const uint8_t CLIENT_MAC[6] = {0x12, 0x45, 0xCC, 0xDD, 0xEE, 0x88};
// Initialize AP MAC address
const uint8_t AP_MAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD};

// Function to send a frame and wait for response
int send_frame_and_wait(uint8_t *frame_buffer, size_t frame_size, 
                        uint8_t *response_buffer, size_t *response_size,
                        const char *frame_name) {
    int retries = 0;
    response_received = 0;
    *response_size = 0;

    while (retries < MAX_RETRIES && !response_received) {
        printf("Sending %s (Attempt %d)\n", frame_name, retries + 1);
        
        // Send the frame
        if (sendto(client_socket, frame_buffer, frame_size, 0, 
                  (struct sockaddr *)&server_addr, server_addr_len) < 0) {
            perror("sendto failed");
            return -1;
        }

        // Wait for response with timeout
        waiting_for_response = 1;
        setup_timer(ACK_TIMEOUT);
        
        // Wait for response
        ssize_t recv_size = recvfrom(client_socket, response_buffer, MAX_BUFFER_SIZE, 0, 
                               (struct sockaddr *)&server_addr, &server_addr_len);
        
        if (recv_size > 0) {
            // Response received
            udp_payload *payload = (udp_payload *)response_buffer;
            
            // Validate frame identifiers
            if (payload->start_frame_id != START_FRAME_ID || payload->end_frame_id != END_FRAME_ID) {
                printf("Invalid frame identifiers in response\n");
                continue;  // Treat as no response and retry
            }
            
            // Validate FCS
            uint32_t calculated_fcs = getCheckSumValue(&payload->frame, sizeof(ieee80211_frame), 0, 4);
            if (calculated_fcs != payload->frame.fcs) {
                printf("FCS Error in response\n");
                continue;  // Treat as no response and retry
            }
            
            // Valid response
            *response_size = recv_size;
            response_received = 1;
            waiting_for_response = 0;
            cancel_timer();
            printf("Valid response received for %s\n", frame_name);
            return 1;  // Success
        } else {
            // Check if it's a timeout or other error
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Socket timeout waiting for response to %s\n", frame_name);
                retries++;
            } else {
                perror("recvfrom failed with error");
                retries++;
            }
        }
        
        // Check if timer expired
        if (!waiting_for_response && !response_received) {
            printf("Timer expired waiting for response to %s\n", frame_name);
            retries++;
        }
        
        // Cancel timer before next attempt
        cancel_timer();
    }

    if (retries >= MAX_RETRIES) {
        printf("Access Point does not respond.\n");
        return 0;  // Failed after max retries
    }
    
    return 1;  // Success
}

// Create Association Request frame
size_t create_association_request(uint8_t *buffer) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 0;           // Management frame
    frame.frame_control.subtype = 0;        // Association Request
    frame.frame_control.to_ds = 1;          // To AP
    frame.frame_control.from_ds = 0;        // From client
    frame.frame_control.more_frag = 0;
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID
    frame.duration_id = 0;
    
    // Set addresses
    memcpy(frame.addr1, AP_MAC, 6);         // Receiver (AP)
    memcpy(frame.addr2, CLIENT_MAC, 6);     // Transmitter (Client)
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

// Create Probe Request frame
size_t create_probe_request(uint8_t *buffer) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 0;           // Management frame
    frame.frame_control.subtype = 4;        // Probe Request
    frame.frame_control.to_ds = 1;          // To AP
    frame.frame_control.from_ds = 0;        // From client
    frame.frame_control.more_frag = 0;
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID
    frame.duration_id = 0;
    
    // Set addresses
    memcpy(frame.addr1, AP_MAC, 6);         // Receiver (AP)
    memcpy(frame.addr2, CLIENT_MAC, 6);     // Transmitter (Client)
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

// Create RTS frame
size_t create_rts_frame(uint8_t *buffer, uint16_t duration_id) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 1;           // Control frame
    frame.frame_control.subtype = 11;       // RTS
    frame.frame_control.to_ds = 1;          // To AP
    frame.frame_control.from_ds = 0;        // From client
    frame.frame_control.more_frag = 0;
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID
    frame.duration_id = duration_id;
    
    // Set addresses
    memcpy(frame.addr1, AP_MAC, 6);         // Receiver (AP)
    memcpy(frame.addr2, CLIENT_MAC, 6);     // Transmitter (Client)
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

// Create data frame
size_t create_data_frame(uint8_t *buffer, uint16_t duration_id, int fragment_number, int more_fragments) {
    // Create IEEE 802.11 frame structure
    ieee80211_frame frame;
    memset(&frame, 0, sizeof(ieee80211_frame));
    
    // Set frame control fields
    frame.frame_control.protocol_version = 0;
    frame.frame_control.type = 2;           // Data frame
    frame.frame_control.subtype = 0;        // Data
    frame.frame_control.to_ds = 1;          // To AP
    frame.frame_control.from_ds = 0;        // From client
    frame.frame_control.more_frag = more_fragments;  // Set based on parameter
    frame.frame_control.retry = 0;
    frame.frame_control.power_mgmt = 0;
    frame.frame_control.more_data = 0;
    frame.frame_control.wep = 0;
    frame.frame_control.order = 0;
    
    // Set duration ID
    frame.duration_id = duration_id;
    
    // Set addresses
    memcpy(frame.addr1, AP_MAC, 6);         // Receiver (AP)
    memcpy(frame.addr2, CLIENT_MAC, 6);     // Transmitter (Client)
    memcpy(frame.addr3, AP_MAC, 6);         // AP BSSID
    
    // Set sequence control (use fragment number)
    frame.seq_ctrl = fragment_number;
    
    // Set addr4 to all zeros
    memset(frame.addr4, 0, 6);
    
    // Add some payload data (random, for example)
    char payload_data[100]; // Using small payload for simplicity
    snprintf(payload_data, sizeof(payload_data), "This is frame %d data", fragment_number);
    size_t payload_size = strlen(payload_data);
    memcpy(frame.payload, payload_data, payload_size);
    
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

// Create data frame with incorrect FCS
size_t create_data_frame_bad_fcs(uint8_t *buffer, uint16_t duration_id, int fragment_number, int more_fragments) {
    // Create a normal data frame first
    size_t size = create_data_frame(buffer, duration_id, fragment_number, more_fragments);
    
    // Modify the FCS to make it incorrect
    udp_payload *payload = (udp_payload *)buffer;
    payload->frame.fcs = 0x00000000; // Deliberately wrong FCS
    
    return size;
}

int main() {
    // Create socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up client address
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(CLIENT_PORT);

    // Bind socket to client address
    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("Binding failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // Buffers for sending and receiving
    uint8_t send_buffer[MAX_BUFFER_SIZE];
    uint8_t recv_buffer[MAX_BUFFER_SIZE];
    size_t frame_size, response_size;

    // Set socket timeout for non-blocking operation
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP Client started. Connecting to AP at %s:%d\n", SERVER_IP, SERVER_PORT);

    // Step 1: Send Association Request
    printf("\n--- Step 1: Association Request ---\n");
    frame_size = create_association_request(send_buffer);
    if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, "Association Request")) {
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Step 2: Send Probe Request
    printf("\n--- Step 2: Probe Request ---\n");
    frame_size = create_probe_request(send_buffer);
    if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, "Probe Request")) {
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Step 3: Send RTS
    printf("\n--- Step 3: RTS Frame ---\n");
    frame_size = create_rts_frame(send_buffer, 4);
    if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, "RTS Frame")) {
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Step 4: Send Data Frame
    printf("\n--- Step 4: Data Frame ---\n");
    frame_size = create_data_frame(send_buffer, 2, 0, 0);
    if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, "Data Frame")) {
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Step 5: Send Frame with Bad FCS
    printf("\n--- Step 5: Frame with Bad FCS ---\n");
    frame_size = create_data_frame_bad_fcs(send_buffer, 2, 0, 0);
    send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, "Frame with Bad FCS");
    
    // Step 6: Multiple frame procedure - First send RTS with higher duration
    printf("\n--- Step 6: Multiple Frame Procedure ---\n");
    printf("Sending RTS for multiple frames...\n");
    frame_size = create_rts_frame(send_buffer, 12);
    if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, "RTS for Multiple Frames")) {
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    // Now send 5 fragmented frames with decreasing duration
    printf("Sending 5 fragmented frames...\n");
    for (int i = 0; i < 5; i++) {
        int more_fragments = (i < 4) ? 1 : 0;  // Last frame has no more fragments
        uint16_t duration = 10 - (i * 2);      // Decreasing duration
        
        frame_size = create_data_frame(send_buffer, duration, i, more_fragments);
        if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, 
                               "Fragmented Data Frame")) {
            printf("No ACK Received for Frame No.%d\n", i+1);
        }
    }
    
    // Step 7: Send 1 correct frame and 4 frames with errors
    printf("\n--- Step 7: Multiple Frames with Errors ---\n");
    
    // First frame is correct
    printf("Sending 1 correct frame and 4 frames with errors...\n");
    frame_size = create_data_frame(send_buffer, 2, 0, 1);
    if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, 
                           "Correct Data Frame")) {
        printf("No ACK Received for Frame No.1\n");
    }
    
    // Four frames with errors
    for (int i = 1; i < 5; i++) {
        int more_fragments = (i < 4) ? 1 : 0;  // Last frame has no more fragments
        
        // Create frame with bad FCS
        frame_size = create_data_frame_bad_fcs(send_buffer, 2, i, more_fragments);
        
        // Send frame and expect no response due to bad FCS
        if (!send_frame_and_wait(send_buffer, frame_size, recv_buffer, &response_size, 
                               "Data Frame with Bad FCS")) {
            printf("No ACK Received for Frame No.%d\n", i+1);
        }
    }
    
    // Close socket
    close(client_socket);
    printf("\nClient terminated.\n");
    
    return 0;
}