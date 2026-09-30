#pragma once
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Buffer/Buffer.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <DeformableSnow/SnowSimulation.h>

namespace DeformableSnow
{
    class SnowMesh
    {
    public:
        bool Create(AZ::EntityId entity, SnowSimulation& simulation, const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset);
        bool Update(const SnowSimulation& simulation);
        void SetTransform(const AZ::Transform& transform);
        void Release();
    private:
        AZ::Render::MeshFeatureProcessorInterface* m_processor = nullptr;
        AZ::Render::MeshFeatureProcessorInterface::MeshHandle m_mesh;
        AZ::Data::Instance<AZ::RPI::Buffer> m_buffers[6];
        std::vector<float> m_streams[6];
        void Extract(const SnowSimulation& simulation);
    };
}
