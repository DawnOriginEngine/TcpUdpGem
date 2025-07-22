
#include "TcpUdpGemSystemComponent.h"

#include <TcpUdpGem/TcpUdpGemTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace TcpUdpGem
{
    AZ_COMPONENT_IMPL(TcpUdpGemSystemComponent, "TcpUdpGemSystemComponent",
        TcpUdpGemSystemComponentTypeId);

    void TcpUdpGemSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TcpUdpGemSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void TcpUdpGemSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TcpUdpGemService"));
    }

    void TcpUdpGemSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("TcpUdpGemService"));
    }

    void TcpUdpGemSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void TcpUdpGemSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    TcpUdpGemSystemComponent::TcpUdpGemSystemComponent()
    {
        if (TcpUdpGemInterface::Get() == nullptr)
        {
            TcpUdpGemInterface::Register(this);
        }
    }

    TcpUdpGemSystemComponent::~TcpUdpGemSystemComponent()
    {
        if (TcpUdpGemInterface::Get() == this)
        {
            TcpUdpGemInterface::Unregister(this);
        }
    }

    void TcpUdpGemSystemComponent::Init()
    {
    }

    void TcpUdpGemSystemComponent::Activate()
    {
        TcpUdpGemRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void TcpUdpGemSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        TcpUdpGemRequestBus::Handler::BusDisconnect();
    }

    void TcpUdpGemSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace TcpUdpGem
