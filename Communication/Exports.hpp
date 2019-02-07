#pragma once

#include "ClientSocket.hpp"
#include "ServerSocket.hpp"

#ifdef COMMUNICATION_EXPORTS
#define COMMUNICATION_TAG extern "C" __declspec(dllexport)
#else
#define COMMUNICATION_TAG extern "C" __declspec(dllimport)
#endif

namespace Communication
{
	COMMUNICATION_TAG	ClientSocket*	CreateClientSocket();
	COMMUNICATION_TAG	void			DeleteClientSocket(ClientSocket *);

	COMMUNICATION_TAG	ServerSocket*	CreateServerSocket();
	COMMUNICATION_TAG	void			DeleteServerSocket(ServerSocket *);
}
