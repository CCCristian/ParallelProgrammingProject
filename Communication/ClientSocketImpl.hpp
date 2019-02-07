#pragma once

#include "Socket.hpp"
#include "ClientSocket.hpp"

namespace Communication
{
	class ClientSocketImpl
		: public Socket
		, public ClientSocket
	{
		SOCKET socket = INVALID_SOCKET;
		addrinfo hints, *result = nullptr;

	public:
		ClientSocketImpl();
		ClientSocketImpl(SOCKET socket);
		~ClientSocketImpl();

		virtual int connect(const std::string& hostname, int port) override;
		virtual int close() override;
		virtual int sendBuffer(const Buffer& buffer) override;
		virtual int receiveBuffer(Buffer& buffer) override;
	};
}
