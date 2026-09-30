/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <DeformableSnow/DeformableSnowFeatureProcessorInterface.h>

namespace DeformableSnow
{
    class DeformableSnowFeatureProcessor final
        : public DeformableSnowFeatureProcessorInterface
    {
    public:
        AZ_RTTI(DeformableSnowFeatureProcessor, "{295F3EC3-05E4-43B3-B077-64B4A76B11BF}", DeformableSnowFeatureProcessorInterface);
        AZ_CLASS_ALLOCATOR(DeformableSnowFeatureProcessor, AZ::SystemAllocator)

        static void Reflect(AZ::ReflectContext* context);

        DeformableSnowFeatureProcessor() = default;
        virtual ~DeformableSnowFeatureProcessor() = default;

        // FeatureProcessor overrides
        void Activate() override;
        void Deactivate() override;
        void Simulate(const FeatureProcessor::SimulatePacket& packet) override;

    };
}
