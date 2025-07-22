
#include <TcpComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzNetworking/Utilities/IpAddress.h>
#include <AzCore/Console/ILogger.h>

namespace TcpUdpGem
{
    AZ_COMPONENT_IMPL(TcpComponent, "TcpComponent", "{36DDF312-EB3C-4801-AB95-C34C8E48F750}");

    void TcpComponent::Activate()
    {
        TcpRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
        
        m_tcpSocket = AZStd::make_unique<AzNetworking::TcpSocket>();
    }

    void TcpComponent::Deactivate()
    {
        Disconnect();
        m_tcpSocket.reset();
        
        AZ::TickBus::Handler::BusDisconnect();
        TcpRequestBus::Handler::BusDisconnect(GetEntityId());
    }

    void TcpComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TcpComponent, AZ::Component>()
                ->Version(1)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<TcpComponent>("TcpComponent", "")
                           ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                           ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer")
                           ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                           ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                           ->ClassElement(AZ::Edit::ClassElements::Group, "Server")
                           ->Attribute(AZ::Edit::Attributes::AutoExpand, false);
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TcpComponent>("TcpComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Multiplayer")
                ;
                
            behaviorContext->EBus<TcpRequestBus>("TcpRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Multiplayer")
                ->Event("Listen", &TcpRequestBus::Events::Listen)
                ->Event("Connect", &TcpRequestBus::Events::Connect)
                ->Event("Disconnect", &TcpRequestBus::Events::Disconnect)
                ->Event("IsConnected", &TcpRequestBus::Events::IsConnected)
                ->Event("Send", &TcpRequestBus::Events::Send)
                ->Event("SetReceiveCallback", &TcpRequestBus::Events::SetReceiveCallback)
                ;
                
            behaviorContext->EBus<TcpNotificationBus>("TcpNotification")
                ->Attribute(AZ::Script::Attributes::Category, "TcpNotification")
                ->Handler<TcpNotificationBusHandler>();
        }
    }

    void TcpComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TcpComponentService"));
    }

    void TcpComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void TcpComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void TcpComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
    
    void TcpComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
        AZ_UNUSED(deltaTime);
        AZ_UNUSED(time);
        
        if (m_tcpSocket && m_tcpSocket->IsOpen())
        {
            ProcessIncomingMessages();
        }
    }
    
    bool TcpComponent::Listen(AZ::u16 port)
    {
        if (!m_tcpSocket)
        {
            AZ_Error("TcpComponent", false, "TCP socket is not initialized");
            return false;
        }
        
        if (m_tcpSocket->IsOpen())
        {
            AZ_Warning("TcpComponent", false, "TCP socket is already open");
            return false;
        }
        
        bool success = m_tcpSocket->Listen(port);
        if (success)
        {
            m_isServer = true;
            AZ_TracePrintf("TcpComponent", "TCP server listening on port %d", port);
            TcpNotificationBus::Event(GetEntityId(), &TcpNotificationBus::Events::OnConnected);
        }
        else
        {
            AZ_Error("TcpComponent", false, "Failed to start TCP server on port %d", port);
            TcpNotificationBus::Event(GetEntityId(), &TcpNotificationBus::Events::OnConnectionFailed);
        }
        
        return success;
    }
    
    bool TcpComponent::Connect(const AZStd::string& ip, AZ::u16 port)
    {
        if (!m_tcpSocket)
        {
            AZ_Error("TcpComponent", false, "TCP socket is not initialized");
            return false;
        }
        
        if (m_tcpSocket->IsOpen())
        {
            AZ_Warning("TcpComponent", false, "TCP socket is already open");
            return false;
        }
        
        AzNetworking::IpAddress targetAddress(ip.c_str(), port, AzNetworking::ProtocolType::Tcp);
        if (!targetAddress.IsValid())
        {
            AZ_Error("TcpComponent", false, "Invalid IP address: %s:%d", ip.c_str(), port);
            TcpNotificationBus::Event(GetEntityId(), &TcpNotificationBus::Events::OnConnectionFailed);
            return false;
        }
        
        bool success = m_tcpSocket->Connect(targetAddress, 0);
        if (success)
        {
            m_isServer = false;
            AZ_TracePrintf("TcpComponent", "Connected to TCP server %s:%d", ip.c_str(), port);
            TcpNotificationBus::Event(GetEntityId(), &TcpNotificationBus::Events::OnConnected);
        }
        else
        {
            AZ_Error("TcpComponent", false, "Failed to connect to TCP server %s:%d", ip.c_str(), port);
            TcpNotificationBus::Event(GetEntityId(), &TcpNotificationBus::Events::OnConnectionFailed);
        }
        
        return success;
    }
    
    void TcpComponent::Disconnect()
    {
        if (m_tcpSocket && m_tcpSocket->IsOpen())
        {
            m_tcpSocket->Close();
            AZ_TracePrintf("TcpComponent", "TCP connection closed");
            TcpNotificationBus::Event(GetEntityId(), &TcpNotificationBus::Events::OnDisconnected);
        }
        m_isServer = false;
    }
    
    bool TcpComponent::IsConnected() const
    {
        return m_tcpSocket && m_tcpSocket->IsOpen();
    }
    
    bool TcpComponent::Send(const AZStd::string& message)
    {
        if (!m_tcpSocket || !m_tcpSocket->IsOpen())
        {
            AZ_Warning("TcpComponent", false, "TCP socket is not connected");
            return false;
        }
        
        const uint8_t* data = reinterpret_cast<const uint8_t*>(message.c_str());
        uint32_t size = static_cast<uint32_t>(message.size());
        
        int32_t bytesSent = m_tcpSocket->Send(data, size);
        if (bytesSent <= 0)
        {
            AZ_Warning("TcpComponent", false, "Failed to send TCP message");
            return false;
        }
        
        AZ_TracePrintf("TcpComponent", "Sent %d bytes via TCP", bytesSent);
        return true;
    }
    
    void TcpComponent::SetReceiveCallback(AZStd::function<void(const AZStd::string&)> callback)
    {
        m_receiveCallback = callback;
    }
    
    void TcpComponent::ProcessIncomingMessages()
    {
        if (!m_tcpSocket || !m_tcpSocket->IsOpen())
        {
            return;
        }
        
        int32_t bytesReceived = m_tcpSocket->Receive(m_receiveBuffer.data(), static_cast<uint32_t>(m_receiveBuffer.size()));
        if (bytesReceived > 0)
        {
            AZStd::string message(reinterpret_cast<const char*>(m_receiveBuffer.data()), bytesReceived);
            
            AZ_TracePrintf("TcpComponent", "Received %d bytes via TCP: %s", bytesReceived, message.c_str());
            
            if (m_receiveCallback)
            {
                m_receiveCallback(message);
            }
            
            OnMessageReceived(message);
        }
    }
    
    void TcpComponent::OnMessageReceived(const AZStd::string& message)
    {
        TcpNotificationBus::Event(GetEntityId(), &TcpNotificationBus::Events::OnMessageReceived, message);
    }
} // namespace TcpUdpGem
