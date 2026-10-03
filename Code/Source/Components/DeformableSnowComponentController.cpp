#include <Components/DeformableSnowComponentController.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContextAttributes.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <limits>

namespace DeformableSnow
{
    namespace
    {
        bool ConvertSnowConfig(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& node)
        {
            // Discard the obsolete role flag from previously saved components.
            node.RemoveElementByName(AZ_CRC_CE("Authority"));
            const int materialIndex = node.FindElement(AZ_CRC_CE("Material"));
            if (materialIndex >= 0)
            {
                AZStd::string path;
                if (!node.GetSubElement(materialIndex).GetData(path)) return false;
                node.RemoveElement(materialIndex);
                auto material = DeformableSnowComponentConfig{}.m_material;
                if (!path.empty() && path != material.GetHint() && path != "materials/snow/wintercoresnow.azmaterial")
                {
                    const auto id = AZ::RPI::AssetUtils::GetAssetIdForProductPath(path.c_str());
                    if (!id.IsValid()) return false;
                    material = {id, azrtti_typeid<AZ::RPI::MaterialAsset>(), path};
                }
                if (node.AddElementWithData(context, "MaterialAsset", material) < 0) return false;
            }
            return true;
        }
    }

    DeformableSnowComponentConfig::DeformableSnowComponentConfig()
        // Stable source UUID of this gem's Assets/Materials/Snow/Snow.material.
        // A typed reference also lets asset bundling discover the material and its textures.
        : m_material(AZ::Data::AssetId(AZ::Uuid("{5794DBF8-CBD4-5F98-BA2F-9C4A609A8D97}"), 0),
            azrtti_typeid<AZ::RPI::MaterialAsset>(), "materials/snow/snow.azmaterial")
    {
    }

    void DeformableSnowComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<DeformableSnowComponentConfig, AZ::ComponentConfig>()->Version(3, &ConvertSnowConfig)
                ->Field("Columns", &DeformableSnowComponentConfig::m_columns)
                ->Field("Rows", &DeformableSnowComponentConfig::m_rows)
                ->Field("CellSize", &DeformableSnowComponentConfig::m_cell)
                ->Field("SnowLevel", &DeformableSnowComponentConfig::m_level)
                ->Field("RecoverySeconds", &DeformableSnowComponentConfig::m_recoverySeconds)
                ->Field("LifetimeSeconds", &DeformableSnowComponentConfig::m_lifetimeSeconds)
                ->Field("UpdateRate", &DeformableSnowComponentConfig::m_updateRate)
                ->Field("MaterialAsset", &DeformableSnowComponentConfig::m_material);
            if (auto* ec = sc->GetEditContext())
            {
                ec->Class<DeformableSnowComponentConfig>("Snow surface", "Distances in metres")
                    ->DataElement(0, &DeformableSnowComponentConfig::m_columns, "Columns", "Grid vertices on X")
                        ->Attribute(AZ::Edit::Attributes::Min, 3)->Attribute(AZ::Edit::Attributes::Max, 1025)
                    ->DataElement(0, &DeformableSnowComponentConfig::m_rows, "Rows", "Grid vertices on Y")
                        ->Attribute(AZ::Edit::Attributes::Min, 3)->Attribute(AZ::Edit::Attributes::Max, 1025)
                    ->DataElement(0, &DeformableSnowComponentConfig::m_cell, "Cell size", "Metres per cell")
                        ->Attribute(AZ::Edit::Attributes::Min, .025f)->Attribute(AZ::Edit::Attributes::Max, 10.f)
                    ->DataElement(0, &DeformableSnowComponentConfig::m_level, "Snow level", "Local height in metres")
                    ->DataElement(0, &DeformableSnowComponentConfig::m_lifetimeSeconds, "Track lifetime", "Seconds until a track disappears completely, including recovery")
                        ->Attribute(AZ::Edit::Attributes::Min, .01f)
                    ->DataElement(0, &DeformableSnowComponentConfig::m_recoverySeconds, "Recovery seconds", "Smooth recovery during the final seconds of a track's lifetime; zero keeps tracks indefinitely")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->DataElement(0, &DeformableSnowComponentConfig::m_updateRate, "Update rate", "Updates per second")
                        ->Attribute(AZ::Edit::Attributes::Min, 1.f)->Attribute(AZ::Edit::Attributes::Max, 60.f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &DeformableSnowComponentConfig::m_material,
                        "Material", "Material for the snow surface. The gem's powder snow material is selected by default.")
                        ->Attribute(AZ::Edit::Attributes::ShowProductAssetFileName, true);
            }
        }
    }
    void DeformableSnowComponentController::Reflect(AZ::ReflectContext* context)
    {
        DeformableSnowComponentConfig::Reflect(context);
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<DeformableSnowComponentController>()->Version(1)
                ->Field("Configuration", &DeformableSnowComponentController::m_configuration);
            if (auto* ec = sc->GetEditContext())
                ec->Class<DeformableSnowComponentController>("Snow", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(0, &DeformableSnowComponentController::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly);
        }
        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
            bc->EBus<SnowSurfaceRequestBus>("SnowSurfaceRequestBus")
                ->Attribute(AZ::Script::Attributes::Module, "snow")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Event("Stamp", &SnowSurfaceRequests::Stamp)
                ->Event("SubmitContact", &SnowSurfaceRequests::SubmitContact)
                ->Event("ForgetContact", &SnowSurfaceRequests::ForgetContact)
                ->Event("Clear", &SnowSurfaceRequests::Clear)
                ->Event("GetHeight", &SnowSurfaceRequests::GetHeight)
                ->Event("GetVertexCount", &SnowSurfaceRequests::GetVertexCount);
    }
    void DeformableSnowComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType&) {}
    void DeformableSnowComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& s) { s.push_back(AZ_CRC_CE("DeformableSnowService")); }
    void DeformableSnowComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& s) { s.push_back(AZ_CRC_CE("DeformableSnowService")); }
    void DeformableSnowComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& s) { s.push_back(AZ_CRC_CE("TransformService")); }
    void DeformableSnowComponentController::Activate(AZ::EntityId entityId)
    {
        m_entityId = entityId;
        AZ::TransformBus::EventResult(m_world, entityId, &AZ::TransformInterface::GetWorldTM);
        Rebuild();
        AZ::TransformNotificationBus::Handler::BusConnect(entityId);
        SnowSurfaceRequestBus::Handler::BusConnect(entityId);
        AZ::TickBus::Handler::BusConnect();
    }
    void DeformableSnowComponentController::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        SnowSurfaceRequestBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        m_mesh.Release();
        m_contacts.clear();
        m_renderReady = false;
        m_entityId.SetInvalid();
    }
    void DeformableSnowComponentController::SetConfiguration(const DeformableSnowComponentConfig& config)
    {
        m_configuration = config;
        if (m_entityId.IsValid()) Rebuild();
    }
    void DeformableSnowComponentController::Rebuild()
    {
        SnowSettings settings{m_configuration.m_columns, m_configuration.m_rows, m_configuration.m_cell,
            m_configuration.m_level, m_configuration.m_recoverySeconds, m_configuration.m_lifetimeSeconds};
        if (!settings.IsValid())
        {
            AZ_Warning("DeformableSnow", false, "Invalid snow configuration; using safe defaults");
            settings = SnowSettings{};
        }
        m_simulation.Reset(settings);
        m_contacts.clear();
        m_renderReady = m_mesh.Create(m_entityId, m_simulation, m_configuration.m_material);
        m_mesh.SetTransform(m_world);
        m_clock = m_retryClock = 0;
        m_uploadPending = false;
    }
    void DeformableSnowComponentController::OnTransformChanged(const AZ::Transform&, const AZ::Transform& world)
    {
        m_world = world;
        m_mesh.SetTransform(world);
    }
    void DeformableSnowComponentController::OnTick(float dt, AZ::ScriptTimePoint)
    {
        if (!std::isfinite(dt) || dt <= 0) return;
        m_clock += dt;
        m_retryClock += dt;
        const float rate = std::isfinite(m_configuration.m_updateRate) ? std::clamp(m_configuration.m_updateRate, 1.f, 60.f) : 20.f;
        if (m_clock < 1.f / rate) return;
        m_simulation.Advance(m_clock);
        m_clock = 0;
        if (!m_renderReady && m_retryClock >= 1.f)
        {
            m_retryClock = 0;
            m_renderReady = m_mesh.Create(m_entityId, m_simulation, m_configuration.m_material);
            m_mesh.SetTransform(m_world);
        }
        m_uploadPending |= m_simulation.Dirty();
        m_simulation.Refresh();
        if (m_renderReady && m_uploadPending) m_uploadPending = !m_mesh.Update(m_simulation);
    }
    bool DeformableSnowComponentController::Stamp(const AZ::Vector3& position, AZ::u32 kind, float radius, float yaw)
    {
        if (kind > 2 || !position.IsFinite() || !std::isfinite(yaw)) return false;
        const float scale = m_world.GetUniformScale();
        if (scale < .0001f) return false;
        const auto inverse = m_world.GetInverse();
        const auto local = inverse.TransformPoint(position);
        const auto direction = inverse.TransformVector(AZ::Vector3(std::cos(yaw), std::sin(yaw), 0));
        return m_simulation.Stamp({local.GetX(), local.GetY(), local.GetZ(), radius / scale,
            std::atan2(direction.GetY(), direction.GetX()), uint8_t(kind)});
    }
    void DeformableSnowComponentController::SubmitContact(AZ::EntityId source, const AZ::Vector3& position,
        AZ::u32 kind, float radius, float yaw, float speed, bool supported)
    {
        if (!source.IsValid() || kind > 2 || !position.IsFinite()) return;
        if (m_contacts.size() >= 1024 && m_contacts.find(source) == m_contacts.end()) return;
        const SnowStamp contact{position.GetX(), position.GetY(), position.GetZ(), radius, yaw, uint8_t(kind)};
        if (!contact.IsValid()) return;
        for (const auto& s : m_contacts[source].Update(contact, speed, supported))
            Stamp(AZ::Vector3(s.x, s.y, s.z), s.kind, s.radius, s.yaw);
    }
    void DeformableSnowComponentController::Clear() { Rebuild(); }
    float DeformableSnowComponentController::GetHeight(const AZ::Vector3& position)
    {
        if (!position.IsFinite() || m_simulation.Vertices().empty() || m_world.GetUniformScale() < .0001f)
            return std::numeric_limits<float>::quiet_NaN();
        const auto local = m_world.GetInverse().TransformPoint(position);
        const auto& s = m_simulation.Settings();
        const float gx = local.GetX() / s.cell + (s.columns - 1) * .5f;
        const float gy = local.GetY() / s.cell + (s.rows - 1) * .5f;
        if (gx < 0 || gy < 0 || gx > s.columns - 1 || gy > s.rows - 1) return std::numeric_limits<float>::quiet_NaN();
        const auto x = uint32_t(gx), y = uint32_t(gy), xr = std::min(x + 1, s.columns - 1), yr = std::min(y + 1, s.rows - 1);
        const float a = gx - x, b = gy - y;
        const float height = (1-b) * ((1-a)*m_simulation.Height(x,y)+a*m_simulation.Height(xr,y))
            + b*((1-a)*m_simulation.Height(x,yr)+a*m_simulation.Height(xr,yr));
        return m_world.TransformPoint(AZ::Vector3(local.GetX(), local.GetY(), height)).GetZ();
    }
    bool DeformableSnowComponentController::RestoreSnapshot(const SnowSnapshot& snapshot)
    {
        if (!m_simulation.Restore(snapshot)) return false;
        m_contacts.clear();
        m_clock = m_retryClock = 0;
        m_uploadPending = false;
        m_renderReady = m_mesh.Create(m_entityId, m_simulation, m_configuration.m_material);
        m_mesh.SetTransform(m_world);
        return true;
    }
}
