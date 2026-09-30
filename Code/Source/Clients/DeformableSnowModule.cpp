
#include <DeformableSnow/DeformableSnowTypeIds.h>
#include <DeformableSnowModuleInterface.h>
#include "DeformableSnowSystemComponent.h"

#include <AzCore/RTTI/RTTI.h>

#include <Components/DeformableSnowComponent.h>

namespace DeformableSnow
{
    class DeformableSnowModule
        : public DeformableSnowModuleInterface
    {
    public:
        AZ_RTTI(DeformableSnowModule, DeformableSnowModuleTypeId, DeformableSnowModuleInterface);
        AZ_CLASS_ALLOCATOR(DeformableSnowModule, AZ::SystemAllocator);

        DeformableSnowModule()
        {
            m_descriptors.insert(m_descriptors.end(),
                {
                    DeformableSnowComponent::CreateDescriptor(),
                });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const
        {
            return AZ::ComponentTypeList{ azrtti_typeid<DeformableSnowSystemComponent>() };
        }
    };
}// namespace DeformableSnow

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), DeformableSnow::DeformableSnowModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_DeformableSnow, DeformableSnow::DeformableSnowModule)
#endif
