////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2008 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/SocketTCP.hpp>
#include <SFML/Network/IPAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <algorithm>
#include <iostream>


namespace sf
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
SocketTCP::SocketTCP() :
mySocket(SocketHelper::InvalidSocket())
{

}


////////////////////////////////////////////////////////////
/// Change the blocking state of the socket
////////////////////////////////////////////////////////////
void SocketTCP::SetBlocking(bool Blocking)
{
    // Make sure our socket is valid
    if (!IsValid())
        Create();

    SocketHelper::SetBlocking(mySocket, Blocking);
}


////////////////////////////////////////////////////////////
/// Connect to another computer on a specified port
////////////////////////////////////////////////////////////
bool SocketTCP::Connect(unsigned short Port, const IPAddress& HostAddress)
{
    // Make sure our socket is valid
    if (!IsValid())
        Create();

    // Build the host address
    sockaddr_in SockAddr;
    memset(SockAddr.sin_zero, 0, sizeof(SockAddr.sin_zero));
    SockAddr.sin_addr.s_addr = inet_addr(HostAddress.ToString().c_str());
    SockAddr.sin_family      = AF_INET;
    SockAddr.sin_port        = htons(Port);

    // Connect
    if (connect(mySocket, reinterpret_cast<sockaddr*>(&SockAddr), sizeof(SockAddr)) == -1)
    {
        // Error...
        std::cerr << "Failed to connect socket to host " << HostAddress << std::endl;
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
/// Listen to a specified port for incoming data or connections
////////////////////////////////////////////////////////////
bool SocketTCP::Listen(unsigned short Port)
{
    // Make sure our socket is valid
    if (!IsValid())
        Create();

    // Build the address
    sockaddr_in SockAddr;
    memset(SockAddr.sin_zero, 0, sizeof(SockAddr.sin_zero));
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    SockAddr.sin_family      = AF_INET;
    SockAddr.sin_port        = htons(Port);

    // Bind the socket to the specified port
    if (bind(mySocket, reinterpret_cast<sockaddr*>(&SockAddr), sizeof(SockAddr)) == -1)
    {
        // Not likely to happen, but...
        std::cerr << "Failed to bind socket to port " << Port << std::endl;
        return false;
    }

    // Listen to the bound port
    if (listen(mySocket, 0) == -1)
    {
        // Oops, socket is deaf
        std::cerr << "Failed to listen to port " << Port << std::endl;
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
/// Wait for a connection (must be listening to a port).
/// This function will block if the socket is blocking
////////////////////////////////////////////////////////////
Socket::Status SocketTCP::Accept(SocketTCP& Connected, IPAddress* Address)
{
    // Address that will be filled with client informations
    sockaddr_in ClientAddress;
    SocketHelper::LengthType Length = sizeof(ClientAddress);

    // Accept a new connection
    Connected = accept(mySocket, reinterpret_cast<sockaddr*>(&ClientAddress), &Length);

    // Check errors
    if (!Connected.IsValid())
    {
        if (Address)
            *Address = IPAddress();

        return SocketHelper::GetErrorStatus();
    }

    // Fill address if requested
    if (Address)
        *Address = IPAddress(inet_ntoa(ClientAddress.sin_addr));

    return Socket::Done;
}


////////////////////////////////////////////////////////////
/// Send an array of bytes to the host (must be connected first)
////////////////////////////////////////////////////////////
Socket::Status SocketTCP::Send(const char* Data, std::size_t Size)
{
    // First check that socket is valid
    if (!IsValid())
        return Socket::Error;

    // Check parameters
    if (Data && Size)
    {
        // Loop until every byte has been sent
        int Sent = 0;
        int SizeToSend = static_cast<int>(Size);
        for (int Length = 0; Length < SizeToSend; Length += Sent)
        {
            // Send a chunk of data
            Sent = send(mySocket, Data + Length, SizeToSend - Length, 0);

            // Check if an error occured
            if (Sent <= 0)
                return SocketHelper::GetErrorStatus();
        }

        return Socket::Done;
    }
    else
    {
        // Error...
        std::cerr << "Cannot send data over the network (invalid parameters)" << std::endl;
        return Socket::Error;
    }
}


////////////////////////////////////////////////////////////
/// Receive an array of bytes from the host (must be connected first).
/// This function will block if the socket is blocking
////////////////////////////////////////////////////////////
Socket::Status SocketTCP::Receive(char* Data, std::size_t MaxSize, std::size_t& SizeReceived)
{
    // First clear the size received
    SizeReceived = 0;

    // Check that socket is valid
    if (!IsValid())
        return Socket::Error;

    // Check parameters
    if (Data && MaxSize)
    {
        // Receive a chunk of bytes
        int Received = recv(mySocket, Data, static_cast<int>(MaxSize), 0);

        // Check the number of bytes received
        if (Received > 0)
        {
            SizeReceived = static_cast<std::size_t>(Received);
            return Socket::Done;
        }
        else if (Received == 0)
        {
            return Socket::Disconnected;
        }
        else
        {
            return SocketHelper::GetErrorStatus();
        }
    }
    else
    {
        // Error...
        std::cerr << "Cannot receive data from the network (invalid parameters)" << std::endl;
        return Socket::Error;
    }
}


////////////////////////////////////////////////////////////
/// Send a packet of data to the host (must be connected first)
////////////////////////////////////////////////////////////
Socket::Status SocketTCP::Send(Packet& PacketToSend)
{
    // Let the packet do custom stuff before sending it
    PacketToSend.OnSend();

    // First send the packet size
    Uint32 PacketSize = htonl(PacketToSend.GetDataSize());
    Send(reinterpret_cast<const char*>(&PacketSize), sizeof(PacketSize));

    // Send the packet data
    if (PacketSize > 0)
    {
        return Send(PacketToSend.GetData(), PacketToSend.GetDataSize());
    }
    else
    {
        return Socket::Done;
    }
}


////////////////////////////////////////////////////////////
/// Receive a packet from the host (must be connected first).
/// This function will block if the socket is blocking
////////////////////////////////////////////////////////////
Socket::Status SocketTCP::Receive(Packet& PacketToReceive)
{
    // We start by getting the size of the incoming packet
    Uint32      PacketSize = 0;
    std::size_t Received   = 0;
    if (myPendingPacketSize < 0)
    {
        Socket::Status Status = Receive(reinterpret_cast<char*>(&PacketSize), sizeof(PacketSize), Received);
        if (Status != Socket::Done)
            return Status;

        PacketSize = ntohl(PacketSize);
    }
    else
    {
        // There is a pending packet : we already know its size
        PacketSize = myPendingPacketSize;
    }

    // Clear the user packet
    PacketToReceive.Clear();

    // Then loop until we receive all the packet data
    char Buffer[1024];
    while (myPendingPacket.GetDataSize() < PacketSize)
    {
        // Receive a chunk of data
        Uint32 SizeToGet = std::min(PacketSize - myPendingPacket.GetDataSize(), static_cast<Uint32>(sizeof(Buffer)));
        Socket::Status Status = Receive(Buffer, SizeToGet, Received);
        if (Status != Socket::Done)
        {
            // We must save the size of the pending packet until we can receive its content
            if (Status == Socket::NotReady)
                myPendingPacketSize = PacketSize;
            return Status;
        }

        // Append it into the packet
        myPendingPacket.Append(Buffer, Received);
    }

    // We have received all the datas : we can copy it to the user packet, and clear our internal packet
    PacketToReceive = myPendingPacket;
    myPendingPacket.Clear();
    myPendingPacketSize = -1;

    // Let the packet do custom stuff after data reception
    PacketToReceive.OnReceive();

    return Socket::Done;
}


////////////////////////////////////////////////////////////
/// Close the socket
////////////////////////////////////////////////////////////
bool SocketTCP::Close()
{
    if (!SocketHelper::Close(mySocket))
    {
        std::cerr << "Failed to close socket" << std::endl;
        return false;
    }

    mySocket = SocketHelper::InvalidSocket();
    return true;
}


////////////////////////////////////////////////////////////
/// Check if the socket is in a valid state ; this function
/// can be called any time to check if the socket is OK
////////////////////////////////////////////////////////////
bool SocketTCP::IsValid() const
{
    return mySocket != SocketHelper::InvalidSocket();
}


////////////////////////////////////////////////////////////
/// Comparison operator ==
////////////////////////////////////////////////////////////
bool SocketTCP::operator ==(const SocketTCP& Other) const
{
    return mySocket == Other.mySocket;
}


////////////////////////////////////////////////////////////
/// Comparison operator !=
////////////////////////////////////////////////////////////
bool SocketTCP::operator !=(const SocketTCP& Other) const
{
    return mySocket != Other.mySocket;
}


////////////////////////////////////////////////////////////
/// Comparison operator <.
/// Provided for compatibility with standard containers, as
/// comparing two sockets doesn't make much sense...
////////////////////////////////////////////////////////////
bool SocketTCP::operator <(const SocketTCP& Other) const
{
    return mySocket < Other.mySocket;
}


////////////////////////////////////////////////////////////
/// Construct the socket from a socket descriptor
/// (for internal use only)
////////////////////////////////////////////////////////////
SocketTCP::SocketTCP(SocketHelper::SocketType Descriptor) :
mySocket           (Descriptor),
myPendingPacketSize(-1)
{
    // Set blocking by default (should always be the case anyway)
    if (IsValid())
        SetBlocking(true);
}


////////////////////////////////////////////////////////////
/// Create the socket
////////////////////////////////////////////////////////////
void SocketTCP::Create()
{
    // Get a new socket descriptor
    mySocket = socket(PF_INET, SOCK_STREAM, 0);

    if (IsValid())
    {
        // To avoid the "Address already in use" error message when trying to bind to the same port
        char Yes = 1;
        if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &Yes, sizeof(int)) == -1)
        {
            std::cerr << "Failed to set socket option \"reuse address\" ; "
                      << "binding to a same port may fail if too fast" << std::endl;
        }

        // Set blocking by default (should always be the case anyway)
        SetBlocking(true);
    }

    // Reset the pending packet
    myPendingPacket.Clear();
    myPendingPacketSize = -1;
}

} // namespace sf
