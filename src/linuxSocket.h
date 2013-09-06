// File:  linuxSocket.h
// Date:  9/3/2013
// Auth:  K. Loux
// Desc:  Linux socket class.

#ifndef LINUX_SOCKET_H_
#define LINUX_SOCKET_H_

// Standard C++ headers
#include <string>
#include <vector>
#include <iostream>

// pThread headers
#include <ptrhead.h>

class LinuxSocket
{
public:
	enum SocketType
	{
		SocketTCPServer,
		SocketTCPClient,
		SocketUDPServer,
		SocketUDPClient
	};

	LinuxSocket(SocketType type = SocketTCPClient, std::ostream &outStream = std::cout);
	~LinuxSocket();

	bool Create(const unsigned short &port, const std::string &target = "");

	bool SetBlocking(bool blocking);

	int Receive(void);

	// NOTE:  If type == SocketTCPServer, calling method MUST aquire and release mutex when using GetLastMessage
	const unsigned char *GetLastMessage() const { clientMessageSize = 0; return rcvBuffer; };

	bool GetLock(void);
	bool ReleaseLock(void);

	bool UDPSend(const char *addr, const short &port, void *buffer, const int &bufferSize);// UDP version
	bool TCPSend(void *buffer, const int &bufferSize);// TCP version

	bool IsTCP(void) const { return type == SocketTCPServer || type == SocketTCPClient; };
	bool IsServer(void) const { return type == SocketTCPServer || type == SocketUDPServer; };

	static const int SOCKET_ERROR = -1;

private:
	static const unsigned int maxMessageSize;
	static const unsigned int maxConnections;

	const SocketType type;
	std::ostream &outStream;

	unsigned char *rcvBuffer;
	int sock;

	bool Bind(const sockaddr_in &address);
	bool Listen(void);
	bool Connect(const sockaddr_in &address);
	bool EnableAddressReusue(void);

	static std::vector<std::string> GetLocalIPAddress(void) const;
	static std::string GetBestLocalIPAddress(const std::string &destination) const;
	static sockaddr_in AssembleAddress(const unsigned short &port, const std::string &target = "");
	static std::string GetTypeString(SocketType type);
	static std::string GetLastError(void);

	int DoReceive(int sock, struct sockaddr_in *senderAddr = NULL);

	// TCP server methods and members
	void ListenThreadEntry(void*);
	void HandleClient(int newSock);

	volatile bool continueListening;
	volatile int clientMessageSize;
	pthread_t listenerThread;
	pthread_mutex_t bufferMutex;
	fd_set clients;
	fd_set readSocks;
	int maxSock;
};

#endif// LINUX_SOCKET_H_