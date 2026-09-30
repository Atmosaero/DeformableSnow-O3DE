
#pragma once

namespace DeformableSnow
{
    // System Component TypeIds
    inline constexpr const char* DeformableSnowSystemComponentTypeId = "{DB704E0C-3F39-46E8-B3D7-1B5905B819FE}";
    inline constexpr const char* DeformableSnowEditorSystemComponentTypeId = "{358429FE-DC8D-414B-BE10-A54482657E2A}";

    // Module derived classes TypeIds
    inline constexpr const char* DeformableSnowModuleInterfaceTypeId = "{5ABAB473-D5F4-46FA-81C7-C31A407C8598}";
    inline constexpr const char* DeformableSnowModuleTypeId = "{7A29F1C3-95CD-4B99-8140-3A9321A6ED9C}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* DeformableSnowEditorModuleTypeId = DeformableSnowModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* DeformableSnowRequestsTypeId = "{5D78552E-0C22-4014-A40F-A2DAAB79528A}";
} // namespace DeformableSnow
