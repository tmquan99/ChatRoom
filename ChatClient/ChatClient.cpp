#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool InitWinSock()
{
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void JoinChatRoomAnnouncement(SOCKET clientSocket, string nickname)
{
	string msg = nickname + " has joined the chat room";
	int byteSent = send(clientSocket, msg.c_str(), msg.length(), 0);
	if (byteSent != SOCKET_ERROR) {
		cout << msg << endl;
	}
}

void SendClientMessage(SOCKET clientSocket, string nickname)
{
	JoinChatRoomAnnouncement(clientSocket, nickname);
	string message;

	while (true) {
		getline(cin, message);
		string msg = nickname + ": " + message;
		int byteSent = send(clientSocket, msg.c_str(), msg.length(), 0);
		if (byteSent == SOCKET_ERROR) {
			cout << "Failed to send message" << endl;
			break;
		}

		if (message == "exit") {
			cout << "Exitting the chat room" << endl;
			break;
		}
	}

	closesocket(clientSocket);
}

void ReceiveMessage(SOCKET clientSocket)
{
	char buffer[4096];
	int recvLength;
	string msg = "";
	while (true) {
		recvLength = recv(clientSocket, buffer, 4096, 0);
		if (recvLength <= 0) {
			cout << "Disconnected from server" << endl;
			break;
		}
		else {
			msg = string(buffer, recvLength);
			cout << msg << endl;
		}
	}

	closesocket(clientSocket);
}

int main()
{
	//////// Initialize WinSock
	if (!InitWinSock()) {
		cout << "Failed to initialize WinSock" << endl;
		return 1;
	}
	else {
		cout << "WinSock initialized" << endl;
	}

	//////// Create a socket
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		cout << "Failed to create socket" << endl;
		WSACleanup();
		return 1;
	}
	else {
		cout << "Socket created" << endl;
	}

	//////// Get the nickname
	cout << "Enter your nickname: " << endl;
	string nickname;
	getline(cin, nickname);
	system("cls");

	//////// Connect to server
	sockaddr_in serverAddr;
	int port = 8080;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr.s_addr);

	if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "Failed to connect to server" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	else {
		thread senderthread(SendClientMessage, clientSocket, nickname);
		thread receiver(ReceiveMessage, clientSocket);
		senderthread.join();
		receiver.join();
	}

	//////// Close the socket
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
