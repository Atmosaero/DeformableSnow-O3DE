/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/base.h>
#include <Atom/RPI.Public/FeatureProcessor.h>

namespace DeformableSnow
{
    class DeformableSnow;

    using DeformableSnowHandle = AZStd::shared_ptr<DeformableSnow>;

    // DeformableSnowFeatureProcessorInterface provides an interface to the feature processor for code outside of Atom
    class DeformableSnowFeatureProcessorInterface
        : public AZ::RPI::FeatureProcessor
    {
    public:
        AZ_RTTI(DeformableSnowFeatureProcessorInterface, "{4B6C4AAD-8C6F-4E84-BAFF-147B02171921}", AZ::RPI::FeatureProcessor);

    };
}
