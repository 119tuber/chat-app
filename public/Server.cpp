#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>
#include <mutex>
#include <json/json.h>  // JSON library for C++

// A structure to store user and message
struct ChatMessage {
    std::string user;
    std::string message;
};

std::vector<ChatMessage> messages;
std::mutex messages_mutex;

void handle_request(int new_socket) {
    char buffer[1024] = {0};
    read(new_socket, buffer, 1024);

    std::string request(buffer);
    std::string response;

    std::cout << "Received Request: " << request << std::endl;  // Debug print

    // Lock messages for thread safety
    messages_mutex.lock();

    std::string cors_headers = "Access-Control-Allow-Origin: *\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\nAccess-Control-Allow-Headers: Content-Type\n";

    if (request.find("POST /sendMessage") == 0) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string body = request.substr(body_start + 4);

            // Parsing JSON data (assuming frontend sends { "user": "username", "message": "text" })
            size_t user_pos = body.find("\"user\":\"");
            size_t message_pos = body.find("\"message\":\"");
            if (user_pos != std::string::npos && message_pos != std::string::npos) {
                // Extract the user
                user_pos += 8; // length of "user":" prefix
                size_t user_end = body.find("\"", user_pos);
                std::string user = body.substr(user_pos, user_end - user_pos);

                // Extract the message
                message_pos += 10; // length of "message":" prefix
                size_t message_end = body.find("\"", message_pos);
                std::string message = body.substr(message_pos, message_end - message_pos);

                // Add to messages vector
                messages.push_back({user, message});
                response = "HTTP/1.1 200 OK\n" + cors_headers + "Content-Type: text/plain\n\nMessage received";
            } else {
                response = "HTTP/1.1 400 Bad Request\n" + cors_headers + "Content-Type: text/plain\n\nInvalid JSON format";
            }
        } else {
            response = "HTTP/1.1 400 Bad Request\n" + cors_headers + "Content-Type: text/plain\n\nInvalid request body";
        }
    } else if (request.find("GET /getMessages") == 0) {
        response = "HTTP/1.1 200 OK\n" + cors_headers + "Content-Type: application/json\n\n[";

        // Format each message as JSON
        for (size_t i = 0; i < messages.size(); ++i) {
            std::string escaped_user = messages[i].user;
            std::string escaped_message = messages[i].message;

            // Escape double quotes
            size_t pos = 0;
            while ((pos = escaped_user.find("\"", pos)) != std::string::npos) {
                escaped_user.replace(pos, 1, "\\\"");
                pos += 2;
            }
            pos = 0;
            while ((pos = escaped_message.find("\"", pos)) != std::string::npos) {
                escaped_message.replace(pos, 1, "\\\"");
                pos += 2;
            }

            response += "{\"user\":\"" + escaped_user + "\",\"message\":\"" + escaped_message + "\"}";

            if (i != messages.size() - 1) {
                response += ",";
            }
        }

        response += "]";  // Close the JSON array
    } else if (request.find("OPTIONS /") == 0) {
        response = "HTTP/1.1 200 OK\n" + cors_headers + "Content-Type: text/plain\n\n";
    } else {
        response = "HTTP/1.1 404 Not Found\n" + cors_headers + "Content-Type: text/plain\n\nPage not found";
    }

    messages_mutex.unlock();

    std::cout << "Sending Response: " << response << std::endl;  // Debug print

    send(new_socket, response.c_str(), response.size(), 0);
    close(new_socket);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Set socket options to reuse the address and port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Bind to all available interfaces
    address.sin_port = htons(8080);         // Port 8080

    // Bind socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return -1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    // Accept incoming connections
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }
        handle_request(new_socket);
    }

    close(server_fd);
    return 0;
}
