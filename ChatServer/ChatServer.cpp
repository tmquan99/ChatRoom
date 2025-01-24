#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

bool InitWinSock()
{
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients) {
	//////// Send & Recieve messages
	char buffer[4096];

	while (true) {
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived <= 0) {
			cout << "Client disconnected" << endl;
			break;
		}

		string message = string(buffer, bytesReceived);
		cout << "Message from " << message << endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}

	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end()) {
		clients.erase(it);
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

	/////// Create a socket
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		cout << "Failed to create socket" << endl;
		WSACleanup();
		return 1;
	}
	else {
		cout << "Socket created" << endl;
	}

	/////// Bind the socket to an IP address and port
	sockaddr_in serverAddr;
	int port = 8080;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	if (InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr.s_addr) != 1){
		cout << "Invalid IP address" << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	else {
		cout << "IP address is valid" << endl;
	}
	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "Failed to bind the socket" << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	else {
		cout << "Socket bound" << endl;
	}

	//////// Listen for incoming connections
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "Failed to listen" << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	else {
		cout << "Listening on port " << port << endl;
	}

	//////// Accept a new connection
	vector<SOCKET> clients;

	while (true) {
		SOCKET clientSocket = accept(serverSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			cout << "Failed to accept connection" << endl;
		}
		else {
			cout << "Connection accepted" << endl;
			clients.push_back(clientSocket);
			thread t1(InteractWithClient, clientSocket, ref(clients));
			t1.detach();
		}
	}

	//////// Close the socket
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
