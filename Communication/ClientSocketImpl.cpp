#include "ClientSocketImpl.hpp"
#include "Exports.hpp"
#include "ScopeGuard.hpp"


COMMUNICATION_TAG Communication::ClientSocket* CreateClientSocket()
{
	return new Communication::ClientSocketImpl();
}

COMMUNICATION_TAG void DeleteClientSocket(Communication::ClientSocket* sock)
{
	delete sock;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


namespace Communication
{
	ClientSocketImpl::ClientSocketImpl()
	{
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	}

	ClientSocketImpl::ClientSocketImpl(SOCKET socket): socket(socket)
	{
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	}

	ClientSocketImpl::~ClientSocketImpl()
	{
		close();
	}
	
	int ClientSocketImpl::connect(const std::string& hostname, int port)
	{
		if (socket != INVALID_SOCKET)
		{
			_log_("Un socket a fost deja creat.");
			return ERROR_ALREADY_ASSIGNED;
		}

		if (int error = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &result); error)
		{
			_log_("Getaddrinfo fail cu error = ", error);
			return error;
		}
		ScopeGuard freeInfo([this] { freeaddrinfo(result); });

		socket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (socket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			_log_("Nu s-a putut crea socketul, error = ", error);
			return error;
		}

		if (int error = ::connect(socket, result->ai_addr, int(result->ai_addrlen)); error == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			_log_("Nu s-a reusit conectarea la ", result->ai_addr->sa_data, ", error = ", error);
			return error;
		}
		return ERROR_SUCCESS;
	}

	int ClientSocketImpl::close()
	{
		socket = INVALID_SOCKET;
		return _close(socket);
	}

	int ClientSocketImpl::sendBuffer(const Buffer& buffer)
	{
		if (socket == INVALID_SOCKET)
		{
			_log_("Socket-ul nu este valid pentru trimitere de date.");
			return ERROR_INVALID_HANDLE;
		}


		int ret = send(socket, static_cast<const char*>(buffer.operator const void *()), int(buffer.getSize()), 0);
		if (ret == SOCKET_ERROR)
		{
			ret = WSAGetLastError();
			_log_("Nu s-a reusit trimiterea de ", buffer.getSize(), " bytes la ", result->ai_addr, ", error = ", ret);
			return ret;
		}
		return ERROR_SUCCESS;
	}

	int ClientSocketImpl::receiveBuffer(Buffer& buffer)
	{
		if (socket == INVALID_SOCKET)
		{
			_log_("Socket-ul nu este valid pentru primire de date.");
			return ERROR_INVALID_HANDLE;
		}


		constexpr int tempBufferSize = 512;
		char received[tempBufferSize];
		std::vector<std::vector<char>> tempBuffers;

		// Read parts of the full buffer
		int ret = recv(socket, received, tempBufferSize, 0);
		while (ret && ret != SOCKET_ERROR)
		{
			if (ret < tempBufferSize)
				break;

			tempBuffers.push_back(std::vector<char>(tempBufferSize));
			std::memcpy(&tempBuffers.back()[0], received, tempBufferSize);
			ret = recv(socket, received, tempBufferSize, 0);
		}
		if (ret == SOCKET_ERROR)
		{
			ret = WSAGetLastError();
			_log_("Apelul recv a intors eroarea ", ret);
			return ret;
		}

		// Merge the parts into the full buffer
		const int fullBufferSize = int(tempBuffers.size()) * tempBufferSize + ret;
		char* fullBuffer = static_cast<char *>(std::malloc(fullBufferSize));
		if (fullBuffer == nullptr)
		{
			_log_("Nu s-a putut aloca o zona de memorie de ", fullBufferSize, " pentru a putea stoca buffer-ul.");
			return ERROR_OUTOFMEMORY;
		}
		int offset = 0;
		for (auto& vec : tempBuffers)
		{
			std::memcpy(fullBuffer + offset, &vec[0], tempBufferSize);
			offset += tempBufferSize;
		}
		std::memcpy(fullBuffer + offset, received, ret);	// Merge the last bit

		buffer = Buffer(std::move(static_cast<void *>(fullBuffer)));
		return ERROR_SUCCESS;
	}
}
