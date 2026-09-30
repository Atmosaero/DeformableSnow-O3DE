/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include "DeformableSnowEditorSystemComponent.h"

#include <DeformableSnow/DeformableSnowTypeIds.h>

namespace DeformableSnow
{
    AZ_COMPONENT_IMPL(DeformableSnowEditorSystemComponent, "DeformableSnowEditorSystemComponent",
        DeformableSnowEditorSystemComponentTypeId, BaseSystemComponent);

    void DeformableSnowEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DeformableSnowEditorSystemComponent, DeformableSnowSystemComponent>()
                ->Version(0);
        }
    }

    DeformableSnowEditorSystemComponent::DeformableSnowEditorSystemComponent() = default;

    DeformableSnowEditorSystemComponent::~DeformableSnowEditorSystemComponent() = default;

    void DeformableSnowEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("DeformableSnowSystemEditorService"));
    }

    void DeformableSnowEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("DeformableSnowSystemEditorService"));
    }

    void DeformableSnowEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void DeformableSnowEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void DeformableSnowEditorSystemComponent::Activate()
    {
        DeformableSnowSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void DeformableSnowEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        DeformableSnowSystemComponent::Deactivate();
    }

} // namespace DeformableSnow
