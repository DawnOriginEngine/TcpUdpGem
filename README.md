# TcpUdpGem

一个为Open 3D Engine (O3DE) 提供TCP和UDP网络通信功能的Gem。

## 功能特性

- **UDP通信**：支持UDP消息发送和接收
- **UDP组播**：支持加入/离开组播组和组播消息发送
- **TCP通信**：支持TCP客户端和服务器模式
- **事件驱动**：基于O3DE的EBus系统进行事件通知
- **脚本支持**：所有功能都可以在脚本中使用
- **跨平台**：支持Windows、Linux、Mac等平台

## 安装

1. 将此Gem复制到你的O3DE项目的`Gems`目录中
2. 在项目的`project.json`文件中添加此Gem的引用
3. 重新生成项目文件并编译

## 组件说明

### UdpComponent

UDP网络通信组件，提供UDP消息发送、接收和组播功能。

#### 主要方法

- `Open(port)` - 打开UDP套接字并绑定到指定端口
- `Close()` - 关闭UDP套接字
- `Send(ip, port, message)` - 发送UDP消息到指定地址
- `SetReceiveCallback(callback)` - 设置消息接收回调函数
- `JoinMulticastGroup(multicastAddress)` - 加入组播组
- `LeaveMulticastGroup(multicastAddress)` - 离开组播组
- `SendMulticast(multicastAddress, port, message)` - 发送组播消息

#### 事件通知

- `OnMessageReceived(senderIp, senderPort, message)` - 接收到UDP消息时触发

### TcpComponent

TCP网络通信组件，支持客户端和服务器模式。

#### 主要方法

- `Listen(port)` - 启动TCP服务器，监听指定端口
- `Connect(ip, port)` - 连接到TCP服务器
- `Disconnect()` - 断开TCP连接
- `IsConnected()` - 检查连接状态
- `Send(message)` - 发送TCP消息
- `SetReceiveCallback(callback)` - 设置消息接收回调函数

#### 事件通知

- `OnConnected()` - 连接建立时触发
- `OnDisconnected()` - 连接断开时触发
- `OnConnectionFailed()` - 连接失败时触发
- `OnMessageReceived(message)` - 接收到TCP消息时触发

## 使用示例

### UDP通信示例

#### C++代码

```cpp
// 获取UdpComponent
AZ::Entity* entity = GetEntity();
UdpComponent* udpComponent = entity->FindComponent<UdpComponent>();

// 打开UDP套接字
udpComponent->Open(8080);

// 设置接收回调
udpComponent->SetReceiveCallback([](const AZStd::string& senderIp, AZ::u16 senderPort, const AZStd::string& message) {
    AZ_Printf("UDP", "Received from %s:%d: %s", senderIp.c_str(), senderPort, message.c_str());
});

// 发送消息
udpComponent->Send("192.168.1.100", 8081, "Hello UDP!");

// 加入组播组
udpComponent->JoinMulticastGroup("224.0.0.1");

// 发送组播消息
udpComponent->SendMulticast("224.0.0.1", 8082, "Hello Multicast!");
```

#### 脚本代码 (Lua)

```lua
-- 获取UdpComponent
local udpRequestBus = UdpRequestBus.Connect(self, self.entityId)

-- 打开UDP套接字
udpRequestBus:Open(8080)

-- 设置接收回调
udpRequestBus:SetReceiveCallback(function(senderIp, senderPort, message)
    Debug.Log("Received from " .. senderIp .. ":" .. senderPort .. ": " .. message)
end)

-- 发送消息
udpRequestBus:Send("192.168.1.100", 8081, "Hello UDP!")

-- 加入组播组
udpRequestBus:JoinMulticastGroup("224.0.0.1")

-- 发送组播消息
udpRequestBus:SendMulticast("224.0.0.1", 8082, "Hello Multicast!")
```

### TCP通信示例

#### TCP服务器

```cpp
// 获取TcpComponent
AZ::Entity* entity = GetEntity();
TcpComponent* tcpComponent = entity->FindComponent<TcpComponent>();

// 启动TCP服务器
tcpComponent->Listen(9090);

// 设置接收回调
tcpComponent->SetReceiveCallback([](const AZStd::string& message) {
    AZ_Printf("TCP Server", "Received: %s", message.c_str());
});

// 发送响应消息
tcpComponent->Send("Hello from server!");
```

#### TCP客户端

```cpp
// 获取TcpComponent
AZ::Entity* entity = GetEntity();
TcpComponent* tcpComponent = entity->FindComponent<TcpComponent>();

// 连接到TCP服务器
if (tcpComponent->Connect("192.168.1.100", 9090))
{
    // 设置接收回调
    tcpComponent->SetReceiveCallback([](const AZStd::string& message) {
        AZ_Printf("TCP Client", "Received: %s", message.c_str());
    });
    
    // 发送消息
    tcpComponent->Send("Hello from client!");
}
```

#### 脚本代码 (Lua)

```lua
-- TCP服务器
local tcpRequestBus = TcpRequestBus.Connect(self, self.entityId)

-- 启动服务器
tcpRequestBus:Listen(9090)

-- 设置接收回调
tcpRequestBus:SetReceiveCallback(function(message)
    Debug.Log("TCP Server received: " .. message)
end)

-- TCP客户端
local tcpRequestBus = TcpRequestBus.Connect(self, self.entityId)

-- 连接到服务器
if tcpRequestBus:Connect("192.168.1.100", 9090) then
    -- 设置接收回调
    tcpRequestBus:SetReceiveCallback(function(message)
        Debug.Log("TCP Client received: " .. message)
    end)
    
    -- 发送消息
    tcpRequestBus:Send("Hello from client!")
end
```

## 事件处理

### UDP事件处理

```cpp
class MyUdpHandler : public UdpNotificationBus::Handler
{
public:
    void OnMessageReceived(const AZStd::string& senderIp, AZ::u16 senderPort, const AZStd::string& message) override
    {
        AZ_Printf("UDP", "Message from %s:%d: %s", senderIp.c_str(), senderPort, message.c_str());
    }
};

// 连接事件总线
MyUdpHandler handler;
handler.BusConnect(entityId);
```

### TCP事件处理

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

// 连接事件总线
MyTcpHandler handler;
handler.BusConnect(entityId);
```

## 注意事项

1. **端口占用**：确保使用的端口没有被其他应用程序占用
2. **防火墙设置**：确保防火墙允许相应端口的网络通信
3. **组播地址**：组播地址范围通常是224.0.0.0到239.255.255.255
4. **错误处理**：建议在生产环境中添加适当的错误处理和重连机制
5. **性能考虑**：大量数据传输时考虑使用缓冲和批处理

## 故障排除

### 常见问题

1. **无法绑定端口**
   - 检查端口是否已被占用
   - 确认有足够的权限绑定端口
   - 尝试使用其他端口

2. **无法接收消息**
   - 检查防火墙设置
   - 确认网络连接正常
   - 验证IP地址和端口配置

3. **组播不工作**
   - 检查网络是否支持组播
   - 确认组播地址在有效范围内
   - 检查路由器组播设置

4. **TCP连接失败**
   - 确认服务器正在监听
   - 检查IP地址和端口是否正确
   - 验证网络连通性

## 许可证

本项目遵循Apache 2.0或MIT许可证。详见LICENSE文件。

## 贡献

欢迎提交问题报告和功能请求。如果你想贡献代码，请先创建一个issue讨论你的想法。

## 版本历史

- **v1.0.0** - 初始版本
  - UDP消息发送和接收
  - UDP组播支持
  - TCP客户端和服务器
  - 事件驱动架构
  - 脚本支持