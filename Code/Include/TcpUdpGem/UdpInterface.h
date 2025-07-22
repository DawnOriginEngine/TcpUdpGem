
#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace TcpUdpGem
{
    class UdpRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(TcpUdpGem::UdpRequests, "{D222636B-6F60-4D9F-994D-1956B908246B}");

        virtual void Send(const AZStd::string& ip, AZ::u16 port, const AZStd::string& message) = 0;
        virtual void SetReceiveCallback(AZStd::function<void(const AZStd::string&, AZ::u16, const AZStd::string&)> callback) = 0;

    };

    class UdpNotifications
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(TcpUdpGem::UdpNotifications, "{E1F2A3B4-C5D6-7E8F-9A0B-1C2D3E4F5A6B}");

        virtual void OnMessageReceived(const AZStd::string& fromIp, AZ::u16 fromPort, const AZStd::string& message)
        {
            AZ_UNUSED(fromIp);
            AZ_UNUSED(fromPort);
            AZ_UNUSED(message);
        }
    };

    using UdpRequestBus = AZ::EBus<UdpRequests>;
    using UdpNotificationBus = AZ::EBus<UdpNotifications>;

    class UdpNotificationBusHandler :
        public UdpNotificationBus::Handler,
        public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(UdpNotificationBusHandler, "{31234567-7E8F-9A0B-1C2D-3E4F5A6B7C8D}", AZ::SystemAllocator,
            OnMessageReceived);

        void OnMessageReceived(const AZStd::string& fromIp, AZ::u16 fromPort, const AZStd::string& message) override
        {
            Call(FN_OnMessageReceived, fromIp, fromPort, message);
        }
    };
} // namespace TcpUdpGem
