
#include <UdpComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzNetworking/Utilities/IpAddress.h>
#include <AzNetworking/UdpTransport/DtlsEndpoint.h>
#include <AzNetworking/ConnectionLayer/IConnection.h>
#include <AzCore/Console/ILogger.h>

// 平台相关的头文件用于组播
#ifdef AZ_PLATFORM_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(AZ_PLATFORM_LINUX) || defined(AZ_PLATFORM_ANDROID)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#elif defined(AZ_PLATFORM_MAC) || defined(AZ_PLATFORM_IOS)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace TcpUdpGem
{
    AZ_COMPONENT_IMPL(UdpComponent, "UdpComponent", "{61EC1C44-F5F9-4FAE-BDE0-3315DAD968F1}");

    void UdpComponent::Activate()
    {
        UdpRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();

        m_udpSocket = AZStd::make_unique<AzNetworking::UdpSocket>();
        if (!m_udpSocket->Open(m_port, AzNetworking::UdpSocket::CanAcceptConnections::False, AzNetworking::TrustZone::ExternalClientToServer))
        {
            AZ_Error("UdpComponent", false, "Failed to open UDP socket on port %d", m_port);
            m_udpSocket.reset();
            return;
        }
    }

    void UdpComponent::Deactivate()
    {
        // 离开所有已加入的组播组
        for (const auto& multicastAddress : m_joinedMulticastGroups)
        {
            LeaveMulticastGroup(multicastAddress);
        }
        m_joinedMulticastGroups.clear();
        
        if (m_udpSocket && m_udpSocket->IsOpen())
        {
            m_udpSocket->Close();
            m_udpSocket.reset();
        }
        AZ::TickBus::Handler::BusDisconnect();
        UdpRequestBus::Handler::BusDisconnect(GetEntityId());
    }

    void UdpComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
        AZ_UNUSED(deltaTime);
        AZ_UNUSED(time);
        if (m_udpSocket && m_udpSocket->IsOpen())
        {
            ProcessIncomingMessages();
        }
    }

    void UdpComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UdpComponent, Component>()
                ->Version(1)
                ->Field("Port", &UdpComponent::m_port);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UdpComponent>("UdpComponent", "UdpComponent Class")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UdpComponent::m_port, "Port", "Port");
            }
        }

        auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->EBus<UdpNotificationBus>("UdpNotification")
                ->Attribute(AZ::Script::Attributes::Category, "UdpNotification")
                ->Handler<UdpNotificationBusHandler>();
            
            behaviorContext->EBus<UdpRequestBus>("UdpRequestBus")
                ->Event("Send", &UdpRequestBus::Events::Send)
                ->Event("SetReceiveCallback", &UdpRequestBus::Events::SetReceiveCallback)
                ->Event("JoinMulticastGroup", &UdpRequestBus::Events::JoinMulticastGroup)
                ->Event("LeaveMulticastGroup", &UdpRequestBus::Events::LeaveMulticastGroup)
                ->Event("SendMulticast", &UdpRequestBus::Events::SendMulticast);
        }
    }

    void UdpComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("UdpComponentService"));
    }

    void UdpComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void UdpComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void UdpComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void UdpComponent::Send(const AZStd::string& ip, AZ::u16 port, const AZStd::string& message)
    {
        if (!m_udpSocket || !m_udpSocket->IsOpen())
        {
            AZ_Warning("UdpComponent", false, "UDP socket is not open");
            return;
        }
        
        AzNetworking::IpAddress targetAddress(ip.c_str(), port, AzNetworking::ProtocolType::Udp);
        if (!targetAddress.IsValid())
        {
            AZ_Error("UdpComponent", false, "Invalid IP address: %s:%d", ip.c_str(), port);
            return;
        }
        
        const uint8_t* data = reinterpret_cast<const uint8_t*>(message.c_str());
        uint32_t size = static_cast<uint32_t>(message.size());
        
        AzNetworking::DtlsEndpoint dtlsEndpoint;
        AzNetworking::ConnectionQuality connectionQuality;
        
        int32_t bytesSent = m_udpSocket->Send(targetAddress, data, size, false, dtlsEndpoint, connectionQuality);
        if (bytesSent <= 0)
        {
            AZ_Warning("UdpComponent", false, "Failed to send UDP message to %s:%d", ip.c_str(), port);
        }
        else
        {
            AZ_TracePrintf("UdpComponent", "Sent %d bytes to %s:%d", bytesSent, ip.c_str(), port);
        }
    }
    
    void UdpComponent::SetReceiveCallback(AZStd::function<void(const AZStd::string&, AZ::u16, const AZStd::string&)> callback)
    {
        m_receiveCallback = AZStd::move(callback);
    }
    
    void UdpComponent::ProcessIncomingMessages()
    {
        if (!m_udpSocket || !m_udpSocket->IsOpen())
        {
            return;
        }
        
        AzNetworking::IpAddress senderAddress;
        int32_t bytesReceived = m_udpSocket->Receive(senderAddress, m_receiveBuffer.data(), static_cast<uint32_t>(m_receiveBuffer.size() - 1));
        
        if (bytesReceived > 0)
        {
            m_receiveBuffer[bytesReceived] = '\0';
            
            AZStd::string message(reinterpret_cast<const char*>(m_receiveBuffer.data()), bytesReceived);
            
            AZStd::string senderIp = senderAddress.GetIpString().c_str();
            AZ::u16 senderPort = senderAddress.GetPort(AzNetworking::ByteOrder::Network);
            
            AZ_TracePrintf("UdpComponent", "Received %d bytes from %s:%d: %s", bytesReceived, senderIp.c_str(), senderPort, message.c_str());
            
            if (m_receiveCallback)
            {
                m_receiveCallback(senderIp, senderPort, message);
            }
            
            UdpNotificationBus::Event(GetEntityId(), &UdpNotificationBus::Events::OnMessageReceived, senderIp, senderPort, message);
        }
    }
    
    void UdpComponent::OnMessageReceived(const AZStd::string& message, const AzNetworking::IpAddress& senderAddress)
    {
        AZStd::string senderIp = senderAddress.GetIpString().c_str();
        AZ::u16 senderPort = senderAddress.GetPort(AzNetworking::ByteOrder::Network);
        
        UdpNotificationBus::Event(GetEntityId(), &UdpNotificationBus::Events::OnMessageReceived, senderIp, senderPort, message);
    }
    
    bool UdpComponent::JoinMulticastGroup(const AZStd::string& multicastAddress)
    {
        if (!m_udpSocket || !m_udpSocket->IsOpen())
        {
            AZ_Warning("UdpComponent", false, "UDP socket is not open");
            return false;
        }
        
        // 检查是否是有效的组播地址
        AzNetworking::IpAddress multicastIp(multicastAddress.c_str(), static_cast<uint16_t>(0), AzNetworking::ProtocolType::Udp);
        if (!multicastIp.IsValid())
        {
            AZ_Error("UdpComponent", false, "Invalid multicast address: %s", multicastAddress.c_str());
            return false;
        }
        
        // 检查是否已经加入了这个组播组
        if (m_joinedMulticastGroups.find(multicastAddress) != m_joinedMulticastGroups.end())
        {
            AZ_Warning("UdpComponent", false, "Already joined multicast group: %s", multicastAddress.c_str());
            return true;
        }
        
        // 获取socket文件描述符
        auto socketFd = m_udpSocket->GetSocketFd();
        
#ifdef AZ_PLATFORM_WINDOWS
        struct ip_mreq mreq;
        inet_pton(AF_INET, multicastAddress.c_str(), &mreq.imr_multiaddr);
        mreq.imr_interface.s_addr = INADDR_ANY;
        
        if (setsockopt(static_cast<SOCKET>(socketFd), IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0)
        {
            AZ_Error("UdpComponent", false, "Failed to join multicast group %s", multicastAddress.c_str());
            return false;
        }
#else
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(multicastAddress.c_str());
        mreq.imr_interface.s_addr = INADDR_ANY;
        
        if (setsockopt(static_cast<int>(socketFd), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        {
            AZ_Error("UdpComponent", false, "Failed to join multicast group %s", multicastAddress.c_str());
            return false;
        }
#endif
        
        m_joinedMulticastGroups.insert(multicastAddress);
        AZ_TracePrintf("UdpComponent", "Successfully joined multicast group: %s", multicastAddress.c_str());
        return true;
    }
    
    bool UdpComponent::LeaveMulticastGroup(const AZStd::string& multicastAddress)
    {
        if (!m_udpSocket || !m_udpSocket->IsOpen())
        {
            AZ_Warning("UdpComponent", false, "UDP socket is not open");
            return false;
        }
        
        // 检查是否已经加入了这个组播组
        auto it = m_joinedMulticastGroups.find(multicastAddress);
        if (it == m_joinedMulticastGroups.end())
        {
            AZ_Warning("UdpComponent", false, "Not a member of multicast group: %s", multicastAddress.c_str());
            return false;
        }
        
        // 获取socket文件描述符
        auto socketFd = m_udpSocket->GetSocketFd();
        
#ifdef AZ_PLATFORM_WINDOWS
        struct ip_mreq mreq;
        inet_pton(AF_INET, multicastAddress.c_str(), &mreq.imr_multiaddr);
        mreq.imr_interface.s_addr = INADDR_ANY;
        
        if (setsockopt(static_cast<SOCKET>(socketFd), IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0)
        {
            AZ_Error("UdpComponent", false, "Failed to leave multicast group %s", multicastAddress.c_str());
            return false;
        }
#else
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(multicastAddress.c_str());
        mreq.imr_interface.s_addr = INADDR_ANY;
        
        if (setsockopt(static_cast<int>(socketFd), IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        {
            AZ_Error("UdpComponent", false, "Failed to leave multicast group %s", multicastAddress.c_str());
            return false;
        }
#endif
        
        m_joinedMulticastGroups.erase(it);
        AZ_TracePrintf("UdpComponent", "Successfully left multicast group: %s", multicastAddress.c_str());
        return true;
    }
    
    void UdpComponent::SendMulticast(const AZStd::string& multicastAddress, AZ::u16 port, const AZStd::string& message)
    {
        if (!m_udpSocket || !m_udpSocket->IsOpen())
        {
            AZ_Warning("UdpComponent", false, "UDP socket is not open");
            return;
        }
        
        // 验证组播地址
        AzNetworking::IpAddress targetAddress(multicastAddress.c_str(), port, AzNetworking::ProtocolType::Udp);
        if (!targetAddress.IsValid())
        {
            AZ_Error("UdpComponent", false, "Invalid multicast address: %s:%d", multicastAddress.c_str(), port);
            return;
        }
        
        const uint8_t* data = reinterpret_cast<const uint8_t*>(message.c_str());
        uint32_t size = static_cast<uint32_t>(message.size());
        
        AzNetworking::DtlsEndpoint dtlsEndpoint;
        AzNetworking::ConnectionQuality connectionQuality;
        
        int32_t bytesSent = m_udpSocket->Send(targetAddress, data, size, false, dtlsEndpoint, connectionQuality);
        if (bytesSent <= 0)
        {
            AZ_Warning("UdpComponent", false, "Failed to send multicast message to %s:%d", multicastAddress.c_str(), port);
        }
        else
        {
            AZ_TracePrintf("UdpComponent", "Sent multicast %d bytes to %s:%d", bytesSent, multicastAddress.c_str(), port);
        }
    }
} // namespace TcpUdpGem
