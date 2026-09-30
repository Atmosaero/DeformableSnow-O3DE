#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/TransformBus.h>
#include <DeformableSnow/SnowSurfaceBus.h>
#include <DeformableSnow/SnowContactTracker.h>
#include <AzCore/std/containers/unordered_map.h>
#include <Render/SnowMesh.h>

namespace DeformableSnow
{
    class DeformableSnowComponentConfig final : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(DeformableSnowComponentConfig, "{B35D151C-FAD3-43A2-B64D-7979647DD93D}", ComponentConfig);
        AZ_CLASS_ALLOCATOR(DeformableSnowComponentConfig, AZ::SystemAllocator);
        DeformableSnowComponentConfig();
        static void Reflect(AZ::ReflectContext* context);
        AZ::u32 m_columns = 337, m_rows = 273;
        float m_cell = .125f, m_level = .18f, m_recoverySeconds = 45.f, m_updateRate = 20.f;
        AZ::Data::Asset<AZ::RPI::MaterialAsset> m_material;
    };
    class DeformableSnowComponentController final
        : private AZ::TransformNotificationBus::Handler
        , private AZ::TickBus::Handler
        , public SnowSurfaceRequestBus::Handler
    {
    public:
        AZ_RTTI(DeformableSnowComponentController, "{3ADCEE63-CB7C-45D3-8D00-EF50FB115F42}");
        AZ_CLASS_ALLOCATOR(DeformableSnowComponentController, AZ::SystemAllocator);
        static void Reflect(AZ::ReflectContext* context);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        DeformableSnowComponentController() = default;
        explicit DeformableSnowComponentController(const DeformableSnowComponentConfig& config) : m_configuration(config) {}
        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const DeformableSnowComponentConfig& config);
        const DeformableSnowComponentConfig& GetConfiguration() const { return m_configuration; }
        bool Stamp(const AZ::Vector3& worldPosition, AZ::u32 kind, float radius, float yaw) override;
        void SubmitContact(AZ::EntityId source, const AZ::Vector3& position, AZ::u32 kind,
            float radius, float yaw, float speed, bool supported) override;
        void ForgetContact(AZ::EntityId source) override { m_contacts.erase(source); }
        void Clear() override;
        float GetHeight(const AZ::Vector3& worldPosition) override;
        AZ::u32 GetVertexCount() const override { return aznumeric_cast<AZ::u32>(m_simulation.Vertices().size()); }
        SnowSnapshot GetSnapshot() const override { return m_simulation.Snapshot(); }
        bool RestoreSnapshot(const SnowSnapshot& snapshot) override;
        bool ReplayEvent(const SnowEvent& event) override { return m_simulation.Replay(event); }
        std::vector<SnowEvent> GetEvents() const override { return m_simulation.Journal(); }
    private:
        AZ_DISABLE_COPY(DeformableSnowComponentController);
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;
        void OnTick(float dt, AZ::ScriptTimePoint time) override;
        void Rebuild();
        DeformableSnowComponentConfig m_configuration;
        SnowSimulation m_simulation;
        // Per-source spacing state is owned by this surface, not by the renderer.
        AZStd::unordered_map<AZ::EntityId, SnowContactTracker> m_contacts;
        SnowMesh m_mesh;
        AZ::EntityId m_entityId;
        AZ::Transform m_world = AZ::Transform::CreateIdentity();
        float m_clock = 0, m_retryClock = 0;
        bool m_renderReady = false, m_uploadPending = false;
    };
}
