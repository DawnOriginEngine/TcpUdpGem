
#include <UdpComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzNetworking/Utilities/IpAddress.h>
#include <AzNetworking/UdpTransport/DtlsEndpoint.h>
#include <AzNetworking/ConnectionLayer/IConnection.h>

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
        if (m_udpSocket->IsOpen())
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
                ->Event("Send", &UdpRequestBus::Events::Send);
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
} // namespace TcpUdpGem
