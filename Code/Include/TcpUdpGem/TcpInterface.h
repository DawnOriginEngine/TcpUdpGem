
#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace TcpUdpGem
{
    class TcpRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(TcpUdpGem::TcpRequests, "{FA5DA2B7-6E2D-46A2-BF28-F0FF6D8C585A}");

        virtual bool Listen(AZ::u16 port) = 0;
        virtual bool Connect(const AZStd::string& ip, AZ::u16 port) = 0;
        virtual void Disconnect() = 0;
        virtual bool IsConnected() const = 0;
        
        virtual bool Send(const AZStd::string& message) = 0;
        
        virtual void SetReceiveCallback(AZStd::function<void(const AZStd::string&)> callback) = 0;
    };
    
    class TcpNotifications
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(TcpUdpGem::TcpNotifications, "{8B5E4C92-7A3D-4F1E-9B6C-2D8F5A1E3C7B}");
        
        virtual void OnConnected() = 0;
        virtual void OnDisconnected() = 0;
        virtual void OnConnectionFailed() = 0;        
        virtual void OnMessageReceived(const AZStd::string& message) = 0;
    };

    using TcpRequestBus = AZ::EBus<TcpRequests>;
    using TcpNotificationBus = AZ::EBus<TcpNotifications>;

    class TcpNotificationBusHandler
        : public TcpNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(TcpNotificationBusHandler, "{29847488-9808-4845-9554-71391598852A}", AZ::SystemAllocator,
            OnConnected, OnDisconnected, OnConnectionFailed, OnMessageReceived);

        void OnConnected() override
        {
            Call(FN_OnConnected);
        }

        void OnDisconnected() override
        {
            Call(FN_OnDisconnected);
        }

        void OnConnectionFailed() override
        {
            Call(FN_OnConnectionFailed);
        }

        void OnMessageReceived(const AZStd::string& message) override
        {
            Call(FN_OnMessageReceived, message);
        }
    };
} // namespace TcpUdpGem
