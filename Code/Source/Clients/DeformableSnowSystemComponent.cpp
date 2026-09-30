/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "DeformableSnowSystemComponent.h"

#include <DeformableSnow/DeformableSnowTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>

#include <Render/DeformableSnowFeatureProcessor.h>

namespace DeformableSnow
{
    AZ_COMPONENT_IMPL(DeformableSnowSystemComponent, "DeformableSnowSystemComponent",
        DeformableSnowSystemComponentTypeId);

    void DeformableSnowSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DeformableSnowSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }

        DeformableSnowFeatureProcessor::Reflect(context);
    }

    void DeformableSnowSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("DeformableSnowSystemService"));
    }

    void DeformableSnowSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("DeformableSnowSystemService"));
    }

    void DeformableSnowSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("RPISystem"));
    }

    void DeformableSnowSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    DeformableSnowSystemComponent::DeformableSnowSystemComponent()
    {
        if (DeformableSnowInterface::Get() == nullptr)
        {
            DeformableSnowInterface::Register(this);
        }
    }

    DeformableSnowSystemComponent::~DeformableSnowSystemComponent()
    {
        if (DeformableSnowInterface::Get() == this)
        {
            DeformableSnowInterface::Unregister(this);
        }
    }

    void DeformableSnowSystemComponent::Init()
    {
    }

    void DeformableSnowSystemComponent::Activate()
    {
        DeformableSnowRequestBus::Handler::BusConnect();

        AZ::RPI::FeatureProcessorFactory::Get()->RegisterFeatureProcessor<DeformableSnowFeatureProcessor>();
    }

    void DeformableSnowSystemComponent::Deactivate()
    {
        AZ::RPI::FeatureProcessorFactory::Get()->UnregisterFeatureProcessor<DeformableSnowFeatureProcessor>();

        DeformableSnowRequestBus::Handler::BusDisconnect();
    }

} // namespace DeformableSnow
