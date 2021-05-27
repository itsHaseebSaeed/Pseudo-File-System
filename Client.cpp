#include <iostream> 
#include<WS2tcpip.h>
#include<sstream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main()
{
	// Setup WinScok
	WSADATA winSocData;
	WORD version = MAKEWORD(2, 2);

	int didWSStart = WSAStartup(version, &winSocData);
	if (didWSStart != 0)
	{
		cout << "WSA didn't startup! " << endl;
		return 0;
	}

	// Create Socket
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "TCP Client Socket Failed: " << WSAGetLastError() << endl;
		return 0;
	}
	// Take user info about server
	//string ip;
	//int port;
	//cout << "Entre ip address of the Server: ";
	//getline(cin, ip);
	//cout << "Enter port no: ";
	//cin >> port;
	//cin.ignore();
	//cout << ip << "." << port<<endl;

	//bind port and ip to socket
	struct sockaddr_in clientSockData;
	clientSockData.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &clientSockData.sin_addr);
	clientSockData.sin_port = htons(90);

	//create connection with server
	int didconnect = connect(clientSocket, (sockaddr*)&clientSockData, sizeof(clientSockData));

	if (didconnect == SOCKET_ERROR)
	{
		cout << "Did not connect: " << WSAGetLastError() << endl;
		return 0;
	}
	cout << "Connected" << endl;

	string send_buff, res;
	char recv_buff[1024];
	while (true)
	{
		ZeroMemory(recv_buff, 1024);

		//recieve data from Server
		int didrecv = recv(clientSocket, recv_buff, 1024, 0);
		res = recv_buff;

		if (didrecv == SOCKET_ERROR)
			cout << "Couldn't recieve message from server: " << endl;
		else if (res.compare("exit") == 0)
			break;
		else if (res.compare("cls") == 0)
			system("cls");
		else
			cout << recv_buff;

		//get the command from user and send it to the client
		getline(cin, send_buff);

		int didsend = send(clientSocket, send_buff.c_str(), send_buff.length(), 0);
		if (didsend == SOCKET_ERROR)
			cout << "Couldn't send :" << send_buff << endl;


	}

	closesocket(clientSocket);
	WSACleanup();
}
