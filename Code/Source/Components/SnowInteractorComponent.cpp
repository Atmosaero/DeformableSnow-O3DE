#include "SnowInteractorComponent.h"
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/containers/vector.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <DeformableSnow/SnowSurfaceBus.h>
#include <cmath>

namespace DeformableSnow
{
    // Retain the serialized type name and UUID so existing prefabs survive the move.
    AZ_COMPONENT_IMPL(SnowInteractorComponent, "WCSnowInteractorComponent",
        "{DCE47497-6277-4611-B5C5-5B7D4BE03464}", AZ::Component);

    void SnowInteractorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<SnowInteractorComponent, AZ::Component>()->Version(2)
                ->Field("Surface", &SnowInteractorComponent::m_surface)
                ->Field("Kind", &SnowInteractorComponent::m_kind)
                ->Field("Radius", &SnowInteractorComponent::m_radius)
                ->Field("ContactOffset", &SnowInteractorComponent::m_contactOffset)
                ->Field("SupportDistance", &SnowInteractorComponent::m_supportDistance)
                ->Field("RequireGroundContact", &SnowInteractorComponent::m_requireGroundContact);
            if (auto* ec = sc->GetEditContext())
                ec->Class<SnowInteractorComponent>("Snow Interactor", "Leaves footsteps or continuous tracks on a snow surface")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Snow")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/SnowInteractor.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Viewport/SnowInteractor.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(0, &SnowInteractorComponent::m_surface, "Snow surface", "Entity with Deformable Snow")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &SnowInteractorComponent::m_kind, "Track type", "")
                        ->Attribute(AZ::Edit::Attributes::EnumValues, AZStd::vector<AZ::Edit::EnumConstant<AZ::u32>>{
                            {0, "Footsteps"}, {1, "Rolling object"}, {2, "Ragdoll"}})
                    ->DataElement(0, &SnowInteractorComponent::m_radius, "Radius", "Metres; foot .36, prop .50-.85, ragdoll .66")
                        ->Attribute(AZ::Edit::Attributes::Min, .05f)->Attribute(AZ::Edit::Attributes::Max, 10.f)
                    ->DataElement(0, &SnowInteractorComponent::m_contactOffset, "Contact offset", "Local offset to feet, object bottom, or pelvis")
                    ->DataElement(0, &SnowInteractorComponent::m_requireGroundContact, "Require ground contact",
                        "Raycast against physics ground. Disable for scripted contacts without a physics collider.")
                    ->DataElement(0, &SnowInteractorComponent::m_supportDistance, "Support distance", "Ground detection distance in metres")
                        ->Attribute(AZ::Edit::Attributes::Min, .01f)->Attribute(AZ::Edit::Attributes::Max, 1.f);
        }
    }
    void SnowInteractorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& services)
    {
        services.push_back(AZ_CRC_CE("TransformService"));
    }
    void SnowInteractorComponent::Activate()
    {
        m_clock = 0;
        m_hasPrevious = false;
        AZ::TickBus::Handler::BusConnect();
    }
    void SnowInteractorComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        DeformableSnow::SnowSurfaceRequestBus::Event(m_surface, &DeformableSnow::SnowSurfaceRequests::ForgetContact, GetEntityId());
    }
    void SnowInteractorComponent::OnTick(float dt, AZ::ScriptTimePoint)
    {
        if (!m_surface.IsValid() || !std::isfinite(dt) || dt <= 0) return;
        m_clock += dt;
        if (m_clock < .09f) return;
        AZ::Transform world = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(world, GetEntityId(), &AZ::TransformInterface::GetWorldTM);
        auto contact = world.TransformPoint(m_contactOffset);
        float radius = m_radius;
        if (m_kind == static_cast<AZ::u32>(SnowTrackKind::RollingObject))
        {
            AZ::Aabb bounds = AZ::Aabb::CreateNull();
            AzPhysics::SimulatedBodyComponentRequestsBus::EventResult(bounds, GetEntityId(),
                &AzPhysics::SimulatedBodyComponentRequests::GetAabb);
            if (bounds.IsValid())
            {
                contact = bounds.GetCenter();
                contact.SetZ(bounds.GetMin().GetZ());
                const auto half = bounds.GetExtents() * .5f;
                radius = AZStd::clamp(AZStd::max(half.GetX(), AZStd::max(half.GetY(), half.GetZ())) * 1.2f, .5f, .85f);
            }
        }
        const auto velocity = m_hasPrevious ? (contact - m_previous) / m_clock : AZ::Vector3::CreateZero();
        const float speed = std::sqrt(velocity.GetX()*velocity.GetX() + velocity.GetY()*velocity.GetY());
        const auto direction = speed > .1f ? velocity : world.GetBasisY();
        m_previous = contact;
        m_hasPrevious = true;
        m_clock = 0;
        bool supported = !m_requireGroundContact;
        if (auto* physics = AZ::Interface<AzPhysics::SceneInterface>::Get(); m_requireGroundContact && physics)
        {
            const auto scene = physics->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            if (scene == AzPhysics::InvalidSceneHandle)
            {
                SnowSurfaceRequestBus::Event(m_surface, &SnowSurfaceRequests::ForgetContact, GetEntityId());
                return;
            }
            AzPhysics::RayCastRequest ray;
            ray.m_start = contact + AZ::Vector3(0,0,.1f);
            ray.m_direction = AZ::Vector3(0,0,-1);
            ray.m_distance = std::isfinite(m_supportDistance) ? AZStd::clamp(m_supportDistance, .01f, 1.f) + .1f : .4f;
            ray.m_filterCallback = [id = GetEntityId()](const AzPhysics::SimulatedBody* body, const Physics::Shape*)
            {
                return body->GetEntityId() == id ? AzPhysics::SceneQuery::QueryHitType::None : AzPhysics::SceneQuery::QueryHitType::Block;
            };
            const auto hits = physics->QueryScene(scene, &ray);
            supported = !hits.m_hits.empty() && hits.m_hits.front().m_normal.GetZ() > .2f;
        }
        DeformableSnow::SnowSurfaceRequestBus::Event(m_surface, &DeformableSnow::SnowSurfaceRequests::SubmitContact,
            GetEntityId(), contact, static_cast<AZ::u32>(m_kind), radius, std::atan2(direction.GetY(), direction.GetX()), speed, supported);
    }
}
