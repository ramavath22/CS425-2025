# **CS425 Assignment 3: TCP Handshake**

## Team Members

| Team members | Roll no. |
| ------------ | :------: |
| L Sree Sahithi    |  220577  |
| Ramavath Dinesh Naik        |  220866  |
| Gowthan Chand   |  220313  |

## How to Run the Code

steps to compile and run the TCP handshake client and server:

1.  **Compile the Code**
    Use the provided `Makefile` to compile both the server and client programs. Open your terminal in the assignment directory and type:

    ```bash
    make
    ```

2.  **Run the Server**
    Start the server program. You need administrator rights (root privileges) because it uses raw sockets. Type the following command in your terminal:

    ```bash
    sudo ./server
    ```

    The server will show this message:

    ```
    [+] Server listening on port 12345...
    ```

3.  **Run the Client**
    Open a **new** terminal window. Run the client program, which also requires administrator rights:
    ```bash
    sudo ./client
    ```

**Expected Output:**

- **In the client's terminal:**
  ```
  [+] Client sending SYN...
  [+] TCP Flags:  SYN: 1 ACK: 0 FIN: 0 RST: 0 PSH: 0 SEQ: 200
  [+] SYN sent
  [+] Waiting for SYN-ACK from 127.0.0.1...
  [+] TCP Flags:  SYN: 1 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 400
  [+] Received SYN-ACK from 127.0.0.1
  [+] Client sending final ACK...
  [+] TCP Flags:  SYN: 0 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 600
  [+] Final ACK sent
  [+] Handshake complete.
  ```
- **In the server's terminal:**
  ```
  [+] Server listening on port 12345...
  [+] TCP Flags: SYN: 1 ACK: 0 FIN: 0 RST: 0 PSH: 0 SEQ: 200
  [+] Received SYN from 127.0.0.1
  [+] Sent SYN-ACK
  [+] TCP Flags: SYN: 0 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 600
  [+] Received ACK, handshake complete.
  ```

## Overview

**Assignment Goal:**
The main goal of this assignment is to implement the client-side logic of a simplified TCP three-way handshake using raw sockets. Raw sockets allow direct crafting and manipulation of network packets at the IP and TCP header level.


**Handshake Procedure:**
This custom handshake simulates the standard three-step TCP connection setup, with specific sequence numbers assigned as per the assignment's requirements:

1.  **SYN Packet:** The client initiates the connection by sending a TCP packet with the SYN flag set. This packet uses a sequence number of 200.
2.  **SYN-ACK Response:**  Upon receiving the SYN, the server responds with a TCP packet that has both the SYN and ACK flags set. The server’s sequence number is set to 400, and it acknowledges the client’s SYN by sending an acknowledgment number of 201 (i.e., 200 + 1).
3.  **Final ACK:**  Finally, the client acknowledges the server’s response by sending a packet with only the ACK flag set. This packet has a sequence number of 600 and an acknowledgment number of 401 (i.e., 400 + 1).

This process completes the handshake between the client and server. The exact behavior and structure of the packets are enforced by manually constructing both the IP and TCP headers using raw socket programming.

## Implementation Details (client.cpp)

###  Key Functions

- **print_tcp_flags(struct tcphdr *tcp):** Prints out the values of key TCP flags (SYN, ACK, FIN, etc.) and the sequence number. This is helpful for verifying packet construction and understanding the handshake steps as they happen.
- **send_packet:** 
  - Constructs and sends a raw TCP packet using custom headers. 

  - It: Initializes and fills both the IP and TCP headers manually.

  - Accepts sequence/acknowledgment numbers and flag configurations (SYN, ACK).

  - Uses sendto() to transmit the packet to the destination server.

  - Also logs packet flags and a status message for easier debugging.
- **`main()`:** 
  - Orchestrates the complete client-side TCP handshake:

  - Creates a raw socket using socket(AF_INET, SOCK_RAW, IPPROTO_TCP).

  - Enables manual header handling with setsockopt() using the IP_HDRINCL flag.

  - Builds the server’s address struct using 127.0.0.1 and SERVER_PORT.

  - Sends the initial SYN packet with sequence number 200.

  - Waits to receive the SYN-ACK using recvfrom(), and validates its content.

  - Sends the final ACK packet with sequence number 600, completing the handshake.

### How it Works

- **Raw Socket Usage:** This implementation uses a raw TCP socket (SOCK_RAW with IPPROTO_TCP), giving the program full control over both the IP and TCP headers.
- **Manual Packet Construction:** 
  - Unlike higher-level socket APIs, this client constructs each packet byte-by-byte using struct   iphdr and struct tcphdr. This includes fields like:saddr and daddr (source and destination IPs)source and dest (ports),TCP flags (syn, ack, etc.),and custom sequence/acknowledgment numbers.


The following values are used in accordance with assignment requirements:

SYN: SEQ = 200

SYN-ACK (expected from server): SEQ = 400, ACK = 201

Final ACK: SEQ = 600, ACK = 401
- **Hardcoded Sequence Numbers:** 
  - The following values are used in accordance with assignment requirements:
  
  - SYN: SEQ = 200

  - SYN-ACK (expected from server): SEQ = 400, ACK = 201

  - Final ACK: SEQ = 600, ACK = 401

- **Synchronous Flow:** The client blocks on recvfrom() after sending the SYN. It only proceeds to the final ACK once a valid SYN-ACK is received and verified.

- **Robust Output Logging:**
Every major step is logged, including TCP flags, packet types being sent, and confirmation messages. These messages match the expected assignment output for both client and server terminals.

## Assumptions
- Debug statements(containing TCP flags and relevant IP info) are printed whenever ACK is sent or SYN-ACK is recieved. 
  **Why?** [https://piazza.com/class/m5h01uph1h12eb/post/153]
- Implemented client.cpp using raw sockets, assuming that the client program will be executed with sudo privileges just like server.cpp.
  **Why?** [https://piazza.com/class/m5h01uph1h12eb/post/160]
- Client resides on same IP as the Server.
  **Why?** [https://piazza.com/class/m5h01uph1h12eb/post/169]


## Testing

- **Step-by-step Checks:** Each part of the handshake (sending SYN, receiving SYN-ACK, sending ACK) was checked to make sure the packets were created correctly with the right sequence numbers.
- **Sequential Validation:** The handshake process was validated by carefully examining each stage—initiating the SYN, receiving the SYN-ACK, and completing with the ACK. Each transmitted packet was verified to ensure it carried the expected sequence and acknowledgment numbers.

- **End-to-End Execution:** Both the client and server applications were executed on the same local machine (127.0.0.1). Their console outputs were cross-checked to confirm that all steps of the three-way handshake followed the intended order and logic.
- **Full Run Test:** The client and server were run together on the same computer (localhost). The output messages printed by both programs were checked to confirm that the handshake sequence matched the expected steps and sequence numbers.

##   Challenges Faced

###   Dropped Checksum Validation Logic

During development, we initially attempted to implement **manual checksum validation** for TCP headers on the client side to ensure packet integrity. We wrote code to compute and verify checksums as per the standard TCP/IP protocol specifications.

However, we later realized that the provided `server.cpp` does **not perform any checksum validation** on the received packets. Since the server accepts packets without verifying checksums, our client-side checksum logic would have **no practical effect on the assignment's handshake flow**.

To maintain consistency with the server's logic and avoid unnecessary complexity, we **decided to drop the checksum validation feature** from our final implementation.






## Contribution of Team Members

| Team Member              | Contribution (%) | Work Done                                                    |
| :----------------------- | :--------------: | :----------------------------------------------------------- |
| Sree Sahithi <br/> (220577) |      35%         | Implemented Socket setup, SYN handling, README     |
| Dinesh Naik <br/> (220866)     |      35%         | Handled SYN-ACK recieving & parsing, Worked on Checksum implementation       |
| Gowtham Chand <br/> (220313) |      30%         | Implemented Final ACK, Debugging, README           |

## Sources Used

- Understanding Raw Sockets in C: [https://www.binarytides.com/raw-sockets-c-code-linux/](https://www.binarytides.com/raw-sockets-c-code-linux/) 
- To learn about Socket programming on Linux: [https://www.linuxhowtos.org/C_C++/socket.htm](https://www.linuxhowtos.org/C_C++/socket.htm) 
- Understanding Raw Socket - Send/Recieve: [https://cplusplus.com/forum/general/7109/](https://cplusplus.com/forum/general/7109/) 

## Declaration

We, (**Sree Sahithi, Ramavath Dinesh Naik and Gowtham chand**) declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.
