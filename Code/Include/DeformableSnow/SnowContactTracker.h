#pragma once
#include <DeformableSnow/SnowSimulation.h>

namespace DeformableSnow
{
    // Feed actual support/contact positions from character movement or physics.
    // No actor-specific offsets: the caller supplies the foot / prop bottom / pelvis.
    class SnowContactTracker
    {
    public:
        std::vector<SnowStamp> Update(SnowStamp contact, float speed, bool supported)
        {
            std::vector<SnowStamp> result;
            if (!supported || !contact.IsValid() || !std::isfinite(speed))
            {
                m_hasPrevious = false;
                return result;
            }
            const float dx = contact.x - m_previous.x, dy = contact.y - m_previous.y;
            const float distance = std::sqrt(dx*dx + dy*dy);
            const bool changed = !m_hasPrevious || contact.kind != m_previous.kind;
            const float spacing = contact.kind == 1 ? .18f : contact.kind == 2 ? .28f : .35f;
            if (!changed && (speed < (contact.kind == 2 ? .2f : .6f) || distance < spacing)) return result;
            const auto previous = m_previous;
            m_previous = contact;
            m_hasPrevious = true;
            if (contact.kind == 0)
            {
                m_right = !m_right;
                const float offset = m_right ? .13f : -.13f;
                contact.x -= std::sin(contact.yaw) * offset;
                contact.y += std::cos(contact.yaw) * offset;
            }
            const int steps = !changed && contact.kind == 1 && distance < 1.5f
                ? std::clamp(int(std::ceil(distance / .24f)), 1, 8) : 1;
            for (int i = 1; i <= steps; ++i)
            {
                auto stamp = contact;
                if (steps > 1)
                {
                    const float t = float(i) / steps;
                    stamp.x = previous.x + (contact.x - previous.x) * t;
                    stamp.y = previous.y + (contact.y - previous.y) * t;
                    stamp.z = previous.z + (contact.z - previous.z) * t;
                }
                result.push_back(stamp);
            }
            return result;
        }
    private:
        SnowStamp m_previous;
        bool m_hasPrevious = false, m_right = false;
    };
}
