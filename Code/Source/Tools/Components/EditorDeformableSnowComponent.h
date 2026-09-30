/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>

#include <AzCore/Component/TickBus.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentAdapter.h>
#include <Components/DeformableSnowComponent.h>

#include <DeformableSnow/DeformableSnowTypeIds.h>

namespace DeformableSnow
{
    inline constexpr AZ::TypeId EditorComponentTypeId { "{9C4D57E5-E3C9-43F3-A09C-668B18EBFC7C}" };

    class EditorDeformableSnowComponent final
        : public AZ::Render::EditorRenderComponentAdapter<DeformableSnowComponentController, DeformableSnowComponent, DeformableSnowComponentConfig>
        , private AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , private AzFramework::EntityDebugDisplayEventBus::Handler
        , private AZ::TickBus::Handler
        , private AzToolsFramework::EditorEntityInfoNotificationBus::Handler
    {
    public:
        using BaseClass = AZ::Render::EditorRenderComponentAdapter <DeformableSnowComponentController, DeformableSnowComponent, DeformableSnowComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorDeformableSnowComponent, EditorComponentTypeId, BaseClass);

        static void Reflect(AZ::ReflectContext* context);

        EditorDeformableSnowComponent();
        EditorDeformableSnowComponent(const DeformableSnowComponentConfig& config);

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

    private:

        // AZ::TickBus overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;


    };
}
