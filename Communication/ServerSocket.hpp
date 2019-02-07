#pragma once

#include "ClientSocket.hpp"

namespace Communication
{
	class ServerSocket
	{
	public:
		virtual ~ServerSocket() = default;

		virtual int bind(int port) = 0;
		virtual int listen(int clientCount) = 0;
		virtual ClientSocket* acceptClient() = 0;
		virtual int close() = 0;
	};
}
