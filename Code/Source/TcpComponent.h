
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/functional.h>
#include <AzCore/std/containers/array.h>
#include <AzNetworking/TcpTransport/TcpSocket.h>
#include <TcpUdpGem/TcpInterface.h>

namespace TcpUdpGem
{
    class TcpComponent
        : public AZ::Component
        , public TcpRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(TcpComponent);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        bool Listen(AZ::u16 port) override;
        bool Connect(const AZStd::string& ip, AZ::u16 port) override;
        void Disconnect() override;
        bool IsConnected() const override;
        bool Send(const AZStd::string& message) override;
        void SetReceiveCallback(AZStd::function<void(const AZStd::string&)> callback) override;
        
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    protected:
        void Activate() override;
        void Deactivate() override;
        
        void ProcessIncomingMessages();
        void OnMessageReceived(const AZStd::string& message);
        
    private:
        static constexpr uint32_t MaxReceiveBufferSize = 4096;
        
        AZStd::unique_ptr<AzNetworking::TcpSocket> m_tcpSocket;
        AZStd::function<void(const AZStd::string&)> m_receiveCallback;
        AZStd::array<uint8_t, MaxReceiveBufferSize> m_receiveBuffer;
        bool m_isServer = false;
    };
} // namespace TcpUdpGem
