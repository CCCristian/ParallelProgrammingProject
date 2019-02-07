#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Buffer.hpp>

namespace Communication
{
	class Socket
	{
	protected:
		int _close(SOCKET socket)
		{
			if (socket == INVALID_SOCKET)
				return ERROR_SUCCESS;

			if (int error = ::shutdown(socket, SD_BOTH); error == SOCKET_ERROR)
			{
				error = WSAGetLastError();
				_log_("Socketul nu poate apela shutdown, error = ", error);
				return error;
			}
			if (int error = ::closesocket(socket); error == SOCKET_ERROR)
			{
				error = WSAGetLastError();
				_log_("Socketul nu se poate inchide, error = ", error);
				return error;
			}

			return ERROR_SUCCESS;
		}
	};
}
