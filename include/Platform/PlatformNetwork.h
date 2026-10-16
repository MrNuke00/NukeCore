#pragma once
#include <Platform/PlatformCore.h>

namespace Platform::Network
{
    // =========================================================================
    // Network Error Codes
    // =========================================================================
    enum eResult : int32
    {
        PLATFORM_RESULT(PLATFORM_NETWORK_RESULT_BIT),

        // Common Network Subsystem Errors
        WouldBlock,       // Operation would block (Non-blocking socket pending)
        ConnectionClosed, // Remote peer closed the connection
        HostNotFound,     // Failed to resolve hostname or IP address
        NetworkUnreachable
    };

    // =========================================================================
    // Opaque Handles & Enums
    // =========================================================================
    DEFINE_HANDLE(Socket);

    enum class eAddressFamily : int32
    {
        IPv4,
        IPv6
    };

    enum class eSocketType : int32
    {
        TCP,
        UDP
    };

    // =========================================================================
    // Subsystem Lifecycle
    // =========================================================================
    int32 Init();
    void  Fini();

    // =========================================================================
    // Socket Operations
    // =========================================================================

    /// @brief Creates a new socket and returns its handle.
    /// @param outSocket Reference to the socket handle to be populated on success.
    /// @param family The address family (IPv4 or IPv6).
    /// @param type The socket type (TCP or UDP).
    int32 CreateSocket(SocketHandle& hSocket, eAddressFamily Family, eSocketType Type);

    /// @brief Closes an existing socket and cleans up its OS-level resources.
    /// @param socket The handle of the socket to close.
    int32 CloseSocket(SocketHandle hSocket);

    /// @brief Binds a socket to a specific local IP address and port.
    /// @param socket The socket handle.
    /// @param address Null-terminated string containing the IP address to bind to (e.g., "0.0.0.0").
    /// @param port The local port number.
    int32 Bind(SocketHandle hSocket, const char* address, uint16 port);

    /// @brief Places the socket in a state to listen for incoming connections (TCP only).
    /// @param socket The socket handle.
    /// @param backlog The maximum length of the queue of pending connections.
    int32 Listen(SocketHandle hSocket, int32 backlog = 10);

    /// @brief Accepts an incoming connection request on a listening socket.
    /// @param listenSocket The listening socket handle.
    /// @param outClientSocket Reference to store the newly connected client's socket handle.
    int32 Accept(SocketHandle hListenSocket, SocketHandle& hClientSocket);

    /// @brief Initiates a connection to a remote host.
    /// @param socket The socket handle.
    /// @param address Null-terminated string containing the remote IP address.
    /// @param port The remote port number.
    int32 Connect(SocketHandle hSocket, const char* address, uint16 port);

    /// @brief Sends data over a connected socket.
    /// @param socket The socket handle.
    /// @param data Pointer to the buffer containing data to send.
    /// @param size The number of bytes to send.
    /// @param outBytesSent Pointer to store the actual number of bytes transmitted.
    int32 Send(SocketHandle hSocket, const void* data, uint32 size, uint32* outBytesSent);

    /// @brief Receives data from a connected socket.
    /// @param socket The socket handle.
    /// @param buffer Pointer to the buffer where received data will be stored.
    /// @param size The maximum number of bytes to read.
    /// @param outBytesReceived Pointer to store the actual number of bytes read.
    int32 Receive(SocketHandle hSocket, void* buffer, uint32 size, uint32* outBytesReceived);

    /// @brief Toggles non-blocking mode for the socket.
    /// @param socket The socket handle.
    /// @param isNonBlocking True to enable non-blocking mode, false for blocking.
    int32 SetNonBlocking(SocketHandle hSocket, bool isNonBlocking);
}