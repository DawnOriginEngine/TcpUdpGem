
#include <AzCore/Serialization/SerializeContext.h>
#include "TcpUdpGemEditorSystemComponent.h"

#include <TcpUdpGem/TcpUdpGemTypeIds.h>

namespace TcpUdpGem
{
    AZ_COMPONENT_IMPL(TcpUdpGemEditorSystemComponent, "TcpUdpGemEditorSystemComponent",
        TcpUdpGemEditorSystemComponentTypeId, BaseSystemComponent);

    void TcpUdpGemEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TcpUdpGemEditorSystemComponent, TcpUdpGemSystemComponent>()
                ->Version(0);
        }
    }

    TcpUdpGemEditorSystemComponent::TcpUdpGemEditorSystemComponent() = default;

    TcpUdpGemEditorSystemComponent::~TcpUdpGemEditorSystemComponent() = default;

    void TcpUdpGemEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("TcpUdpGemEditorService"));
    }

    void TcpUdpGemEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("TcpUdpGemEditorService"));
    }

    void TcpUdpGemEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void TcpUdpGemEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void TcpUdpGemEditorSystemComponent::Activate()
    {
        TcpUdpGemSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void TcpUdpGemEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        TcpUdpGemSystemComponent::Deactivate();
    }

} // namespace TcpUdpGem
