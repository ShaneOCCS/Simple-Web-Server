#include <winsock2.h>   
#include <ws2tcpip.h>    
#include <stdio.h>        
#include <string.h>      
#include <cJSON.h>       
#include <time.h>         

#pragma comment(lib, "ws2_32.lib")  // Link against Winsock library

// Sets the server socket address info
void setServer_Info(struct sockaddr_in* serverAddr) {
    serverAddr->sin_family = AF_INET;           // Use IPv4
    serverAddr->sin_addr.s_addr = INADDR_ANY;   // Accept connections from any IP
    serverAddr->sin_port = htons(8080);         // Use port 8080 (convert to network byte order)
}

// Returns appropriate HTTP response based on method and path
const char* listenForPaths(const char* method, const char* path) {
    // Handle CORS preflight requests
    if (strcmp(method, "OPTIONS") == 0) {
        return
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
    }

    // Respond to GET /API/ContactMe
    if (strcmp(path, "/API/ContactMe") == 0) {
        return
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Home Page";
    }

    // Default 404 response for unknown routes
    return
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: 9\r\n"
        "\r\n"
        "Not Found";
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[2048];             // Buffer to hold incoming HTTP request
    int clientLen = sizeof(serverAddr);

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create TCP socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Set server address configuration
    setServer_Info(&serverAddr);

    // Bind socket to IP/port
    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    // Start listening for connections (max queue = 5)
    listen(serverSocket, 5);

    printf("Listening on http://127.0.0.1:8080\n");

    // Main loop: accept and process incoming connections
    while (1) {
        // Accept a client connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddr, &clientLen);

        // Receive the HTTP request into buffer
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        // If failed to receive data, close socket and continue
        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            continue;
        }

        // Null-terminate received string
        buffer[bytesReceived] = '\0';

        // Extract HTTP method and path from request
        char method[8], path[1024];
        sscanf(buffer, "%s %s", method, path);
        printf("Request: %s %s\n", method, path);

        // Locate the beginning of JSON body (after headers)
        char *json_start = strstr(buffer, "\r\n\r\n") + 4;

        // Parse JSON body using cJSON
        cJSON *root = cJSON_Parse(json_start);
        if (root == NULL) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
            }
            closesocket(clientSocket);
            continue;
        }

        // Print parsed JSON to console (for debugging/logging)
        char *json_printed = cJSON_Print(root);
        if (json_printed != NULL) {
            printf("Parsed JSON:\n%s\n", json_printed);
            free(json_printed);
        }

        // Open log file for appending contact messages
        FILE* logFile = fopen("C:/Users/shane/CLionProjects/ContactMe/contactme.txt", "a");
        if (logFile != NULL) {
            // Extract fields from JSON
            const cJSON *fullname = cJSON_GetObjectItem(root, "fullname");
            const cJSON *email = cJSON_GetObjectItem(root, "email");
            const cJSON *message = cJSON_GetObjectItem(root, "message");

            // Get current time and format it
            time_t now;
            time(&now);
            char timestamp[26];
            ctime_s(timestamp, sizeof(timestamp), &now);
            timestamp[strlen(timestamp)-1] = '\0';  // Remove newline

            // Write data to log file
            fprintf(logFile, "[%s] New contact:\n", timestamp);
            fprintf(logFile, "  Fullname: %s\n", fullname ? fullname->valuestring : "N/A");
            fprintf(logFile, "  Email: %s\n", email ? email->valuestring : "N/A");
            fprintf(logFile, "  Message: %s\n", message ? message->valuestring : "N/A");
            fprintf(logFile, "----------------------------------------\n");

            fclose(logFile);  // Close file
        } else {
            perror("Failed to open contactme.txt");
        }

        cJSON_Delete(root);  // Free JSON object memory

        // Generate and send response back to client
        const char* response = listenForPaths(method, path);
        send(clientSocket, response, strlen(response), 0);

        // Close connection to client
        closesocket(clientSocket);
    }

    // Cleanup socket and Winsock library
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
