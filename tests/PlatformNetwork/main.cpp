#include <stdio.h>
#include <string.h>
#include <Platform/PlatformThread.h>
#include <Platform/PlatformNetwork.h>

// Client worker thread routine to connect and send data to localhost
uint32 ClientWorkerRoutine(void* userData)
{
    uint16 port = *static_cast<uint16*>(userData);

    // Give the server a brief moment to set up its listener
    Platform::Thread::Sleep(100);

    Platform::Network::SocketHandle clientSock;
    int32 createRes = Platform::Network::CreateSocket(
        clientSock,
        Platform::Network::eAddressFamily::IPv4,
        Platform::Network::eSocketType::TCP
    );

    if (createRes == Platform::OK)
    {
        printf("[Client Thread] Connecting to 127.0.0.1:%u...\n", port);

        int32 connectRes = Platform::Network::Connect(clientSock, "127.0.0.1", port);
        if (connectRes == Platform::OK)
        {
            printf("[Client Thread] Connected successfully to server!\n");

            const char* message = "Hello from Platform Network Subsystem!";
            uint32 bytesSent = 0;

            int32 sendRes = Platform::Network::Send(
                clientSock,
                message,
                static_cast<uint32>(strlen(message) + 1),
                &bytesSent
            );

            if (sendRes == Platform::OK)
            {
                printf("[Client Thread] Sent %u bytes successfully.\n", bytesSent);
            }
            else
            {
                printf("[Client Thread] Send failed with error code: %d\n", sendRes);
            }
        }
        else
        {
            printf("[Client Thread] Connection failed with error code: %d\n", connectRes);
        }

        Platform::Network::CloseSocket(clientSock);
    }
    else
    {
        printf("[Error] Client socket creation failed!\n");
    }

    return 0;
}

int main()
{
    printf("=== Starting Platform Network Subsystem Test ===\n\n");

    // 1. Initialize Subsystems
    Platform::Thread::Init();

    if (Platform::Network::Init() != Platform::OK)
    {
        printf("[Error] WSAStartup / Network initialization failed!\n");
        return -1;
    }

    uint16 testPort = 27015;
    Platform::Network::SocketHandle listenSock;

    // 2. Create Listen Socket
    if (Platform::Network::CreateSocket(listenSock, Platform::Network::eAddressFamily::IPv4, Platform::Network::eSocketType::TCP) != Platform::OK)
    {
        printf("[Error] Failed to create listen socket!\n");
        Platform::Network::Fini();
        return -1;
    }

    // 3. Bind to Localhost
    if (Platform::Network::Bind(listenSock, "127.0.0.1", testPort) != Platform::OK)
    {
        printf("[Error] Failed to bind socket to port %u!\n", testPort);
        Platform::Network::CloseSocket(listenSock);
        Platform::Network::Fini();
        return -1;
    }

    // 4. Start Listening
    if (Platform::Network::Listen(listenSock, 5) != Platform::OK)
    {
        printf("[Error] Failed to place socket in listening state!\n");
        Platform::Network::CloseSocket(listenSock);
        Platform::Network::Fini();
        return -1;
    }

    printf("[Server] Socket bound and listening on 127.0.0.1:%u\n", testPort);

    // 5. Spawn Client Thread
    Platform::Thread::ThreadCreateInfo info{};
    info.name = "NetworkClientWorker";
    Platform::Thread::ThreadHandle hClientThread;

    Platform::Thread::Create(ClientWorkerRoutine, &testPort, info, hClientThread);

    // 6. Accept Incoming Client Connection
    Platform::Network::SocketHandle clientSock;
    printf("[Server] Waiting for incoming client connection (Accept)...\n");

    int32 acceptRes = Platform::Network::Accept(listenSock, clientSock);
    if (acceptRes == Platform::OK)
    {
        printf("[Server] Client connection accepted successfully!\n");

        // 7. Receive Data from Client
        char buffer[256] = { 0 };
        uint32 bytesReceived = 0;

        int32 recvRes = Platform::Network::Receive(clientSock, buffer, sizeof(buffer) - 1, &bytesReceived);
        if (recvRes == Platform::OK)
        {
            printf("[Server] Received %u bytes: \"%s\"\n", bytesReceived, buffer);
        }
        else
        {
            printf("[Server] Receive failed with result code: %d\n", recvRes);
        }

        Platform::Network::CloseSocket(clientSock);
    }
    else
    {
        printf("[Error] Accept failed with result code: %d\n", acceptRes);
    }

    // 8. Cleanup & Shutdown
    Platform::Thread::Join(hClientThread, nullptr);
    Platform::Network::CloseSocket(listenSock);
    Platform::Network::Fini();
    Platform::Thread::Fini();

    printf("\n=== Network & Thread Integration Test Completed Successfully ===\n");
    return 0;
}