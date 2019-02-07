#include "ServerSocketImpl.hpp"
#include "ClientSocketImpl.hpp"
#include "Exports.hpp"
#include "ScopeGuard.hpp"


COMMUNICATION_TAG Communication::ServerSocket* CreateServerSocket()
{
	return new Communication::ServerSocketImpl();
}

COMMUNICATION_TAG void DeleteServerSocket(Communication::ServerSocket* sock)
{
	delete sock;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


namespace Communication
{
	ServerSocketImpl::ServerSocketImpl()
	{
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

	}
	ServerSocketImpl::~ServerSocketImpl()
	{
		close();
	}
	
	int ServerSocketImpl::bind(int port)
	{
		if (listener != INVALID_SOCKET)
		{
			_log_("Un socket a fost deja binded.");
			return ERROR_ALREADY_ASSIGNED;
		}

		if (int error = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result); error)
		{
			_log_("Getaddrinfo fail cu error = ", error);
			return error;
		}
		ScopeGuard freeInfo([this] { freeaddrinfo(result); });

		listener = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (listener == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			_log_("Nu s-a putut crea socketul, error = ", error);
			return error;
		}

		if (int error = ::bind(listener, result->ai_addr, int(result->ai_addrlen)); error == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			_log_("Nu s-a reusit bind-ul la ", result->ai_addr->sa_data, ", error = ", error);
			return error;
		}
		return ERROR_SUCCESS;
	}

	int ServerSocketImpl::listen(int clientCount)
	{
		if (int error = ::listen(listener, clientCount); error == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			_log_("Nu s-a reusit listen cu backlog = ", clientCount, ", error = ", error);
			return error;
		}
		return ERROR_SUCCESS;
	}
	
	ClientSocket* ServerSocketImpl::acceptClient()
	{
		SOCKET client = accept(listener, NULL, NULL);
		if (client == INVALID_SOCKET)
		{
			_log_("Nu s-a reusit acceptarea, error = ", WSAGetLastError());
			return nullptr;
		}
		return new ClientSocketImpl(client);
	}

	int ServerSocketImpl::close()
	{
		listener = INVALID_SOCKET;
		return _close(listener);
	}
}
