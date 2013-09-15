// File:  linuxSocket.h
// Date:  9/3/2013
// Auth:  K. Loux
// Desc:  Linux socket class.

// Standard C++ headers
#include <cstdlib>
#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sstream>

// *nix headers
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

// Local headers
#include "linuxSocket.h"

using namespace std;

//==========================================================================
// Class:			LinuxSocket
// Function:		Constant definitions
//
// Description:		Static constant definitions for the LinuxSocket class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
const unsigned int LinuxSocket::maxMessageSize = 1024;
const unsigned int LinuxSocket::maxConnections = 5;
const unsigned int LinuxSocket::selectTimeout = 5;// [sec]

//==========================================================================
// Class:			LinuxSocket
// Function:		LinuxSocket
//
// Description:		Constructor for LinuxSocket class.
//
// Input Arguments:
//		type	= SocketType
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
LinuxSocket::LinuxSocket(SocketType type, ostream& outStream) : type(type), outStream(outStream)
{
	clientMessageSize = 0;
	pthread_mutex_init(&bufferMutex, NULL);

	rcvBuffer = new unsigned char[maxMessageSize];
}

//==========================================================================
// Class:			LinuxSocket
// Function:		~LinuxSocket
//
// Description:		Destructor for LinuxSocket class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
LinuxSocket::~LinuxSocket()
{
	continueListening = false;
	if (type == SocketTCPServer)
		pthread_join(listenerThread, NULL);// TODO:  Add messaging/check return values here?
	pthread_mutex_destroy(&bufferMutex);// TODO:  Add messaging/check return values here?

	delete [] rcvBuffer;
	rcvBuffer = NULL;

	close(sock);
	outStream << "  Socket " << sock << " has been destroyed" << endl;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		Create
//
// Description:		Creates a new socket.
//
// Input Arguments:
//		port		= const unsigned short& specifying the local port to which
//					  the socket should be bound
//		target		= [optional, default ""] const string& containing the
//					  IP address to which messages will be sent; useful for
//					  determining which of several NICs to bind to
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the socket is created successfully, false otherwise
//
//==========================================================================
bool LinuxSocket::Create(const unsigned short &port, const string &target)
{
	if (IsTCP())
		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock == SOCKET_ERROR)
	{
		outStream << "  Socket creation Failed:  " << GetLastError() << endl;
		outStream << "  Port: " << port << endl;
		outStream << "  Type: " << GetTypeString(type) << endl;

		return false;
	}

	outStream << "  Created " << GetTypeString(type) << " socket with id " << sock << endl;

	if (type == SocketTCPClient)
		return Connect(AssembleAddress(port, target));
	else// TCP Servers and any UDP socket
		return Bind(AssembleAddress(port, target));
	return true;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		AssembleAddress
//
// Description:		Assembles the specified address into a socket address structure.
//
// Input Arguments:
//		port	= const unsigned short&
//		target	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		sockaddr_in pointing to the specified address and port
//
//==========================================================================
sockaddr_in LinuxSocket::AssembleAddress(const unsigned short &port, const std::string &target)
{
//	assert(!IsServer() || !target.empty());

	sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	if (target.empty())
		address.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(GetBestLocalIPAddress(target).c_str());
	else
		address.sin_addr.s_addr = inet_addr(target.c_str());//inet_addr(GetBestLocalIPAddress(target).c_str());
	address.sin_port = htons(port);

	return address;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		Bind
//
// Description:		Bind this socket to the specified port.
//
// Input Arguments:
//		address		= const sockaddr_in&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the socket is bound successfully, false otherwise
//
//==========================================================================
bool LinuxSocket::Bind(const sockaddr_in &address)
{
	if (type == SocketTCPServer)
		EnableAddressReusue();

	if (bind(sock, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		outStream << "  Bind to port " << ntohs(address.sin_port) << " failed:  " << GetLastError() << endl;
		return false;
	}

	outStream << "  Socket " << sock << " successfully bound to port " << ntohs(address.sin_port) << endl;

	if (type == SocketTCPServer)
		return Listen();

	return true;
}

//==========================================================================
// Class:			friend of LinuxSocket
// Function:		LaunchThread
//
// Description:		Listener thread entry point (launches member function).
//
// Input Arguments:
//		pThisSocket =	void* (really a pointer to a LinuxSocket)
//
// Output Arguments:
//		None
//
// Return Value:
//		void*
//
//==========================================================================
void *LaunchThread(void *pThisSocket)
{
	static_cast<LinuxSocket*>(pThisSocket)->ListenThreadEntry();
	return NULL;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		Listen
//
// Description:		Spawns a new thread and puts the socket in a listen state
//					(in the new thread).
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if successful, false otherwise
//
//==========================================================================
bool LinuxSocket::Listen(void)
{
	continueListening = true;

	// TODO:  Move this to a better place?
	// TCP severs crash if writing to a broken pipe, if we don't explicitly ignore the error
	signal(SIGPIPE, SIG_IGN);
/*	int optval(1);
	if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == SOCKET_ERROR)
		cout << "Error setting socket options" << endl;*/

	if (listen(sock, maxConnections) == SOCKET_ERROR)
	{
		outStream << "  Listen on socket ID " << sock << " failed:  " << GetLastError() << endl;
		return false;
	}

	outStream << "  Socket " << sock << " listening" << endl;

	if (pthread_create(&listenerThread, NULL, &LaunchThread, (void*)this) == 0)
	{
		outStream << "  Spawned listening thread with ID " << listenerThread << endl;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		ListenThreadEntry
//
// Description:		Listener thread entry point.  Listening, accepting
//					connections and receiving data happens in this thread,
//					but access to the data and sends are handled in the main
//					thread.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void LinuxSocket::ListenThreadEntry(void)
{
	FD_ZERO(&clients);
	FD_SET(sock, &clients);
	maxSock = sock;

	struct timeval timeout;
	timeout.tv_sec = selectTimeout;
	timeout.tv_usec = 0;

	int s;
	while (continueListening)
	{
		readSocks = clients;
		if (select(maxSock + 1, &readSocks, NULL, NULL, &timeout) == SOCKET_ERROR)
		{
			// Failed to accept connection
			// TODO:  message?  Logger is not thread-safe...
			// examples all show exit(1) or similar
			continue;
		}

		for (s = 0; s <= maxSock; s++)
		{
			if (FD_ISSET(s, &readSocks))
			{
				// Socket ID s is ready
				if (s == sock)// New connection
				{
					int newSock;
					struct sockaddr_in clientAddress;
					unsigned int size = sizeof(struct sockaddr_in);
					newSock = accept(sock, (struct sockaddr*)&clientAddress, &size);
					if (newSock == SOCKET_ERROR)
					{
						// Failed to accept connection
						// TODO:  message?  Logger is not thread-safe...
						continue;
					}

					cout << "connection from: " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << endl;

					FD_SET(newSock, &clients);
					if (newSock > maxSock)
						maxSock = newSock;
				}
				else
					HandleClient(s);
			}
		}
	}
}

//==========================================================================
// Class:			LinuxSocket
// Function:		HandleClient
//
// Description:		Handles incomming requests from client sockets.
//
// Input Arguments:
//		newSock	= int
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void LinuxSocket::HandleClient(int newSock)
{
	pthread_mutex_lock(&bufferMutex);// TODO:  Check return values?
	clientMessageSize = DoReceive(newSock);
	pthread_mutex_unlock(&bufferMutex);// TODO:  Check return values?

	// On disconnect
	if (clientMessageSize <= 0)
	{
		// TODO:  Message?  through some thread-safe means?
		FD_CLR(newSock, &clients);
		close(newSock);
	}
}

//==========================================================================
// Class:			LinuxSocket
// Function:		Connect
//
// Description:		Connect to the specified server.
//
// Input Arguments:
//		address		= const sockaddr_in&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the connection is successful, false otherwise
//
//==========================================================================
bool LinuxSocket::Connect(const sockaddr_in &address)
{
	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		outStream << "  Connect to " << ntohs(address.sin_port) << " failed:  " << GetLastError() << endl;
		return false;
	}

	outStream << "  Socket " << sock << " on port " << ntohs(address.sin_port) << " successfully connected" << endl;

	return true;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		EnableAddressReusue
//
// Description:		Sets the socket options to enable re-use of the address.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if successful, false otherwise
//
//==========================================================================
bool LinuxSocket::EnableAddressReusue(void)
{
	int one(1);
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == SOCKET_ERROR)
	{
		outStream << "  Set socket options failed for socket " << sock << ":  " << GetLastError() << endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		SetBlocking
//
// Description:		Sets the blocking mode as specified.
//
// Input Arguments:
//		blocking	= bool specifying whether socket operations should block or not
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool LinuxSocket::SetBlocking(bool blocking)
{
#ifdef WIN32
	unsigned long mode = blocking ? 0 : 1;// 1 = Non-Blocking, 0 = Blocking
	return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0)
		return false;

	flags = blocking ? (flags &~ O_NONBLOCK) : (flags | O_NONBLOCK);
	return fcntl(sock, F_SETFL, flags) == 0;
#endif
}

//==========================================================================
// Class:			LinuxSocket
// Function:		Receive
//
// Description:		Receives messages from the socket.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		int specifying bytes received, or SOCKET_ERROR on error
//
//==========================================================================
int LinuxSocket::Receive(void)
{
	if (type == SocketTCPServer)
		return clientMessageSize;

	struct sockaddr_in senderAddr;
	int bytesrcv;

	bytesrcv = DoReceive(sock, &senderAddr);
	if (bytesrcv == SOCKET_ERROR)
	{
		outStream << "  Error receiving message: " << GetLastError() << endl;
		return SOCKET_ERROR;
	}
	else if (bytesrcv == 0)
	{
		outStream << "  Received empty packet from "
			<< inet_ntoa(senderAddr.sin_addr) << ":" << ntohs(senderAddr.sin_port) << endl;
		return SOCKET_ERROR;
	}

	// Helpful for debugging, but generally we don't want it in our logs
	outStream << "  Received " << bytesrcv << " bytes from "
		<< inet_ntoa(senderAddr.sin_addr) << ":" << ntohs(senderAddr.sin_port) << endl;//*/

	return bytesrcv;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		DoReceive
//
// Description:		Receives messages from the specified socket.  Does not
//					include any error handling.  This class is geared towards
//					small messages (receivable in a single call to recv()).
//					It is possible to handle larger messages, but the calling
//					methods will have to do some work to properly reconstruct
//					the message.
//
// Input Arguments:
//		sock		= int
//		senderAddr	= struct sockaddr_in*
//
// Output Arguments:
//		None
//
// Return Value:
//		int specifying bytes received, or SOCKET_ERROR on error
//
//==========================================================================
int LinuxSocket::DoReceive(int sock, struct sockaddr_in *senderAddr)
{
	if (senderAddr)
	{
		socklen_t addrSize = sizeof(*senderAddr);
		return recvfrom(sock, rcvBuffer, maxMessageSize, 0, (struct sockaddr*)senderAddr, &addrSize);
	}
	else
	{
		return recv(sock, rcvBuffer, maxMessageSize, 0);
	}
}

//==========================================================================
// Class:			LinuxSocket
// Function:		UDPSend
//
// Description:		Sends a message to the specified address and port (UDP).
//
// Input Arguments:
//		addr		= const char* containing the destination IP for the message
//		port		= const short& specifying the destination port for the message
//		buffer		= const void* pointing to the message body contents
//		bufferSize	= const int& specifying the size of the message
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool LinuxSocket::UDPSend(const char *addr, const short &port,
	const void *buffer, const int &bufferSize)
{
	assert(!IsTCP());

	struct sockaddr_in targetAddress = AssembleAddress(port, addr);

	int bytesSent = sendto(sock, buffer, bufferSize, 0,
		(struct sockaddr*)&targetAddress, sizeof(targetAddress));

	if (bytesSent == SOCKET_ERROR)
	{
		outStream << "  Error sending UDP message: " << GetLastError() << endl;
		return false;
	}

	if (bytesSent != bufferSize)
	{
		outStream << "  Wrong number of bytes sent (UDP) to "
			<< inet_ntoa(targetAddress.sin_addr) << ":"
			<< ntohs(targetAddress.sin_port) << endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		TCPSend
//
// Description:		Sends a message to the connected server (TCP).
//
// Input Arguments:
//		buffer		= const void* pointing to the message body contents
//		bufferSize	= const int& specifying the size of the message
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool LinuxSocket::TCPSend(const void *buffer, const int &bufferSize)
{
	assert(IsTCP());

	if (IsServer())
		return TCPServerSend(buffer, bufferSize);

	int bytesSent = send(sock, buffer, bufferSize, 0);

	if (bytesSent == SOCKET_ERROR)
	{
		outStream << "  Error sending TCP message: " << GetLastError() << endl;
		return false;
	}

	if (bytesSent != bufferSize)
	{
		outStream << "  Wrong number of bytes sent (TCP)" << endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		TCPServerSend
//
// Description:		Sends a message to all of the connected clients (TCP).
//
// Input Arguments:
//		buffer		= const void* pointing to the message body contents
//		bufferSize	= const int& specifying the size of the message
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool LinuxSocket::TCPServerSend(const void *buffer, const int &bufferSize)
{
	int bytesSent, s;
	bool success(true), calledSend(false);

	for (s = 0; s <= maxSock; s++)
	{
		if (!FD_ISSET(s, &clients) || s == sock)
			continue;

		bytesSent = send(s, buffer, bufferSize, 0);
		calledSend = true;

		if (bytesSent == SOCKET_ERROR)
		{
			outStream << "  Error sending TCP message on socket " << s << ": " << GetLastError() << endl;
			success = false;
		}
		else if (bytesSent != bufferSize)
		{
			outStream << "  Wrong number of bytes sent (TCP) on socket "<< s << endl;
			success = false;
		}
	}

	return success && calledSend;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		GetLocalIPAddress
//
// Description:		Retrieves a list of all local IP addresses.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		vector<string> containing local network interface addresses
//
//==========================================================================
vector<string> LinuxSocket::GetLocalIPAddress(void)
{
	vector<string> ips;
	char host[80];

	// Get the host name
	if (gethostname(host, sizeof(host)) == SOCKET_ERROR)
	{
		//outStream << "  Error getting host name: " << GetLastError() << endl;
		return ips;
	}

	struct hostent *hostEntity = gethostbyname(host);

	if (hostEntity == 0)
	{
		//outStream << "  Bad host lookup!" << endl;
		return ips;
	}

	struct in_addr addr;

	// Return ALL addresses
	for (int i = 0; hostEntity->h_addr_list[i] != 0; ++i)
	{
		memcpy(&addr, hostEntity->h_addr_list[i], sizeof(struct in_addr));
		ips.push_back(inet_ntoa(addr));
	}

	return ips;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		GetBestLocalIPAddress
//
// Description:		Retrieves the most likely local IP address given the destination
//					address.
//
// Input Arguments:
//		destination	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string containing the best local IP
//
//==========================================================================
std::string LinuxSocket::GetBestLocalIPAddress(const string &destination)
{
	unsigned int i;
	vector<string> ips(GetLocalIPAddress());
	string compareString(destination.substr(0, destination.find_last_of('.')));
	for (i = 0; i < ips.size(); i++)
	{
		// Use the first address that matches beginning of destination
		if (ips[i].substr(0, compareString.size()).compare(compareString) == 0)
			return ips[i];
	}

	return ips[0];
}

//==========================================================================
// Class:			LinuxSocket
// Function:		GetTypeString
//
// Description:		Returns a string describing the socket type.
//
// Input Arguments:
//		type	= SocketType
//
// Output Arguments:
//		None
//
// Return Value:
//		string containing the type description
//
//==========================================================================
std::string LinuxSocket::GetTypeString(SocketType type)
{
	if (type == SocketTCPServer)
		return "TCP Server";
	else if (type == SocketTCPClient)
		return "TCP Client";
	else if (type == SocketUDPServer)
		return "UDP Server";
	else if (type == SocketUDPClient)
		return "UPD Client";
	else
		assert(false);
}

//==========================================================================
// Class:			LinuxSocket
// Function:		GetLastError
//
// Description:		Returns a string describing the last error received.  Broken
//					out into a separate function for portability.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		string containing the error description
//
//==========================================================================
std::string LinuxSocket::GetLastError(void)
{
#ifdef WIN32
	// TODO:  Implement
#else
	stringstream errorString;
	errorString << "(" << errno << ") " << strerror(errno);
	return errorString.str();
#endif
}

//==========================================================================
// Class:			LinuxSocket
// Function:		GetLock
//
// Description:		Aquires a lock on the buffer mutex.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if lock aquired, false otherwise
//
//==========================================================================
bool LinuxSocket::GetLock(void)
{
	return pthread_mutex_trylock(&bufferMutex) == 0;
}

//==========================================================================
// Class:			LinuxSocket
// Function:		ReleaseLock
//
// Description:		Releases the lock on the buffer mutex.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if lock released, false otherwise
//
//==========================================================================
bool LinuxSocket::ReleaseLock(void)
{
	return pthread_mutex_unlock(&bufferMutex) == 0;
}
