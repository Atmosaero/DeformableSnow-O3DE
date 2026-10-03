#pragma once
#include <DeformableSnow/SnowSimulation.h>

namespace DeformableSnow
{
    // Distance accumulation keeps spacing independent of movement speed and tick rate.
    class SnowContactTracker
    {
    public:
        std::vector<SnowStamp> Update(SnowStamp contact, float speed, bool supported)
        {
            std::vector<SnowStamp> result;
            if (!contact.IsValid() || !std::isfinite(speed))
            {
                Reset();
                return result;
            }
            if (!supported)
            {
                m_wasAirborne = true;
                m_hasPrevious = false;
                m_distance = 0;
                return result;
            }
            const bool landing = m_wasAirborne;
            m_wasAirborne = false;
            if (!m_hasPrevious || contact.kind != m_previous.kind)
            {
                m_previous = contact;
                m_hasPrevious = true;
                m_distance = 0;
                if (landing && contact.kind == 0)
                {
                    m_right = false;
                    Emit(result, contact);
                    Emit(result, contact);
                }
                else Emit(result, contact);
                return result;
            }

            const auto previous = m_previous;
            m_previous = contact;
            const float dx = contact.x - previous.x, dy = contact.y - previous.y;
            const float distance = std::sqrt(dx * dx + dy * dy);
            if (!std::isfinite(distance) || distance >= 1.5f)
            {
                // Start a new trail at the destination; never bridge a teleport.
                m_distance = 0;
                Emit(result, contact);
                return result;
            }
            if (distance < .0001f) return result;
            const float spacing = contact.kind == 1 ? .18f : contact.kind == 2 ? .28f : .35f;
            float travelled = 0;
            while (m_distance + distance - travelled >= spacing)
            {
                travelled += spacing - m_distance;
                const float t = std::clamp(travelled / distance, 0.f, 1.f);
                auto stamp = contact;
                stamp.x = previous.x + dx * t;
                stamp.y = previous.y + dy * t;
                stamp.z = previous.z + (contact.z - previous.z) * t;
                Emit(result, stamp);
                m_distance = 0;
            }
            m_distance += distance - travelled;
            return result;
        }
    private:
        void Reset()
        {
            m_hasPrevious = m_wasAirborne = false;
            m_distance = 0;
        }
        void Emit(std::vector<SnowStamp>& result, SnowStamp contact)
        {
            if (contact.kind == 0)
            {
                m_right = !m_right;
                const float offset = m_right ? .13f : -.13f;
                contact.x -= std::sin(contact.yaw) * offset;
                contact.y += std::cos(contact.yaw) * offset;
            }
            result.push_back(contact);
        }
        SnowStamp m_previous;
        float m_distance = 0;
        bool m_hasPrevious = false, m_right = false, m_wasAirborne = false;
    };
}
