Justin Chung
COEN 331
Dr. Moataghed
3/9/2025

Programming Assignment readme.txt


IEEE 802.11 Wi-Fi Protocol Simulation
Programming Assignment Submission

Project Description
This project simulates IEEE 802.11 Wi-Fi protocol frame exchanges between a client station and an access point (AP) using UDP sockets. It demonstrates key concepts, including:

    Management, control, and data frames
    Frame validation and fragmentation
    Error handling and recovery

Files Included

1. frame.h
Purpose: Defines IEEE 802.11 frame structures and constants
Contents:
    Frame type and subtype definitions (management, control, data)
    802.11 frame structure with packed attributes
    Function declarations for Frame Check Sequence (FCS) calculations

2. frame.c
Purpose: Implements frame validation functionality
Key Functions:
    getCheckSumValue(): Computes the Frame Check Sequence (FCS)
    generate32bitChecksum(): Helper function for checksum calculation

3. server.c
Purpose: Simulates an Access Point (AP)
Key Functions:
    Receives and processes frames from the client
    Validates frame integrity using FCS
    Sends appropriate responses based on frame type:
        Association Response → Sent for Association Requests
        Probe Response → Sent for Probe Requests
        CTS (Clear to Send) → Sent for RTS (Request to Send), decrementing duration_id
        ACK (Acknowledge) → Sent for valid data frames, decrementing duration_id

4. client.c
Purpose: Simulates a client station
Key Functions:
    Sends IEEE 802.11 frames to the AP in sequence
    Implements a retry mechanism with timeout handling
    Demonstrates frame fragmentation
    Tests error handling with intentionally corrupted frames

Compilation and Execution Instructions
Run the following commands to compile the project:

    make

    In Terminal 1, 
	make run-server

    In Terminal 2,
	make run-client


Program Demonstration
This simulation demonstrates various IEEE 802.11 frame exchanges:

    1. Association Request/Response
        Establishes a basic connection between the client and AP

    2. Probe Request/Response
        Simulates network discovery

    3. RTS/CTS Exchange
        Implements channel reservation before data transmission

    4. Data Frame Exchange
        Sends a simple data frame with an acknowledgment (ACK)

    5. Bad FCS Test
        Sends frames with a corrupted FCS to test error detection

    6. Multiple Frame Procedure (Fragmentation Test)
        Transmits five fragmented frames:
            RTS with Duration = 12 followed by CTS with Duration = 11
            Five frames with decreasing duration and correct more_fragment bit

    7. Error Recovery Test
        Tests the handling of erroneous frames:
            One correct frame followed by four invalid FCS frames
            Retry mechanism triggers for unacknowledged frames

Technical Implementation Details

Socket Communication:
    Uses UDP sockets with:
        Server listening on port 8080
        Client sending from port 8081

Frame Validation:
    Implements custom checksum-based FCS calculation

Retry Mechanism:
    Uses a 3-second timeout with three retry attempts for lost/unacknowledged frames

Duration Management:
    Proper decrementing of the duration field in control frames

Fragmentation Handling:
    Implements frame sequencing and more_fragments bit

Server Output

    ./server
    UDP Server (Access Point) started. Listening on port 8080

    Received packet from 127.0.0.1:8081
    Received Association Request
    Sending Association Response

    Received packet from 127.0.0.1:8081
    Received Probe Request
    Sending Probe Response

    Received packet from 127.0.0.1:8081
    Received RTS, duration_id=4
    Sending CTS, duration_id=3

    Received packet from 127.0.0.1:8081
    Received Data Frame, duration_id=2, more_fragments=0, seq_ctrl=0
    Sending ACK, duration_id=1

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xB8DCE156

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xB8DCE156

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xB8DCE156

    Received packet from 127.0.0.1:8081
    Received RTS, duration_id=12
    Sending CTS, duration_id=11

    Received packet from 127.0.0.1:8081
    Received Data Frame, duration_id=10, more_fragments=1, seq_ctrl=0
    Sending ACK, duration_id=9

    Received packet from 127.0.0.1:8081
    Received Data Frame, duration_id=8, more_fragments=1, seq_ctrl=1
    Sending ACK, duration_id=7

    Received packet from 127.0.0.1:8081
    Received Data Frame, duration_id=6, more_fragments=1, seq_ctrl=2
    Sending ACK, duration_id=5

    Received packet from 127.0.0.1:8081
    Received Data Frame, duration_id=4, more_fragments=1, seq_ctrl=3
    Sending ACK, duration_id=3

    Received packet from 127.0.0.1:8081
    Received Data Frame, duration_id=2, more_fragments=0, seq_ctrl=4
    Sending ACK, duration_id=1

    Received packet from 127.0.0.1:8081
    Received Data Frame, duration_id=2, more_fragments=1, seq_ctrl=0
    Sending ACK, duration_id=1

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0x723FC213

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0x723FC213

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0x723FC213

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xDF69534C

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xDF69534C

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xDF69534C

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xDB9B02F8

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xDB9B02F8

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0xDB9B02F8

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0x2F7A747B

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0x2F7A747B

    Received packet from 127.0.0.1:8081
    FCS ERROR: Received=0x00000000, Calculated=0x2F7A747B


Client Output

    ./client
    UDP Client started. Connecting to AP at 127.0.0.1:8080

    --- Step 1: Association Request ---
    Sending Association Request (Attempt 1)
    Valid response received for Association Request

    --- Step 2: Probe Request ---
    Sending Probe Request (Attempt 1)
    Valid response received for Probe Request

    --- Step 3: RTS Frame ---
    Sending RTS Frame (Attempt 1)
    Valid response received for RTS Frame

    --- Step 4: Data Frame ---
    Sending Data Frame (Attempt 1)
    Valid response received for Data Frame

    --- Step 5: Frame with Bad FCS ---
    Sending Frame with Bad FCS (Attempt 1)
    Socket timeout waiting for response to Frame with Bad FCS
    Sending Frame with Bad FCS (Attempt 2)
    Socket timeout waiting for response to Frame with Bad FCS
    Sending Frame with Bad FCS (Attempt 3)
    Socket timeout waiting for response to Frame with Bad FCS
    Access Point does not respond.

    --- Step 6: Multiple Frame Procedure ---
    Sending RTS for multiple frames...
    Sending RTS for Multiple Frames (Attempt 1)
    Valid response received for RTS for Multiple Frames
    Sending 5 fragmented frames...
    Sending Fragmented Data Frame (Attempt 1)
    Valid response received for Fragmented Data Frame
    Sending Fragmented Data Frame (Attempt 1)
    Valid response received for Fragmented Data Frame
    Sending Fragmented Data Frame (Attempt 1)
    Valid response received for Fragmented Data Frame
    Sending Fragmented Data Frame (Attempt 1)
    Valid response received for Fragmented Data Frame
    Sending Fragmented Data Frame (Attempt 1)
    Valid response received for Fragmented Data Frame

    --- Step 7: Multiple Frames with Errors ---
    Sending 1 correct frame and 4 frames with errors...
    Sending Correct Data Frame (Attempt 1)
    Valid response received for Correct Data Frame
    Sending Data Frame with Bad FCS (Attempt 1)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 2)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 3)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Access Point does not respond.
    No ACK Received for Frame No.2
    Sending Data Frame with Bad FCS (Attempt 1)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 2)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 3)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Access Point does not respond.
    No ACK Received for Frame No.3
    Sending Data Frame with Bad FCS (Attempt 1)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 2)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 3)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Access Point does not respond.
    No ACK Received for Frame No.4
    Sending Data Frame with Bad FCS (Attempt 1)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 2)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Sending Data Frame with Bad FCS (Attempt 3)
    Socket timeout waiting for response to Data Frame with Bad FCS
    Access Point does not respond.
    No ACK Received for Frame No.5
