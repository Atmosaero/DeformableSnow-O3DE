#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>

namespace DeformableSnow
{
    enum class SnowTrackKind : AZ::u32 { Footsteps, RollingObject, Ragdoll };
    // Attach to a character, a rolling prop, or a pelvis-following ragdoll entity.
    class SnowInteractorComponent final : public AZ::Component, private AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(SnowInteractorComponent);
        static void Reflect(AZ::ReflectContext* context);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        void Activate() override;
        void Deactivate() override;
    private:
        void OnTick(float dt, AZ::ScriptTimePoint time) override;
        AZ::EntityId m_surface;
        // Numeric storage shared by serialization and the editor combo.
        AZ::u32 m_kind = static_cast<AZ::u32>(SnowTrackKind::Footsteps);
        float m_radius = .36f;
        AZ::Vector3 m_contactOffset = AZ::Vector3::CreateZero();
        float m_supportDistance = .3f;
        bool m_requireGroundContact = true;
        float m_clock = 0;
        AZ::Vector3 m_previous = AZ::Vector3::CreateZero();
        bool m_hasPrevious = false;
    };
}
