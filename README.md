# IEEE 802.11 Wi-Fi Protocol Simulation

This project provides a simulation of IEEE 802.11 Wi-Fi protocol frame exchanges between a client and an access point using UDP sockets.

## Overview

The simulation consists of a client that sends various types of IEEE 802.11 frames to an access point server, which processes the frames and responds with appropriate frame types according to the protocol. This helps demonstrate key concepts of the Wi-Fi protocol including:

* Management frames (Association, Probe)
* Control frames (RTS/CTS, ACK)
* Data frames (including fragmentation)
* Frame validation using FCS (Frame Check Sequence)
* Error handling

## Components

* [frame.h](vscode-file://vscode-app/Applications/Visual%20Studio%20Code.app/Contents/Resources/app/out/vs/code/electron-sandbox/workbench/workbench.html): Header file defining IEEE 802.11 frame structures and constants
* [frame.c](vscode-file://vscode-app/Applications/Visual%20Studio%20Code.app/Contents/Resources/app/out/vs/code/electron-sandbox/workbench/workbench.html): Implementation of frame utility functions including checksum calculation
* [server.c](vscode-file://vscode-app/Applications/Visual%20Studio%20Code.app/Contents/Resources/app/out/vs/code/electron-sandbox/workbench/workbench.html): Access Point (AP) implementation that receives and responds to frames
* [client.c](vscode-file://vscode-app/Applications/Visual%20Studio%20Code.app/Contents/Resources/app/out/vs/code/electron-sandbox/workbench/workbench.html): Client implementation that sends various test frames to the AP

## Compilation Instructions

To compile the program, use the following commands:

**gcc** **-c** **frame.c** **-o** **frame.o**

**gcc** **frame.o** **server.c** **-o** **server**

**gcc** **frame.o** **client.c** **-o** **client**

## Running the Simulation

1. First, start the server (in one terminal):

   **./server**
2. Then, run the client (in another terminal):

   **./client**

## Test Scenarios

The client executes the following test cases in sequence:

1. **Association Request/Response** : Basic connection establishment
2. **Probe Request/Response** : Network discovery
3. **RTS/CTS Exchange** : Channel reservation
4. **Data Frame Exchange** : Simple data transmission
5. **Bad FCS Test** : Error handling with corrupt frames
6. **Fragmentation Test** : Transmission of multiple fragmented frames
7. **Error Recovery Test** : Mix of correct and incorrect frames

## Expected Output

* The server will display received frame information and send appropriate responses
* The client will show progress through the test sequence and responses received
* For corrupted frames, the server will detect FCS errors and not respond
* The client will retry sending frames when no response is received (up to 3 times)

## Technical Details

* Communication: UDP sockets (Server port: 8080, Client port: 8081)
* MAC addresses: Hardcoded for simulation (can be modified in the source)
* Frame structure: IEEE 802.11 frame format with support for all required fields
* Checksum: Simple XOR-based implementation for demonstration (not CRC32 as in real Wi-Fi)

## Requirements

* C compiler (GCC recommended)
* Standard C libraries
* POSIX-compliant operating system (Linux/macOS)
