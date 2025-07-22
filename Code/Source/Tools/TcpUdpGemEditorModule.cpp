
#include <TcpUdpGem/TcpUdpGemTypeIds.h>
#include <TcpUdpGemModuleInterface.h>
#include "TcpUdpGemEditorSystemComponent.h"

namespace TcpUdpGem
{
    class TcpUdpGemEditorModule
        : public TcpUdpGemModuleInterface
    {
    public:
        AZ_RTTI(TcpUdpGemEditorModule, TcpUdpGemEditorModuleTypeId, TcpUdpGemModuleInterface);
        AZ_CLASS_ALLOCATOR(TcpUdpGemEditorModule, AZ::SystemAllocator);

        TcpUdpGemEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                TcpUdpGemEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<TcpUdpGemEditorSystemComponent>(),
            };
        }
    };
}// namespace TcpUdpGem

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), TcpUdpGem::TcpUdpGemEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_TcpUdpGem_Editor, TcpUdpGem::TcpUdpGemEditorModule)
#endif
