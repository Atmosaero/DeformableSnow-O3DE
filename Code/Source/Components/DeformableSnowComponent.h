/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Components/DeformableSnowComponentController.h>
#include <AzFramework/Components/ComponentAdapter.h>

namespace DeformableSnow
{
    inline constexpr AZ::TypeId DeformableSnowComponentTypeId { "{5B3B780B-0D74-4A3F-A7D8-070F84637060}" };

    class DeformableSnowComponent final
        : public AzFramework::Components::ComponentAdapter<DeformableSnowComponentController, DeformableSnowComponentConfig>
    {
    public:
        using BaseClass = AzFramework::Components::ComponentAdapter<DeformableSnowComponentController, DeformableSnowComponentConfig>;
        AZ_COMPONENT(DeformableSnowComponent, DeformableSnowComponentTypeId, BaseClass);

        DeformableSnowComponent() = default;
        DeformableSnowComponent(const DeformableSnowComponentConfig& config);

        static void Reflect(AZ::ReflectContext* context);
    };
}
