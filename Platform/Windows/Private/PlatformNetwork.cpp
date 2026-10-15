#include <Platform/PlatformNetwork.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// Ensure proper linkage for Winsock2 library
#pragma comment(lib, "Ws2_32.lib")

namespace Platform::Network
{
    // Static WSADATA instance for lifecycle management
    static WSADATA g_wsaData;

    // Helper to translate Winsock error codes to engine eResult values
    static int32 TranslateError()
    {
        int err = ::WSAGetLastError();
        switch (err)
        {
        case WSAEWOULDBLOCK:    return eResult::WouldBlock;
        case WSAENOTCONN:
        case WSAECONNRESET:
        case WSAECONNABORTED:
        case WSAESHUTDOWN:      return eResult::ConnectionClosed;
        case WSAHOST_NOT_FOUND: return eResult::HostNotFound;
        case WSAENETUNREACH:    return eResult::NetworkUnreachable;
        case WSAEINVAL:         return eResult::InvalidArgument;
        default:                return eResult::Error;
        }
    }

    int32 Init()
    {
        int result = ::WSAStartup(MAKEWORD(2, 2), &g_wsaData);
        if (result != 0)
        {
            return eResult::Error;
        }
        return eResult::OK;
    }

    void Fini()
    {
        ::WSACleanup();
    }

    int32 CreateSocket(SocketHandle& outSocket, eAddressFamily family, eSocketType type)
    {
        int af = (family == eAddressFamily::IPv6) ? AF_INET6 : AF_INET;
        int sockType = (type == eSocketType::UDP) ? SOCK_DGRAM : SOCK_STREAM;
        int protocol = (type == eSocketType::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

        SOCKET s = ::socket(af, sockType, protocol);
        if (s == INVALID_SOCKET)
        {
            return TranslateError();
        }

        outSocket.ptr = reinterpret_cast<void*>(s);
        return eResult::OK;
    }

    int32 CloseSocket(SocketHandle socket)
    {
        SOCKET s = reinterpret_cast<SOCKET>(socket.ptr);
        if (s == INVALID_SOCKET || s == 0)
        {
            return eResult::UnvalidHandle;
        }

        int result = ::closesocket(s);
        if (result == SOCKET_ERROR)
        {
            return TranslateError();
        }
        return eResult::OK;
    }

    int32 Bind(SocketHandle socket, const char* address, uint16 port)
    {
        SOCKET s = reinterpret_cast<SOCKET>(socket.ptr);
        if (s == INVALID_SOCKET) return eResult::UnvalidHandle;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);

        if (!address || address[0] == '\0')
        {
            addr.sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
            if (::inet_pton(AF_INET, address, &addr.sin_addr) <= 0)
            {
                return eResult::InvalidArgument;
            }
        }

        if (::bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            return TranslateError();
        }

        return eResult::OK;
    }

    int32 Listen(SocketHandle socket, int32 backlog)
    {
        SOCKET s = reinterpret_cast<SOCKET>(socket.ptr);
        if (s == INVALID_SOCKET) return eResult::UnvalidHandle;

        if (::listen(s, backlog) == SOCKET_ERROR)
        {
            return TranslateError();
        }

        return eResult::OK;
    }

    int32 Accept(SocketHandle listenSocket, SocketHandle& outClientSocket)
    {
        SOCKET s = reinterpret_cast<SOCKET>(listenSocket.ptr);
        if (s == INVALID_SOCKET) return eResult::UnvalidHandle;

        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);

        SOCKET clientSock = ::accept(s, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        if (clientSock == INVALID_SOCKET)
        {
            return TranslateError();
        }

        outClientSocket.ptr = reinterpret_cast<void*>(clientSock);
        return eResult::OK;
    }

    int32 Connect(SocketHandle socket, const char* address, uint16 port)
    {
        SOCKET s = reinterpret_cast<SOCKET>(socket.ptr);
        if (s == INVALID_SOCKET) return eResult::UnvalidHandle;
        if (!address) return eResult::InvalidArgument;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);

        if (::inet_pton(AF_INET, address, &addr.sin_addr) <= 0)
        {
            return eResult::InvalidArgument;
        }

        if (::connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            return TranslateError();
        }

        return eResult::OK;
    }

    int32 Send(SocketHandle socket, const void* data, uint32 size, uint32* outBytesSent)
    {
        SOCKET s = reinterpret_cast<SOCKET>(socket.ptr);
        if (s == INVALID_SOCKET) return eResult::UnvalidHandle;
        if (!data) return eResult::InvalidArgument;

        int sent = ::send(s, reinterpret_cast<const char*>(data), static_cast<int>(size), 0);
        if (sent == SOCKET_ERROR)
        {
            return TranslateError();
        }

        if (outBytesSent)
        {
            *outBytesSent = static_cast<uint32>(sent);
        }

        return eResult::OK;
    }

    int32 Receive(SocketHandle socket, void* buffer, uint32 size, uint32* outBytesReceived)
    {
        SOCKET s = reinterpret_cast<SOCKET>(socket.ptr);
        if (s == INVALID_SOCKET) return eResult::UnvalidHandle;
        if (!buffer) return eResult::InvalidArgument;

        int received = ::recv(s, reinterpret_cast<char*>(buffer), static_cast<int>(size), 0);
        if (received == 0)
        {
            return eResult::ConnectionClosed;
        }
        if (received == SOCKET_ERROR)
        {
            return TranslateError();
        }

        if (outBytesReceived)
        {
            *outBytesReceived = static_cast<uint32>(received);
        }

        return eResult::OK;
    }

    int32 SetNonBlocking(SocketHandle socket, bool isNonBlocking)
    {
        SOCKET s = reinterpret_cast<SOCKET>(socket.ptr);
        if (s == INVALID_SOCKET) return eResult::UnvalidHandle;

        u_long mode = isNonBlocking ? 1 : 0;
        if (::ioctlsocket(s, FIONBIO, &mode) != 0)
        {
            return TranslateError();
        }

        return eResult::OK;
    }
}
