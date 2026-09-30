
#include <DeformableSnow/DeformableSnowTypeIds.h>
#include <DeformableSnowModuleInterface.h>
#include "DeformableSnowEditorSystemComponent.h"
#include "Components/EditorDeformableSnowComponent.h"

namespace DeformableSnow
{
    class DeformableSnowEditorModule
        : public DeformableSnowModuleInterface
    {
    public:
        AZ_RTTI(DeformableSnowEditorModule, DeformableSnowEditorModuleTypeId, DeformableSnowModuleInterface);
        AZ_CLASS_ALLOCATOR(DeformableSnowEditorModule, AZ::SystemAllocator);

        DeformableSnowEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                DeformableSnowEditorSystemComponent::CreateDescriptor(),
                DeformableSnowComponent::CreateDescriptor(),
                EditorDeformableSnowComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<DeformableSnowEditorSystemComponent>(),
            };
        }
    };
}// namespace DeformableSnow

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), DeformableSnow::DeformableSnowEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_DeformableSnow_Editor, DeformableSnow::DeformableSnowEditorModule)
#endif
