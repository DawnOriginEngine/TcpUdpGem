
#include "TcpUdpGemModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <TcpUdpGem/TcpUdpGemTypeIds.h>

#include <Clients/TcpUdpGemSystemComponent.h>
#include <TcpComponent.h>
#include <UdpComponent.h>

namespace TcpUdpGem
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(TcpUdpGemModuleInterface,
        "TcpUdpGemModuleInterface", TcpUdpGemModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(TcpUdpGemModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(TcpUdpGemModuleInterface, AZ::SystemAllocator);

    TcpUdpGemModuleInterface::TcpUdpGemModuleInterface()
    {
        m_descriptors.insert(m_descriptors.end(), {
            TcpUdpGemSystemComponent::CreateDescriptor(),
            TcpComponent::CreateDescriptor(),
            UdpComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList TcpUdpGemModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<TcpUdpGemSystemComponent>(),
        };
    }
} // namespace TcpUdpGem
