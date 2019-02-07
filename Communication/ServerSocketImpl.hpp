#pragma once

#include "Socket.hpp"
#include "ServerSocket.hpp"

namespace Communication
{
	class ServerSocketImpl
		: public Socket
		, public ServerSocket
	{
		SOCKET listener = INVALID_SOCKET;
		addrinfo hints, *result = nullptr;

	public:
		ServerSocketImpl();
		~ServerSocketImpl();

		virtual int bind(int port) override;
		virtual int listen(int clientCount) override;
		virtual ClientSocket* acceptClient() override;
		virtual int close() override;
	};
}
