#pragma once
#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/Math/Vector3.h>
#include <DeformableSnow/SnowSimulation.h>

namespace DeformableSnow
{
    class SnowSurfaceRequests : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        using BusIdType = AZ::EntityId;
        // Call from gameplay on the main thread. Yaw is world-space radians.
        virtual bool Stamp(const AZ::Vector3& worldPosition, AZ::u32 kind, float radius, float yaw) = 0;
        virtual void SubmitContact(AZ::EntityId source, const AZ::Vector3& worldPosition, AZ::u32 kind,
            float radius, float yaw, float speed, bool supported) = 0;
        virtual void ForgetContact(AZ::EntityId source) = 0;
        virtual void Clear() = 0;
        virtual float GetHeight(const AZ::Vector3& worldPosition) = 0;
        virtual AZ::u32 GetVertexCount() const = 0;
        virtual SnowSnapshot GetSnapshot() const = 0;
        virtual bool RestoreSnapshot(const SnowSnapshot& snapshot) = 0;
        virtual bool ReplayEvent(const SnowEvent& event) = 0;
        virtual std::vector<SnowEvent> GetEvents() const = 0;
    };
    using SnowSurfaceRequestBus = AZ::EBus<SnowSurfaceRequests>;
}
