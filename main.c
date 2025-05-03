#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

void setServer_Info(struct sockaddr_in* serverAddr) {
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_addr.s_addr = INADDR_ANY;
    serverAddr->sin_port = htons(8080);
}

const char* listenForPaths(const char* method, const char* path) {
    if (strcmp(method, "OPTIONS") == 0) {
        return
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
    }

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
    char buffer[2048];
    int clientLen = sizeof(serverAddr);

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    setServer_Info(&serverAddr);
    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    printf("Listening on http://127.0.0.1:8080\n");
while (1) {
    clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddr, &clientLen);
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        continue;
    }

    buffer[bytesReceived] = '\0';
    char method[8], path[1024];
    sscanf(buffer, "%s %s", method, path);
    printf("Request: %s %s\n", method, path);

    char *json_start = strstr(buffer, "\r\n\r\n") + 4;

    cJSON *root = cJSON_Parse(json_start);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
        }
        closesocket(clientSocket);
        continue;
    }

    char *json_printed = cJSON_Print(root);
    if (json_printed != NULL) {
        printf("Parsed JSON:\n%s\n", json_printed);
        free(json_printed);
    }

    FILE* logFile = fopen("C:/Users/shane/CLionProjects/ContactMe/contactme.txt", "a");
    if (logFile != NULL) {
        const cJSON *fullname = cJSON_GetObjectItem(root, "fullname");
        const cJSON *email = cJSON_GetObjectItem(root, "email");
        const cJSON *message = cJSON_GetObjectItem(root, "message");

        time_t now;
        time(&now);
        char timestamp[26];
        ctime_s(timestamp, sizeof(timestamp), &now);
        timestamp[strlen(timestamp)-1] = '\0';

        fprintf(logFile, "[%s] New contact:\n", timestamp);
        fprintf(logFile, "  Fullname: %s\n", fullname ? fullname->valuestring : "N/A");
        fprintf(logFile, "  Email: %s\n", email ? email->valuestring : "N/A");
        fprintf(logFile, "  Message: %s\n", message ? message->valuestring : "N/A");
        fprintf(logFile, "----------------------------------------\n");

        fclose(logFile);
    } else {
        perror("Failed to open contactme.txt");
    }

    cJSON_Delete(root);

    const char* response = listenForPaths(method, path);
    send(clientSocket, response, strlen(response), 0);
    closesocket(clientSocket);
}
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}