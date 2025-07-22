# TcpUdpGem

A Gem that provides TCP and UDP network communication functionality for Open 3D Engine (O3DE).

## Features

- **UDP Communication**: Support for UDP message sending and receiving
- **UDP Multicast**: Support for joining/leaving multicast groups and sending multicast messages
- **TCP Communication**: Support for TCP client and server modes
- **Event-Driven**: Event notifications based on O3DE's EBus system
- **Script Support**: All functionality can be used in scripts
- **Cross-Platform**: Support for Windows, Linux, Mac and other platforms

## Installation

1. Copy this Gem to the `Gems` directory of your O3DE project
2. Add a reference to this Gem in your project's `project.json` file
3. Regenerate project files and compile

## Component Description

### UdpComponent

UDP network communication component that provides UDP message sending, receiving, and multicast functionality.

#### Main Methods

- `Open(port)` - Open UDP socket and bind to specified port
- `Close()` - Close UDP socket
- `Send(ip, port, message)` - Send UDP message to specified address
- `SetReceiveCallback(callback)` - Set message receive callback function
- `JoinMulticastGroup(multicastAddress)` - Join multicast group
- `LeaveMulticastGroup(multicastAddress)` - Leave multicast group
- `SendMulticast(multicastAddress, port, message)` - Send multicast message

#### Event Notifications

- `OnMessageReceived(senderIp, senderPort, message)` - Triggered when UDP message is received

### TcpComponent

TCP network communication component that supports client and server modes.

#### Main Methods

- `Listen(port)` - Start TCP server and listen on specified port
- `Connect(ip, port)` - Connect to TCP server
- `Disconnect()` - Disconnect TCP connection
- `IsConnected()` - Check connection status
- `Send(message)` - Send TCP message
- `SetReceiveCallback(callback)` - Set message receive callback function

#### Event Notifications

- `OnConnected()` - Triggered when connection is established
- `OnDisconnected()` - Triggered when connection is disconnected
- `OnConnectionFailed()` - Triggered when connection fails
- `OnMessageReceived(message)` - Triggered when TCP message is received

## Usage Examples

### UDP Communication Example

#### C++ Code

```cpp
// Get UdpComponent
AZ::Entity* entity = GetEntity();
UdpComponent* udpComponent = entity->FindComponent<UdpComponent>();

// Open UDP socket
udpComponent->Open(8080);

// Set receive callback
udpComponent->SetReceiveCallback([](const AZStd::string& senderIp, AZ::u16 senderPort, const AZStd::string& message) {
    AZ_Printf("UDP", "Received from %s:%d: %s", senderIp.c_str(), senderPort, message.c_str());
});

// Send message
udpComponent->Send("192.168.1.100", 8081, "Hello UDP!");

// Join multicast group
udpComponent->JoinMulticastGroup("224.0.0.1");

// Send multicast message
udpComponent->SendMulticast("224.0.0.1", 8082, "Hello Multicast!");
```

#### Script Code (Lua)

```lua
-- Get UdpComponent
local udpRequestBus = UdpRequestBus.Connect(self, self.entityId)

-- Open UDP socket
udpRequestBus:Open(8080)

-- Set receive callback
udpRequestBus:SetReceiveCallback(function(senderIp, senderPort, message)
    Debug.Log("Received from " .. senderIp .. ":" .. senderPort .. ": " .. message)
end)

-- Send message
udpRequestBus:Send("192.168.1.100", 8081, "Hello UDP!")

-- Join multicast group
udpRequestBus:JoinMulticastGroup("224.0.0.1")

-- Send multicast message
udpRequestBus:SendMulticast("224.0.0.1", 8082, "Hello Multicast!")
```

### TCP Communication Example

#### TCP Server

```cpp
// Get TcpComponent
AZ::Entity* entity = GetEntity();
TcpComponent* tcpComponent = entity->FindComponent<TcpComponent>();

// Start TCP server
tcpComponent->Listen(9090);

// Set receive callback
tcpComponent->SetReceiveCallback([](const AZStd::string& message) {
    AZ_Printf("TCP Server", "Received: %s", message.c_str());
});

// Send response message
tcpComponent->Send("Hello from server!");
```

#### TCP Client

```cpp
// Get TcpComponent
AZ::Entity* entity = GetEntity();
TcpComponent* tcpComponent = entity->FindComponent<TcpComponent>();

// Connect to TCP server
if (tcpComponent->Connect("192.168.1.100", 9090))
{
    // Set receive callback
    tcpComponent->SetReceiveCallback([](const AZStd::string& message) {
        AZ_Printf("TCP Client", "Received: %s", message.c_str());
    });
    
    // Send message
    tcpComponent->Send("Hello from client!");
}
```

#### Script Code (Lua)

```lua
-- TCP Server
local tcpRequestBus = TcpRequestBus.Connect(self, self.entityId)

-- Start server
tcpRequestBus:Listen(9090)

-- Set receive callback
tcpRequestBus:SetReceiveCallback(function(message)
    Debug.Log("TCP Server received: " .. message)
end)

-- TCP Client
local tcpRequestBus = TcpRequestBus.Connect(self, self.entityId)

-- Connect to server
if tcpRequestBus:Connect("192.168.1.100", 9090) then
    -- Set receive callback
    tcpRequestBus:SetReceiveCallback(function(message)
        Debug.Log("TCP Client received: " .. message)
    end)
    
    -- Send message
    tcpRequestBus:Send("Hello from client!")
end
```

## Event Handling

### UDP Event Handling

```cpp
class MyUdpHandler : public UdpNotificationBus::Handler
{
public:
    void OnMessageReceived(const AZStd::string& senderIp, AZ::u16 senderPort, const AZStd::string& message) override
    {
        AZ_Printf("UDP", "Message from %s:%d: %s", senderIp.c_str(), senderPort, message.c_str());
    }
};

// Connect to event bus
MyUdpHandler handler;
handler.BusConnect(entityId);
```

### TCP Event Handling

```cpp
class MyTcpHandler : public TcpNotificationBus::Handler
{
public:
    void OnConnected() override
    {
        AZ_Printf("TCP", "Connected to server");
    }
    
    void OnDisconnected() override
    {
        AZ_Printf("TCP", "Disconnected from server");
    }
    
    void OnConnectionFailed() override
    {
        AZ_Printf("TCP", "Connection failed");
    }
    
    void OnMessageReceived(const AZStd::string& message) override
    {
        AZ_Printf("TCP", "Received: %s", message.c_str());
    }
};

// Connect to event bus
MyTcpHandler handler;
handler.BusConnect(entityId);
```

## Important Notes

1. **Port Usage**: Ensure the ports you use are not occupied by other applications
2. **Firewall Settings**: Ensure the firewall allows network communication on the corresponding ports
3. **Multicast Addresses**: Multicast address range is typically 224.0.0.0 to 239.255.255.255
4. **Error Handling**: It's recommended to add appropriate error handling and reconnection mechanisms in production environments
5. **Performance Considerations**: Consider using buffering and batch processing for large data transfers

## Troubleshooting

### Common Issues

1. **Cannot bind port**
   - Check if the port is already in use
   - Confirm you have sufficient permissions to bind the port
   - Try using a different port

2. **Cannot receive messages**
   - Check firewall settings
   - Confirm network connection is normal
   - Verify IP address and port configuration

3. **Multicast not working**
   - Check if the network supports multicast
   - Confirm multicast address is in valid range
   - Check router multicast settings

4. **TCP connection failed**
   - Confirm server is listening
   - Check if IP address and port are correct
   - Verify network connectivity

## License

This project follows the Apache 2.0 or MIT license. See LICENSE file for details.

## Contributing

Welcome to submit issue reports and feature requests. If you want to contribute code, please create an issue first to discuss your ideas.

## Version History

- **v1.0.0** - Initial version
  - UDP message sending and receiving
  - UDP multicast support
  - TCP client and server
  - Event-driven architecture
  - Script support