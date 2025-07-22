
#pragma once

#include <AzCore/Component/Component.h>
#include <TcpUdpGem/UdpInterface.h>

#include <AzNetworking/UdpTransport/UdpNetworkInterface.h>
#include <AzNetworking/UdpTransport/UdpSocket.h>
#include <AzNetworking/Utilities/IpAddress.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/functional.h>
#include <AzCore/std/containers/array.h>

namespace TcpUdpGem
{

    class UdpComponent
        : public AZ::Component
        , public UdpRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(UdpComponent);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Send(const AZStd::string& ip, AZ::u16 port, const AZStd::string& message) override;
        void SetReceiveCallback(AZStd::function<void(const AZStd::string&, AZ::u16, const AZStd::string&)> callback) override;
    
    protected:
        void Activate() override;
        void Deactivate() override;
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void ProcessIncomingMessages();
        void OnMessageReceived(const AZStd::string& message, const AzNetworking::IpAddress& senderAddress);

        AZStd::unique_ptr<AzNetworking::UdpSocket> m_udpSocket;
        
         static constexpr size_t MaxReceiveBufferSize = 1024;
         AZStd::array<uint8_t, MaxReceiveBufferSize> m_receiveBuffer;

         AZStd::function<void(const AZStd::string&, AZ::u16, const AZStd::string&)> m_receiveCallback;
 
         AZ::u16 m_port = 9000;
    };
} // namespace TcpUdpGem
