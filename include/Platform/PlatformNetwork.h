#pragma once
#include <Platform/PlatformCore.h>

namespace Platform::Network {

    enum eResult : int32
    {
        PLATFORM_RESULT(PLATFORM_NETWORK_RESULT_BIT),

    };

};


/*

/// @brief Wrapper handle for platform socket resources.
DEFINE_HANDLE(Socket); // SocketHandle

/// @brief Defines the address family for the socket.
enum class eAddressFamily
{
    IPv4,
    IPv6
};

/// @brief Defines the protocol type for the socket.
enum class eSocketType
{
    TCP,
    UDP
};



struct PLATFORM_NETWORK_API PlatformNetwork
{
    PlatformNetwork() = delete;
public:

    static iResult Init();
    static void Fini();

public:

    /// @brief Creates a new socket and returns its handle.
    /// @param outSocket Reference to the socket handle to be populated on success.
    /// @param family The address family (IPv4 or IPv6).
    /// @param type The socket type (TCP or UDP).
    static iResult CreateSocket(SocketHandle& outSocket, eAddressFamily family, eSocketType type);

    /// @brief Closes an existing socket and cleans up its OS-level resources.
    /// @param socket The handle of the socket to close.
    static iResult CloseSocket(SocketHandle socket);

    /// @brief Binds a socket to a specific local IP address and port.
    /// @param socket The socket handle.
    /// @param address Null-terminated string containing the IP address to bind to (e.g., "127.0.0.1").
    /// @param port The local port number.
    static iResult Bind(SocketHandle socket, const char* address, uint16_t port);

    /// @brief Places the socket in a state to listen for incoming connections (TCP only).
    /// @param socket The socket handle.
    /// @param backlog The maximum length of the queue of pending connections.
    static iResult Listen(SocketHandle socket, int backlog = 10);

    /// @brief Accepts an incoming connection request on a listening socket.
    /// @param listenSocket The listening socket handle.
    /// @param outClientSocket Reference to store the newly connected client's socket handle.
    static iResult Accept(SocketHandle listenSocket, SocketHandle& outClientSocket);

    /// @brief Initiates a connection to a remote host.
    /// @param socket The socket handle.
    /// @param address Null-terminated string containing the remote IP address.
    /// @param port The remote port number.
    static iResult Connect(SocketHandle socket, const char* address, uint16_t port);

    /// @brief Sends data over a connected socket.
    /// @param socket The socket handle.
    /// @param data Pointer to the buffer containing data to send.
    /// @param size The number of bytes to send.
    /// @param outBytesSent Pointer to store the actual number of bytes transmitted.
    static iResult Send(SocketHandle socket, const void* data, uint32_t size, uint32_t* outBytesSent);

    /// @brief Receives data from a connected socket.
    /// @param socket The socket handle.
    /// @param buffer Pointer to the buffer where received data will be stored.
    /// @param size The maximum number of bytes to read.
    /// @param outBytesReceived Pointer to store the actual number of bytes read.
    static iResult Receive(SocketHandle socket, void* buffer, uint32_t size, uint32_t* outBytesReceived);

    /// @brief Toggles non-blocking mode for the socket.
    /// @param socket The socket handle.
    /// @param isNonBlocking True to enable non-blocking mode, false for blocking.
    static iResult SetNonBlocking(SocketHandle socket, bool isNonBlocking);
};
*/