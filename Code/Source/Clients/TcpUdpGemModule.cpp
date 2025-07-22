
#include <TcpUdpGem/TcpUdpGemTypeIds.h>
#include <TcpUdpGemModuleInterface.h>
#include "TcpUdpGemSystemComponent.h"

namespace TcpUdpGem
{
    class TcpUdpGemModule
        : public TcpUdpGemModuleInterface
    {
    public:
        AZ_RTTI(TcpUdpGemModule, TcpUdpGemModuleTypeId, TcpUdpGemModuleInterface);
        AZ_CLASS_ALLOCATOR(TcpUdpGemModule, AZ::SystemAllocator);
    };
}// namespace TcpUdpGem

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), TcpUdpGem::TcpUdpGemModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_TcpUdpGem, TcpUdpGem::TcpUdpGemModule)
#endif
