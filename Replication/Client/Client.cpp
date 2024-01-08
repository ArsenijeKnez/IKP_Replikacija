#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <combaseapi.h>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define REP1_PORT 27016
#define REP2_PORT 22222 //27017

#define SERVER_IP_ADDERESS "127.0.0.1"
#define IP_ADDRESS_LEN 16
#define BUFFER_SIZE 1024

sockaddr_in serverAddress;
int sockAddrLen = sizeof(struct sockaddr);

SOCKET clientSocket;

bool RegisterService(int ServiceID);
bool SendData(int ServiceID, char* data, int dataSize);
void ReceiveData(void* data, int dataSize);
bool InitializeWindowsSockets();

int main()
{
    int serviceID = 0;

    printf("Welcome to replication service.\n");

    while(true)
    {
        InitializeWindowsSockets();

        memset((char*)&serverAddress, 0, sizeof(serverAddress));

        printf("Select one of the options: \n");
        printf("    1.Register service \n");
        printf("    2.Send data \n");
        printf("    3.Callback \n");
        printf("    4.Close \n");

        char input[10];
        int option = 0;
        if (gets_s(input, sizeof(input)) != nullptr)
            option = atoi(input);

        if (option == 1) {
            if (serviceID != 0) {
                int iResult = closesocket(clientSocket);
                if (iResult == SOCKET_ERROR)
                {
                    printf("closesocket failed with error: %d\n", WSAGetLastError());
                    gets_s(input, sizeof(input));
                    return 1;
                }

                iResult = WSACleanup();
                if (iResult == SOCKET_ERROR)
                {
                    printf("WSACleanup failed with error: %ld\n", WSAGetLastError());
                    gets_s(input, sizeof(input));
                    return 1;
                }
            }

            printf("Enter service ID to reqister to a replicator (1 or 2): \n");
            int id = 0;
            while (id != 1 && id != 2) {
                char input[10];
                if (gets_s(input, sizeof(input)) != nullptr)
                     id = atoi(input);
               
                if(id != 1 && id != 2) {
                    printf("Invalid input. Please enter 1 or 2.");
                    continue;
                }
                serviceID = id;
                if (!RegisterService(id))
                {
                    gets_s(input, sizeof(input));
                    return 1;
                }
            }
        }
        else if(option == 2){
            if (serviceID == 0) {
                printf("Service not reqisterd\n");
                continue;
            }
            printf("Enter message: ");
            char outgoingBuffer[BUFFER_SIZE];
            gets_s(outgoingBuffer, BUFFER_SIZE);

            if (!SendData(serviceID, outgoingBuffer, strlen(outgoingBuffer)))
            { 
                gets_s(input, sizeof(input));
                return 1; 
            }


            /*while (true) {

                printf("Enter message (type \"end\" to finish):\n");
                char outgoingBuffer[BUFFER_SIZE];
                gets_s(outgoingBuffer, BUFFER_SIZE);

                if (outgoingBuffer == "end")
                    break;

                if (!SendData(serviceID, outgoingBuffer, strlen(outgoingBuffer)))
                    return 1;
            }*/
        }
        else if (option == 3) {
            if (serviceID == 0) {
                printf("Service not reqisterd\n");
                continue;
            }

            //ReceiveData(void* data, int dataSize);
        }
        else if (option == 4) {
            if (serviceID != 0) {
                int iResult = closesocket(clientSocket);
                if (iResult == SOCKET_ERROR)
                {
                    printf("closesocket failed with error: %d\n", WSAGetLastError());
                    gets_s(input, sizeof(input));
                    return 1;
                }

                iResult = WSACleanup();
                if (iResult == SOCKET_ERROR)
                {
                    printf("WSACleanup failed with error: %ld\n", WSAGetLastError());
                    gets_s(input, sizeof(input));
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}

bool SendData(int ServiceID, char* data, int dataSize)
{
    int iResult = send(clientSocket, data, dataSize + 1, 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }
    printf("Message sent to replicator%d.\n",ServiceID);
    return true;
}

void ReceiveData(void* data, int dataSize) 
{

}

bool RegisterService(int ServiceID) 
{
    int serverPort = 0;

    if (ServiceID == 1) {
        printf("Trying to reqister to replicator1.\n");
        serverPort = REP1_PORT;
    }
    else if (ServiceID == 2) {
        printf("Trying to reqister to replicator2.\n");
        serverPort = REP2_PORT;
    }

    clientSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDERESS);
    serverAddress.sin_port = htons((u_short)serverPort);

    if (connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(clientSocket);
        WSACleanup();
    }
    char outgoingBuffer[BUFFER_SIZE] = "New process reqisterd!";

    int iResult = send(clientSocket, outgoingBuffer, (int)strlen(outgoingBuffer) + 1, 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    printf("Reqisterd successfully.\n");
    return true;
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return false;
    }
    return true;
}