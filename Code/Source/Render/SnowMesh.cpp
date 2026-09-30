#include "SnowMesh.h"
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/RPI.Reflect/Buffer/BufferAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelLodAssetCreator.h>

namespace DeformableSnow
{
    namespace
    {
        constexpr uint32_t Sizes[] = {3, 3, 4, 3, 2, 4};
        constexpr const char* Semantics[] = {"POSITION", "NORMAL", "TANGENT", "BITANGENT", "UV", "COLOR"};
        constexpr AZ::RHI::Format Formats[] = {AZ::RHI::Format::R32G32B32_FLOAT, AZ::RHI::Format::R32G32B32_FLOAT,
            AZ::RHI::Format::R32G32B32A32_FLOAT, AZ::RHI::Format::R32G32B32_FLOAT,
            AZ::RHI::Format::R32G32_FLOAT, AZ::RHI::Format::R32G32B32A32_FLOAT};

        AZ::Data::Asset<AZ::RPI::BufferAsset> MakeBuffer(const void* data, size_t bytes, uint32_t count,
            AZ::RHI::Format format)
        {
            AZ::RPI::BufferAssetCreator creator;
            creator.Begin(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
            AZ::RHI::BufferDescriptor desc;
            desc.m_byteCount = bytes;
            // RHI requires an exact match with the selected common pool's bind flags.
            desc.m_bindFlags = AZ::RHI::BufferBindFlags::InputAssembly | AZ::RHI::BufferBindFlags::ShaderRead;
            creator.SetBuffer(data, bytes, desc);
            creator.SetBufferViewDescriptor(AZ::RHI::BufferViewDescriptor::CreateTyped(0, count, format));
            // Device-local buffers use the pool's staging resolver for synchronized updates.
            creator.SetUseCommonPool(AZ::RPI::CommonBufferPoolType::StaticInputAssembly);
            AZ::Data::Asset<AZ::RPI::BufferAsset> asset;
            creator.End(asset);
            return asset;
        }
    }

    void SnowMesh::Extract(const SnowSimulation& simulation)
    {
        const auto& vertices = simulation.Vertices();
        for (int stream = 0; stream < 6; ++stream)
        {
            auto& values = m_streams[stream];
            values.resize(vertices.size() * Sizes[stream]);
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                const auto& v = vertices[i];
                const float* sources[] = {v.position, v.normal, v.tangent, v.bitangent, v.uv, v.color};
                std::copy_n(sources[stream], Sizes[stream], values.data() + i * Sizes[stream]);
            }
        }
    }

    bool SnowMesh::Create(AZ::EntityId entity, SnowSimulation& simulation, const AZ::Data::Asset<AZ::RPI::MaterialAsset>& selectedMaterial)
    {
        Release();
        m_processor = AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(entity);
        if (!m_processor) return false; // dedicated server has no render scene
        if (!selectedMaterial.GetId().IsValid()) return false;
        auto materialAsset = AZ::RPI::AssetUtils::LoadAssetById<AZ::RPI::MaterialAsset>(selectedMaterial.GetId());
        if (!materialAsset.IsReady()) return false;
        auto material = AZ::RPI::Material::FindOrCreate(materialAsset);
        if (!material) return false;
        simulation.Refresh();
        Extract(simulation);
        AZ::RPI::ModelLodAssetCreator lod;
        lod.Begin(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
        const auto& indices = simulation.Indices();
        auto index = MakeBuffer(indices.data(), indices.size() * sizeof(uint32_t), uint32_t(indices.size()), AZ::RHI::Format::R32_UINT);
        if (!index.IsReady()) return false;
        lod.SetLodIndexBuffer(index);
        lod.BeginMesh();
        lod.SetMeshName(AZ::Name("WintercoreSnow"));
        lod.SetMeshMaterialSlot(0);
        lod.SetMeshIndexBuffer({index, index->GetBufferViewDescriptor()});
        const auto& s = simulation.Settings();
        const float hx = (s.columns - 1) * s.cell * .5f, hy = (s.rows - 1) * s.cell * .5f;
        lod.SetMeshAabb(AZ::Aabb::CreateFromMinMax(AZ::Vector3(-hx, -hy, s.level - .20f), AZ::Vector3(hx, hy, s.level + .15f)));
        for (int stream = 0; stream < 6; ++stream)
        {
            const auto& data = m_streams[stream];
            auto buffer = MakeBuffer(data.data(), data.size() * sizeof(float), uint32_t(simulation.Vertices().size()), Formats[stream]);
            if (!buffer.IsReady()) { Release(); return false; }
            lod.AddLodStreamBuffer(buffer);
            lod.AddMeshStreamBuffer(AZ::RHI::ShaderSemantic(AZ::Name(Semantics[stream])),
                AZ::Name(stream == 4 ? "UV0" : ""), {buffer, buffer->GetBufferViewDescriptor()});
            m_buffers[stream] = AZ::RPI::Buffer::FindOrCreate(buffer);
            if (!m_buffers[stream]) { Release(); return false; }
        }
        lod.EndMesh();
        AZ::Data::Asset<AZ::RPI::ModelLodAsset> lodAsset;
        if (!lod.End(lodAsset)) { Release(); return false; }
        AZ::RPI::ModelAssetCreator model;
        model.Begin(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
        model.SetName("WintercoreSnow");
        AZ::RPI::ModelMaterialSlot slot;
        slot.m_stableId = 0;
        slot.m_displayName = AZ::Name("Snow");
        slot.m_defaultMaterialAsset = materialAsset;
        model.AddMaterialSlot(slot);
        model.AddLodAsset(AZStd::move(lodAsset));
        AZ::Data::Asset<AZ::RPI::ModelAsset> modelAsset;
        if (!model.End(modelAsset)) { Release(); return false; }
        AZ::Render::MeshHandleDescriptor desc(modelAsset, material);
        desc.m_entityId = entity;
        desc.m_isAlwaysDynamic = true;
        desc.m_isRayTracingEnabled = false;
        m_mesh = m_processor->AcquireMesh(desc);
        return m_mesh.IsValid();
    }

    bool SnowMesh::Update(const SnowSimulation& simulation)
    {
        if (!m_mesh.IsValid()) return false;
        Extract(simulation);
        bool success = true;
        for (int stream : {0, 1, 2, 3, 5})
            success &= m_buffers[stream]->UpdateData(m_streams[stream].data(), m_streams[stream].size() * sizeof(float));
        return success;
    }

    void SnowMesh::SetTransform(const AZ::Transform& transform)
    {
        if (m_processor && m_mesh.IsValid()) m_processor->SetTransform(m_mesh, transform);
    }

    void SnowMesh::Release()
    {
        if (m_processor && m_mesh.IsValid()) m_processor->ReleaseMesh(m_mesh);
        for (auto& buffer : m_buffers) buffer = nullptr;
        for (auto& stream : m_streams) stream.clear();
        m_processor = nullptr;
    }
}
