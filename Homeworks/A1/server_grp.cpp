#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>



#define PORT 12345
#define BUFFER_SIZE 1024



std::mutex mtx; // Mutex for thread safety
std::unordered_map<int, std::string> clients; // Socket -> Username
std::unordered_map<std::string, int> user_sockets; // Username -> Socket
std::unordered_map<std::string, std::unordered_set<std::string>> groups; // Group Name -> Members
std::unordered_map<std::string, std::mutex> group_mutexes; // Group Name -> Mutex



// Send message to a specific client

    void send_message(int client_socket, const std::string& message) {
        send(client_socket, message.c_str(), message.size(), 0);
    }

// Broadcast message to all clients

    void broadcast_message(const std::string& message, int sender_socket) {
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto& client : clients) {
            if (client.first != sender_socket) {
                // send_message(client.first, "[Broadcast] " + message);
                send_message(client.first, message);
            }
        }
    }

// Send private message to a specific user

    void private_message(const std::string& sender, const std::string& recipient, const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        if (user_sockets.find(recipient) != user_sockets.end()) {
            send_message(user_sockets[recipient],  sender + ": " + message);
        } 
        else {
            send_message(user_sockets[sender], "User not found!");
        }
    }

// Send message to a group

    void group_message(const std::string& sender, const std::string& group_name, const std::string& message) {
                                                    
        std::lock_guard<std::mutex> lock(mtx); // This ensures that access to shared resources (clients map) is thread-safe.
        
        // Lock the specific group mutex to ensure thread safety for this group
        std::lock_guard<std::mutex> group_lock(group_mutexes[group_name]);
    
    
        if (groups.find(group_name) == groups.end()) {
            send_message(user_sockets[sender], "Group not found!");
            return;
        }
    
        if (groups[group_name].find(sender) == groups[group_name].end()) {
            send_message(user_sockets[sender], "You are not a member of the group " + group_name + "!");
            return;
        }
    
        for (const auto& member : groups[group_name]) {
                if (user_sockets.find(member) != user_sockets.end() && member != sender) {
                    send_message(user_sockets[member], "[Group " + group_name + "] " + sender + ": " + message);
                }
        }
    }

// Handle client commands
    char buffer[BUFFER_SIZE];
    void handle_client(int client_socket) {
        std::string username;
    
    // Authentication
        send_message(client_socket, "Enter username: ");

        memset(buffer, 0, BUFFER_SIZE);
        if( recv(client_socket, buffer, BUFFER_SIZE, 0)<=0){
            close(client_socket);
            return;
        }
        username = buffer;
        send_message(client_socket, "Enter password: ");

        memset(buffer, 0, BUFFER_SIZE);
        if(recv(client_socket, buffer, BUFFER_SIZE, 0)<=0){
            close(client_socket);

            return;
        }
        
    std::string password = buffer;
    

    // Check credentials from users.txt
    std::ifstream user_file("users.txt");
    std::string line, stored_user, stored_pass;
    bool auth_success = false;
    while (std::getline(user_file, line)) {
        std::istringstream iss(line);

        std::getline(iss, stored_user, ':');
        std::getline(iss, stored_pass);
        if (username == stored_user && password == stored_pass) {
            auth_success = true;
            break;
        }
    }
    user_file.close();

    if (!auth_success) {
        send_message(client_socket, "Authentication failed. Disconnecting.");
        close(client_socket);
        return;
    }

    {   // Add user to active clients
        std::lock_guard<std::mutex> lock(mtx);

        clients[client_socket] = username;
        user_sockets[username] = client_socket;
    }
    send_message(client_socket, "Welcome to the Chat server, " + username);


    // Notify others
        std::string join_msg =  username + " has joined the chat!\n";
        broadcast_message(join_msg, client_socket);


    while (true) {

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(mtx);
            clients.erase(client_socket);
            user_sockets.erase(username);
            close(client_socket);
            return;
        }
        
        std::string message = buffer;        
        std::istringstream iss(message);
        std::string command;
        iss >> command;



        if (command != "/broadcast" && command != "/msg" && command != "/create_group" &&
            command != "/join_group" && command != "/leave_group" && command != "/group_msg" && command != "/exit" ) {

            send_message(client_socket, "Error, Invalid command!");
            continue;
        }


        
        if (command == "/broadcast") {
            std::string msg;
            std::getline(iss, msg);
            msg.erase(0, msg.find_first_not_of(" \t\r\n")); // Trim left spaces

            if(msg.empty()){
                send_message(client_socket,"Usage: /broadcast <message>");

            }

            else{
            broadcast_message("broadcast from " +username + ": " + msg, client_socket);
            }
        } 


        
        else if (command == "/msg") {
            std::string recipient;
            iss >> recipient;
            std::string msg;
            std::getline(iss, msg);
            msg.erase(0, msg.find_first_not_of(" \t\r\n")); // Trim left spaces
            
            if (recipient.empty() || msg.empty()) {
                send_message(client_socket, "Usage: /msg <username> <message>");
            }

            else if (user_sockets.find(recipient) == user_sockets.end()) {
                send_message(client_socket, "User not found!");
            } 

            else {
                send_message(user_sockets[recipient],username + ": " + msg);
            }
        }
 


        else if (command == "/create_group") {  
            std::string group_name;

            if (!(iss >> group_name)){
                send_message(client_socket, "Usage: /create_group <group_name>");

            }
            else { 
                // Read the group name after the command

                std::lock_guard<std::mutex> lock(mtx);
                if(groups.find(group_name)!=groups.end()){
                     send_message(client_socket, "Group already exists!");
                }
                else{
                    groups[group_name].insert(username);
                    send_message(client_socket, "Group " + group_name + " created ."); // Extra space before the period
                }
            }
        }
        


        else if (command == "/join_group") {  
            std::string group_name;

            if (iss >> group_name) { 
            std::lock_guard<std::mutex> lock(mtx);

                // Check if the group exists
                if (groups.find(group_name) == groups.end()) {

                    send_message(client_socket, "Error: Group " + group_name + " does not exist.");
                    return;
                }
                    // Check if user is already part of the group
                else if (groups[group_name].find(username) != groups[group_name].end()) {

                    send_message(client_socket, " You are already a member of the group " + group_name + "!");
                }
                else {
                    groups[group_name].insert(username);                        send_message(client_socket, "You joined the group " + group_name + " .");
                }
            }

            else {
                send_message(client_socket, "Usage: /join_group <group_name>");
            }
        }

    

        
        else if (command == "/leave_group") {  
            std::string group_name;
            
            if (iss >> group_name) { 
                // Read the group name after the command
                std::lock_guard<std::mutex> lock(mtx);
        
                // Check if the group exists
                if (groups.find(group_name) == groups.end()) {
                    send_message(client_socket, "Error: Group " + group_name + " does not exist.");
                    return;
                }

                // Check if the user is part of the group
                else if (groups[group_name].find(username) == groups[group_name].end()) {
                    send_message(client_socket, "Error: You are not a member of the group " + group_name);
                    return;
                }

                // Remove user from the group
                groups[group_name].erase(username);
                send_message(client_socket, "You left the group " + group_name + ".");
            }

            else {
                send_message(client_socket, "Usage: /leave_group <group_name>");
            }
        }

        
        else if (command == "/group_msg") {
            std::string group_name;
            iss >> group_name;
            std::string msg;
            std::getline(iss, msg);
            msg.erase(0, msg.find_first_not_of(" \t\r\n")); // Trim left spaces

            {
            std::lock_guard<std::mutex> lock(mtx);
                if (group_name.empty() || msg.empty()) {
                    send_message(client_socket, "Usage: /group_msg <group_name> <message>");
                }
        
             // Check if the group exists
                else if (groups.find(group_name) == groups.end()) {
                    send_message(client_socket, "Error: Group " + group_name + " does not exist.");
                    continue;
                }

             // Check if the user is a member of the group
                else if (groups[group_name].find(username) == groups[group_name].end()) {
                    send_message(client_socket, "Error: You are not a member of the group " + group_name);
                    continue;
                }
        
            }

            group_message(username, group_name, msg);
        }

        else if (command == "/exit") {
            std::string leave_message = username + " has left the chat server ";
            broadcast_message(leave_message, client_socket);
            close(client_socket);
            return;
        }
    }

     close(client_socket);  // Proper cleanup
  
}


int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int opt=1;

    // Set socket options 
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

         if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_socket, 10) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

    std::cout << "Server is listening on port " << PORT << std::endl;

    

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
            if (client_socket < 0) {
                perror("Client connection failed");
                continue;
            }

        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);  // Proper cleanup

    return 0;
}











































