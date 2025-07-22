
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/TcpUdpGemSystemComponent.h>

namespace TcpUdpGem
{
    /// System component for TcpUdpGem editor
    class TcpUdpGemEditorSystemComponent
        : public TcpUdpGemSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = TcpUdpGemSystemComponent;
    public:
        AZ_COMPONENT_DECL(TcpUdpGemEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        TcpUdpGemEditorSystemComponent();
        ~TcpUdpGemEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace TcpUdpGem
