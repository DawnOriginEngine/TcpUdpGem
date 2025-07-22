
#pragma once

#include <TcpUdpGem/TcpUdpGemTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace TcpUdpGem
{
    class TcpUdpGemRequests
    {
    public:
        AZ_RTTI(TcpUdpGemRequests, TcpUdpGemRequestsTypeId);
        virtual ~TcpUdpGemRequests() = default;
        // Put your public methods here
    };

    class TcpUdpGemBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using TcpUdpGemRequestBus = AZ::EBus<TcpUdpGemRequests, TcpUdpGemBusTraits>;
    using TcpUdpGemInterface = AZ::Interface<TcpUdpGemRequests>;

} // namespace TcpUdpGem
