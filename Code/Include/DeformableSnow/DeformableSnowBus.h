
#pragma once

#include <DeformableSnow/DeformableSnowTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace DeformableSnow
{
    class DeformableSnowRequests
    {
    public:
        AZ_RTTI(DeformableSnowRequests, DeformableSnowRequestsTypeId);
        virtual ~DeformableSnowRequests() = default;
        // Put your public methods here
    };

    class DeformableSnowBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using DeformableSnowRequestBus = AZ::EBus<DeformableSnowRequests, DeformableSnowBusTraits>;
    using DeformableSnowInterface = AZ::Interface<DeformableSnowRequests>;

} // namespace DeformableSnow
