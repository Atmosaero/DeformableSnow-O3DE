/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/DeformableSnowSystemComponent.h>

namespace DeformableSnow
{
    /// System component for DeformableSnow editor
    class DeformableSnowEditorSystemComponent
        : public DeformableSnowSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = DeformableSnowSystemComponent;
    public:
        AZ_COMPONENT_DECL(DeformableSnowEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        DeformableSnowEditorSystemComponent();
        ~DeformableSnowEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace DeformableSnow
