# **CS425 Assignment-1:** 

## Compiling and Running codes
- rm server_grp client_grp
- make
- ./server_grp
- ./client_grp
- python3 stress_test.py

##  Assignment Features
- Implementing a TCP-based chat server that listens on a specific port
- Authentication before using chat server and maintain the list of authorized connections
- Concurrent Client Connections
- Private Messaging between two clients after handshaking
- Broadcasting the message to all clients connected to the server
- Creating, Joining and Leaving the group by any connected client
- Sending the group messages



### Features Implemented:

**TCP based server**

- initializes by creating a server socket, setting necessary socket options , binding it to port , and listening for incoming client connections. Once a client connects, the server accepts the connection.
- Authentication using a `users.txt` file which has about 1000 randomly generated users with passwords.
- Handling multiple client connections using **threads**.
- Works on entering correct commanding after authentication.
- **Private messaging** (`/msg <username> <message>`)
  - Users can send messages to themselves.
  - If the recipient is not in the chat server, the server responds with "User not found!".
  - If the command is correctly formatted, the message is successfully delivered to the recipient.
 - **Example:**
  ```
  /msg Bob Hello!
  ```
  - If Bob is in the chat server, he receives:
    ```
    Alice: Hello!
    ```
  - If Bob is not in the chat server, Alice sees:
    ```
    User not found!
    ```


- **Broadcast messaging** (`/broadcast <message>`)
  - If the user correctly formats the command, the message is successfully sent to all users and     displayed as a broadcast from the sender.

  - **Example:**
  ```
  /broadcast Hello, everyone!
  ```
  - All users in the chat server receive:
    ```
    Broadcast from Alice: Hello, everyone!
    ```


- **Group management:**
   **Create a group** (`/create_group <group_name>`)
   - If the user attempts to create an existing group, the server notifies them with "Group already exists!".
   - If the user correctly formats the command, the group is created, and the creator is automatically added as a member.

   - **Example:**
  ```
  /create_group CS425
  ```
  - If the group does not exist, the server responds:
    ```
    Group CS425 created.
    ```
  - If the group already exists:
    ```
    Group already exists!
    ```


   **Join a group** (`/join_group <group_name>`)
   - If the user attempts to join a non-existent group, the server responds with "Error: Group <group_name> does not exist".
   - If the user is already a member of the group, the server notifies them with "You are already a member of the group <group_name>!".
   - If the user correctly formats the command, they are added to the group and receive the confirmation message "You joined the group <group_name>". 

   - **Example:**
  ```
  /join_group CS425
  ```
  - If the group exists and the user is not a member:
    ```
    You joined the group CS425.
    ```
  - If the group does not exist:
    ```
    Error: Group CS425 does not exist.
    ```
  - If the user is already a member:
    ```
    You are already a member of the group CS425!
    ```

   **Leave a group** (`/leave_group <group_name>`)
   - If the user attempts to leave a non-existent group, the server responds with "Error: Group <group_name> does not exist.".
   - If the user is not a member of the group, the server notifies them with "Error: You are not a member of the group <group_name>.".
   - If the command is correctly formatted, the user is removed from the group and receives the confirmation message "You left the group <group_name>."

   - **Example:**
  ```
  /leave_group CS425
  ```
  - If the user is a member of the group:
    ```
    You left the group CS425.
    ```
  - If the user is not a member:
    ```
    Error: You are not a member of the group CS425.
    ```
  - If the group does not exist:
    ```
    Error: Group CS425 does not exist.
    ```

   **Send a group message** (`/group_msg <group_name> <message>`)
   - If the user attempts to send a message to a non-existent group, the server responds with "Error: Group <group_name> does not exist.".
   - If the user is not a member of the specified group, the server notifies them with "Error: You are not a member of the group <group_name>.".
   - If the command is correctly formatted, the group exists, and the user is a member, the message is successfully sent to all group members.

   - **Example:**
  ```
  /group_msg CS425 Hello team!
  ```
  - If the group exists and the user is a member:
    ```
    [Group ] Alice: Hello team!
    ```
  - If the group does not exist:
    ```
    Error: Group CS425 does not exist.
    ```
  - If the user is not a member:
    ```
    Error: You are not a member of the group CS425.
    ```


 
- **Graceful client disconnection handling (`/exit`).**
   - When a user exits the chat server using `/exit`, they are removed from all groups, and other users can no longer message them.
   - After exiting, the user can rejoin the chat server by running `./client_grp`, entering their username and password.

   - **Example:**
  ```
  /exit
  ```
  - The user is disconnected from the chat server, and a message is broadcasted:
    ```
    Alice has left the chat server.
    ```


- **Error handling** for incorrect commands, missing arguments, and disconnected users.
   - If the user enters an incorrect or misspelled command, the server responds with "Error! Invalid Command".
   - If a correct command is given with incorrect arguments, the server displays the correct usage message.
   - If the user tries to message a non-existent user, the server responds with "User not found".
   - If authentication fails due to incorrect username or password, the user is disconnected with "Authentication failed! Disconnecting".
   
    - **Example:**
  ```
  /invalid_command
  ```
  - If the user enters an unrecognized command, the server responds:
    ```
    Error: Invalid command!
    ```

- **Example:**
  ```
  /msg
  ```
  - If the user provides incorrect arguments for a command:
    ```
    Usage: /msg <username> <message>


### Features Not Implemented:
- **Persistent storage of messages**: server does not save message history.
- **Restricting Login in multiple terminals**: not able to login to server after being disconnected by the server in one of the windows.
- **Missing Group Notifications**: Users do not receive alerts when members join or leave a group.

---

## Design Details

### **Multithreading Approach**
- The server handles multiple client connections **concurrently** using **multithreading**.
- Each client connection is managed by spawning a **dedicated thread** using `std::thread`.
- The `handle_client()` function is executed in a separate thread for each client, allowing independent processing of commands without blocking other connections.

### **Synchronization Using Mutex Locks**
- Since multiple threads access shared resources such as `clients`, `user_sockets`, and `groups`, **mutex locks (`std::mutex`)** are used to ensure thread safety.
- **When sending messages**, a lock is applied to prevent race conditions while accessing shared data.
- **When a client disconnects**, the mutex ensures proper cleanup by removing the user from all data structures without conflicts.





## ** Implementation Details**

### **1️ High-Level Overview**
The chat server follows a **multi-threaded approach**, allowing multiple clients to connect, send messages, and interact simultaneously. The following are the key functionalities implemented in the code:
| Function                 | Description                                                                         |
| ------------------------ | ----------------------------------------------------------------------------------- |
| `main()`                | Initializes the server, sets socket options, binds to port 12345, listens for incoming connections, accepts client connections, spawns threads for each client, and ensures proper socket closure. |
| `handle_client()`       | Handles client authentication and processes commands.                            |
| `send_message()`        | Sends a message to a specific client.                                              |
| `broadcast_message()`   | Sends a message to all connected clients.                                          |
| `private_message()`     | Sends a direct message to a specific user.                                         |
| `group_message()`       | Sends a message to all members of a specified group.                               |
| `create_group()`        | Creates a new group and adds the creator as a member.                             |
| `join_group()`          | Allows a user to join an existing group.                                           |
| `leave_group()`         | Removes a user from a group.                                                       |                                                      |
                                         


### **Code Flow (Server-Side)**

```plaintext
1. Start the server.
2. Load user credentials from users.txt.
3. Bind and listen on PORT 12345.
4. Accept incoming client connections.
5. For each client:
   a. Authenticate user.
   b. If authenticated, add to active clients.
   c. Handle commands (/msg, /broadcast, /create_group, etc.).
   d. If disconnected, clean up client data.
```
```
main() 
    │       
    │── Create socket     # Initialize server
    |── Set socket options
    │── Bind to port
    │── Listen for connections
    │── For each new connection
    └── accept clients and create new client_socket         # Main accept loop
        └── For each new connection
            └── handle_client()         # New thread
```
For each thread
```
handle_client(client_socket)
    │
    ├── Authentication
    │   ├── Send "Enter username: "
    │   ├── Receive username
    │   ├── Send "Enter password: "
    │   ├── Receive password
    │   └── authenticate()
    │       └── Check against users map
    │
    ├── Client Setup
    │   ├── add clients to unordered_map         
    │   ├── Send welcome message
    │   └── Broadcast join message
    │
    ├── Message Processing Loop
    │   └── While client connected
    │       ├── Receive message
    │       │
    │       ├── If "/broadcast"
    │       │   └── broadcast()
    │       │
    │       ├── If "/msg"
    │       │   └── private_message()
    │       │
    │       ├── If "/create_group"
    │       │   └── create_group()
    │       │
    │       ├── If "/join_group"
    │       │   └── join_group()
    │       │
    │       ├── If "/group_msg"
    │       │   └── group_message()
    │       │
    │       ├── If "/leave_group"
    │       │   └── leave_group()
    │       │
    │       |── If "/exit" #leave from the server
    │       |   
    |       |
    |       └── If no command matched send "Error: Invalid command" 
    │
    └── Cleanup
        └── close(client_socket)       # Close connection
            
```

### **Code Flow (Client-Side)**

```plaintext
1. Establish a connection to the server on port 12345.
2. Provide a username and password for authentication.
3. Upon successful authentication:
   a. Launch a dedicated thread to receive incoming messages.
   b. Continuously read user input and transmit commands/messages to the server.
4. When the user enters "/exit" or presses "Ctrl + C", terminate the connection safely.
```

---


### **2 Detailed Explanation of Functions**

#### ** `handle_client(int client_socket)`**
- This function is responsible for **handling client connections**.
- **Steps Performed:**
  1. **Authentication**:
     - The server prompts the client for a **username and password**.
     - It verifies the credentials by reading from `users.txt`.
     - If authentication **fails**, the client is disconnected from the server.
     - If the user is **already logged in from another session**, they are disconnected from this   session.
  2. **Continuous Listening**:
     - After authentication, the server continuously listens for **incoming messages or commands**.
  3. **Processing Commands**:
     - The server processes client commands such as:
       - `/msg` (private messaging)
       - `/broadcast` (public messaging)
       - `/create_group`, `/join_group`, `/group_msg` (group communication)
       - `/exit` (disconnecting from the server)
  4. **Handling Disconnections**:
     - If a client **disconnects**, the server:
       - Removes them from the **active users list** (`clients`).
       - Broadcasts a message that the user has left the chat.

#### ** `send_message(int client_socket, const std::string& message)`**
- This function is used for **sending a message to a specific client**.
- **Usage**:
  - Used in **private messaging (`/msg`)**.
  - Used for sending **server responses to clients** (e.g., "Invalid command!").
- **Implementation**:
  - Calls the `send()` function to deliver the message to the intended client.



#### ** `broadcast_message(const std::string& message, int sender_socket)`**
- This function allows a client to **send a message to all connected clients**.
- **Steps Performed**:
  1. Acquires a **mutex lock** to ensure thread safety.
  2. Iterates over all connected clients.
  3. Sends the message to **everyone except the sender**.
- **Example**:
  - If a user sends `/broadcast Hello!`, all other users receive: [ Broadcast from Alice ] : Hello!  
    
    
    

#### ** `private_message(const std::string& sender, const std::string& recipient, const std::string& message)`**
- Enables **direct messaging between two users**.
- **Steps Performed**:
  1. Checks if the other user is in the chat server.
  2. If the user is **not found**, it notifies the sender (`"User not found!"`).
  3. If the user is found, sends the message to the recipient.
- **Example**:
  ```
  /msg Bob Hello!
  ```
  - If Bob is in the chat server, he receives:
    ```
    Alice: Hello!
    ```
  - If Bob is not in the chat server, Alice sees:
    ```
    User not found!
    ```

---

#### ** `group_message(const std::string& sender, const std::string& group_name, const std::string& message)`**
- Sends a **message to a group** if the sender is a member.
- **Steps Performed**:
  1. Checks if the **group exists**.
  2. If the group **does not exist**, sends `"Group not found!"`.
  3. Checks if the **user is a member** of the group.
  4. If the user **is not a member**, sends `"You are not a member of the group!"`.
  5. If the user **is a member**, broadcasts the message to **all group members**.
- **Example**:
  ```
  /group_msg cs425 Hello everyone!
  ```
  - If `cs425` exists and the sender is a member, all members see:
    ```
    [Group cs425] Alice: Hello everyone!
    ```
  - If the group does not exist:
    ```
    Group not found!
    ```
  - If the user is **not a member**:
    ```
    You are not a member of the group!
    ```

---

#### ** `main()` - Server Setup and Execution**
- **Creates the server socket, sets socket options for reconnection, and binds to port 12345.-**Listens for incoming client connections** (up to **1000 clients** simultaneously).
- **Accepts new connections** and spawns a new **thread for each client**.
- **Ensures proper socket cleanup** when the server shuts down.

---



## **Testing**

### Correctness Testing
1. Private Messages (`/msg`):
   - Sent a message to a valid user → Received.
   - Sent a message to an offline user → "User not found!"

2. Broadcast (`/broadcast`):
   - Sent broadcast → All clients received.

3. Group Chat (`/group_msg`):
   - Sent message to existing group → Group members received.
   - Sent message to non-existing group → "Group not found!"
   - Sent message as a non-member → "You are not a member of the group!"

4. Invalid Commands:
   - Sent `/wrong_command` → "Error: Invalid command!"
   - Sent incomplete commands → "Usage: /msg <username> <message>"

5. User Reconnection:
   - Exited the server and attempted to reconnect → User successfully reconnected.

---

### ** Stress Testing**
To evaluate the server’s performance under high load, the **`stress_test.py`** script(taken from online) was executed with **1000 concurrent clients**, each sending multiple messages.

- **Automates multiple logins** using users from `users.txt`.
- **Spawns concurrent client processes** to simulate real-world load.
- **Maintains long-term connections** and monitors server stability.
- **Introduces small delays** (`0.01s`) between logins to stress test effectively.
- **Logs errors and connection failures** for analysis.
- Server handles **multiple simultaneous logins** without crashes.
- Clients remain **connected** under high load.

This stress test ensures that the server can handle real-world usage efficiently.

---

### ** Summary of Testing**
| **Test Case**             | **Expected Outcome**                 | **Actual Outcome** |
|---------------------------|-------------------------------------|--------------------|
| Private message to user   | Delivered successfully              |  Pass |
| Private message to offline user | `"User not found!"` message |  Pass |
| Broadcast message        | Delivered to all clients            |  Pass |
| Broadcast with spaces    | `"Usage: /broadcast <message>"`     |  Pass |
| Group message (valid)    | Delivered to all group members      |  Pass |
| Group message (invalid)  | `"Group not found!"` error          |  Pass |
| Group message (not a member) | `"You are not a member!"` error |  Pass |
| Invalid command handling | `"Error: Invalid command!"`         |  Pass |
| `/exit` functionality    | Client properly exits and notifies others |  Pass |
| Stress test with 1000 clients | Server remains stable            |  CPU utilising 100% capacity |


---

## ** Server Restrictions**
| Feature | Limitation |
|---------|-------------|
| Max Clients | 1000 concurrent connections |
| Max Groups | memory-dependent |
| Max Users in a Group |  memory-dependent |
| Max Message Size | 1024 bytes |
| Stored Users | **In `users.txt` (No database)** |

---

## ** Challenges Faced**
### **1️ Stess testing**
-  we were not able to find the max clients and max groups that are runnning concurrently however we took a stress_test.py from online and tried to run  the server for 1000 clients operating concurrently. this python file access the usernames and passwords from user.txt and gets authorized. 
- Finally it successfully connected upto 624 clients seamlessly without any delay but took time for connecting 1000 clients. even though it did complete 1000 connections but when the cpu usage was checked in alternate terminal it was **100%**


### **2️ Handling multiple terminals**
- when we user who is connected  to server when trying to login in another terminal we tried to generate an error message `" you have already connected. disconnecting!"` but when the user leaves server in first window and tries to login in same window after disconnecting then too it is generating the same error message as `" you have already connected. disconnecting!"`



---

## Contribution of Team Members

| Team Member        | Contribution (%) | Work Done                                                                                                                              |
| ------------------ | :----------------: | -------------------------------------------------------------------------------------------------------------------------------------- |
| Dinesh Naik  <br/> (220866) | 35%              | Implemented authentication,handled edge cases like empty messages, handled multithreading, Commenting Code, debugging,written design and challenges parts in the readme document.|
| L Sree Sahithi <br/> (220577)     | 35%              | Designed server architecture,  Functionalities, modularized code, Conducted testing, debugging, and written Features and Implementation parts in the readme document.    |
| Gowtham chand <br/> (220313) | 30%              | Conducted stress testing,Implemented string parsing, written testing and remaining parts of the readme document. |

---

## Sources Referred

- [Socket programming and Threading](https://github.com/privacy-iitk/cs425-2025/tree/main/classroom-code) and [lock_guard()](https://www.geeksforgeeks.org/stdunique_lock-or-stdlock_guard-which-is-better/) from geeks for geeks.
- C++ Reference (std::thread, std::mutex)
- Stress_test.py (https://medium.com/%40nomannayeem/the-ultimate-guide-to-load-testing-with-python-from-basics-to-mastery-b5ac41f89a77)



## Declaration
We, (**Dinesh Naik, L Sree Sahithi, and Gowtham Chand**), confirm that this assignment was completed independently without plagiarism. Any external references used have been appropriately cited.

---

## Final Thoughts
This project provided **valuable experience** in **multithreading, socket programming, and concurrency handling**. Future improvements could focus on **optimizing performance, security, and scalability** to make the system more robust and efficient.

---
