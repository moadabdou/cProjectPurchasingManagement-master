#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#include "tools/cJSON.h"
#include "tools/tools.h"
#include "pages/pages.h"
#include "api/api.h"
#include "assets/assets.h"
#include "tools/errors.h"

#pragma comment(lib, "ws2_32.lib")  // Link with Winsock library

#define PORT 8080

#define INDEX "GET / "
#define DASHBOARD "GET /dashboard"
#define API   "POST /api"
#define IMAGE "GET /images/"


//server  http request  handlers 
int main() {

    //intialize  the  server  
    Sessions SESSIONS;
    SESSIONS.sessions = (int *)calloc(10 ,sizeof(int));
    SESSIONS.max = 10;
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    
    int client_addr_len = sizeof(client_addr);
    char buffer[4096*2] = {0};

    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        printf("\n new requist ---------\n");
        // Accept an incoming connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        // Read client request
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read == SOCKET_ERROR) {
            printf("Receive failed. Error Code: %d\n", WSAGetLastError());
            closesocket(client_socket);
            continue;
        }
        buffer[bytes_read] = '\0';
        printf("\n recieved  : \n %s \n " , buffer );

        Cookie id_cookie = get_cookie(buffer, "id");

        printf("\n cookie : %s %s" , id_cookie.name , id_cookie.value);

        // Handle GET or POST requests
          if (strncmp(buffer, INDEX , strlen(INDEX) ) == 0 || strncmp(buffer, DASHBOARD , strlen(DASHBOARD) ) == 0) {

            //checking  cookies validity 
            char *endptr;  
            int num_id =  strtol(id_cookie.value , &endptr ,  10);

            if (*endptr == '\0'){
                printf("\n check session : %d" , check_in_sessions(SESSIONS , num_id));
                if (check_in_sessions(SESSIONS , num_id)){
                    dashboard_html(client_socket);
                }else {
                    index_html(client_socket);
                }
            }else {
                printf("\nERROR :  invalid id cookie > login page");
                index_html(client_socket);
            }

        } else if (strncmp(buffer, API, strlen(API)) == 0) {

            handle_post(client_socket, buffer,  SESSIONS);

        } else  if (strncmp(buffer, IMAGE, strlen(IMAGE)) == 0){
            handle_images(client_socket, buffer);
        }else {
            SEND_ERROR_404;
        }

        // Close the client socket after handling the request
        closesocket(client_socket);
    }

    // Cleanup and close the server socket
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
