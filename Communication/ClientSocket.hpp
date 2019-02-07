#pragma once

#include <string>
#include <Buffer.hpp>

namespace Communication
{
	class ClientSocket
	{
	public:
		virtual ~ClientSocket() = default;

		virtual int connect(const std::string& hostname, int port) = 0;
		virtual int close() = 0;
		virtual int sendBuffer(const Buffer& buffer) = 0;
		virtual int receiveBuffer(Buffer& buffer) = 0;
	};
}
