#pragma once

// CPU simulation shared by the client, dedicated server and deterministic tests.
// All distances are in metres, all positions are surface-local.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace DeformableSnow
{
    struct SnowSettings
    {
        uint32_t columns = 337, rows = 273;
        float cell = .125f, level = .18f, recoverySeconds = 45.f;
        bool IsValid() const
        {
            return columns >= 3 && columns <= 1025 && rows >= 3 && rows <= 1025
                && std::isfinite(cell) && cell >= .025f && cell <= 10.f
                && std::isfinite(level) && std::isfinite(recoverySeconds) && recoverySeconds >= 0;
        }
    };

    struct SnowStamp
    {
        float x = 0, y = 0, z = 0, radius = .36f, yaw = 0;
        uint8_t kind = 0; // foot, rolling object, ragdoll
        bool IsValid() const
        {
            return kind <= 2 && std::isfinite(x) && std::isfinite(y) && std::isfinite(z)
                && std::isfinite(radius) && radius >= .05f && radius <= 10.f && std::isfinite(yaw);
        }
    };

    struct SnowVertex
    {
        float position[3], normal[3], tangent[4], bitangent[3], uv[2], color[4];
    };

    // An authoritative snapshot includes the entire recovered field. A bounded list
    // of recent stamps alone cannot reconstruct old, partially recovered tracks.
    struct SnowSnapshot
    {
        SnowSettings settings;
        uint64_t sequence = 0;
        double time = 0;
        std::vector<float> heights;
    };

    struct SnowEvent
    {
        uint64_t sequence = 0;
        double time = 0;
        SnowStamp stamp;
    };

    class SnowSimulation
    {
    public:
        bool Reset(const SnowSettings& settings)
        {
            if (!settings.IsValid()) return false;
            m_settings = settings;
            m_heights.assign(size_t(settings.columns) * settings.rows, 0);
            m_base.resize(m_heights.size());
            m_vertices.resize(m_heights.size());
            m_indices.clear();
            m_indices.reserve(size_t(settings.columns - 1) * (settings.rows - 1) * 6);
            m_time = 0;
            m_sequence = 0;
            m_journal.clear();
            for (uint32_t y = 0; y < settings.rows; ++y)
                for (uint32_t x = 0; x < settings.columns; ++x)
                {
                    const auto i = size_t(y) * settings.columns + x;
                    const float px = X(x), py = Y(y);
                    m_base[i] = .0135f * std::sin(px * 1.4f) * std::cos(py * 1.8f)
                        + .0055f * std::sin(px * 8.3f + py * 6.1f) * std::cos(py * 9.7f - px * 2.7f);
                    if (x + 1 < settings.columns && y + 1 < settings.rows)
                    {
                        const auto v = uint32_t(i), c = settings.columns;
                        // Atom uses counter-clockwise front faces (opposite to the UE mesh).
                        m_indices.insert(m_indices.end(), {v, v + 1, v + c, v + 1, v + c + 1, v + c});
                    }
                }
            m_dirty = true;
            return true;
        }

        bool Apply(const SnowStamp& s)
        {
            if (!s.IsValid() || m_heights.empty() || std::abs(s.z - m_settings.level) > .9f) return false;
            const float radius = std::max(.2f, s.radius);
            const float length = s.kind == 0 ? 1.3f : s.kind == 2 ? 1.6f : 1.12f;
            const float width = s.kind == 0 ? .8f : 1.f;
            const float depth = s.kind == 1 ? .16f : s.kind == 2 ? .14f : .12f;
            const float rim = s.kind == 1 ? .10f : s.kind == 2 ? .08f : .07f;
            const float extent = radius * std::max(length, width) * 1.06f;
            if (std::abs(s.x) > X(m_settings.columns - 1) + extent
                || std::abs(s.y) > Y(m_settings.rows - 1) + extent) return false;
            const int cx = int(std::round(s.x / m_settings.cell + (m_settings.columns - 1) * .5f));
            const int cy = int(std::round(s.y / m_settings.cell + (m_settings.rows - 1) * .5f));
            const int range = int(std::ceil(extent / m_settings.cell)) + 2;
            const float cs = std::cos(s.yaw), sn = std::sin(s.yaw);
            bool changed = false;
            for (int y = std::max(0, cy - range); y <= std::min(int(m_settings.rows) - 1, cy + range); ++y)
                for (int x = std::max(0, cx - range); x <= std::min(int(m_settings.columns) - 1, cx + range); ++x)
                {
                    const float px = X(x), py = Y(y), dx = px - s.x, dy = py - s.y;
                    const float along = (dx * cs + dy * sn) / (radius * length);
                    const float across = (-dx * sn + dy * cs) / (radius * width);
                    const float edge = 1.f + .055f * std::sin(px * 19.f + py * 13.f) * std::sin(py * 31.f - px * 7.f);
                    const float r = std::sqrt(along * along + across * across) * edge;
                    if (r >= 1) continue;
                    const float d = r < .55f ? -depth * (1.f - std::pow(r / .55f, 4.f))
                        : rim * std::sin((r - .55f) / .45f * 3.14159265358979323846f);
                    auto& h = m_heights[size_t(y) * m_settings.columns + x];
                    const float old = h;
                    if (d < 0) h = std::max(-.17f, std::min(h, d));
                    else if (h >= 0) h = std::min(.12f, std::max(h, d));
                    changed |= old != h;
                }
            m_dirty |= changed;
            return changed;
        }

        bool Stamp(const SnowStamp& stamp)
        {
            if (!Apply(stamp)) return false;
            m_journal.push_back({++m_sequence, m_time, stamp});
            if (m_journal.size() > 512) m_journal.erase(m_journal.begin(), m_journal.begin() + 128);
            return true;
        }

        void Advance(float seconds)
        {
            if (!std::isfinite(seconds) || seconds <= 0) return;
            m_time += seconds;
            if (m_settings.recoverySeconds <= 0) return;
            const float alpha = std::exp(-seconds / m_settings.recoverySeconds);
            for (auto& h : m_heights)
            {
                const float old = h;
                h *= alpha;
                if (std::abs(h) < .000001f) h = 0;
                m_dirty |= old != h;
            }
        }

        // Call on a replica using ordered server events and server time, never a
        // client-controlled stamp. False means a gap/invalid event: request snapshot.
        bool Replay(const SnowEvent& event)
        {
            if (event.sequence <= m_sequence) return true;
            if (event.sequence != m_sequence + 1 || !std::isfinite(event.time)
                || event.time < m_time || !event.stamp.IsValid()) return false;
            Advance(float(event.time - m_time));
            Apply(event.stamp);
            m_sequence = event.sequence;
            return true;
        }

        SnowSnapshot Snapshot() const { return {m_settings, m_sequence, m_time, m_heights}; }
        bool Restore(const SnowSnapshot& snapshot)
        {
            if (!snapshot.settings.IsValid() || !std::isfinite(snapshot.time) || snapshot.time < 0
                || snapshot.heights.size() != size_t(snapshot.settings.columns) * snapshot.settings.rows) return false;
            for (float h : snapshot.heights) if (!std::isfinite(h) || h < -.17f || h > .12f) return false;
            Reset(snapshot.settings);
            m_time = snapshot.time;
            m_sequence = snapshot.sequence;
            m_heights = snapshot.heights;
            return true;
        }

        void Refresh()
        {
            if (!m_dirty) return;
            for (uint32_t y = 0; y < m_settings.rows; ++y)
                for (uint32_t x = 0; x < m_settings.columns; ++x)
                {
                    const auto i = size_t(y) * m_settings.columns + x;
                    const auto xl = x == 0 ? 0 : x - 1, xr = std::min(x + 1, m_settings.columns - 1);
                    const auto yl = y == 0 ? 0 : y - 1, yr = std::min(y + 1, m_settings.rows - 1);
                    const float dx = (Height(xr, y) - Height(xl, y)) / ((xr - xl) * m_settings.cell);
                    const float dy = (Height(x, yr) - Height(x, yl)) / ((yr - yl) * m_settings.cell);
                    const float n = 1.f / std::sqrt(dx * dx + dy * dy + 1.f);
                    const float t = 1.f / std::sqrt(1.f + dx * dx);
                    const float compact = std::clamp(-m_heights[i] / .17f, 0.f, 1.f);
                    const float raised = std::clamp(m_heights[i] / .12f, 0.f, 1.f);
                    m_vertices[i] = {{X(x), Y(y), Height(x, y)}, {-dx * n, -dy * n, n},
                        {t, 0, dx * t, 1}, {-dy * n * dx * t, n * t * (1 + dx * dx), dy * n * t},
                        {X(x) / 3.f, Y(y) / 3.f},
                        {1 - .50f * compact + .15f * raised, 1 - .42f * compact + .13f * raised,
                         1 - .25f * compact + .09f * raised, 1}};
                }
            m_dirty = false;
        }

        float Height(uint32_t x, uint32_t y) const
        {
            const auto i = size_t(y) * m_settings.columns + x;
            return m_settings.level + m_base[i] + m_heights[i];
        }
        const SnowSettings& Settings() const { return m_settings; }
        const std::vector<SnowVertex>& Vertices() const { return m_vertices; }
        const std::vector<uint32_t>& Indices() const { return m_indices; }
        const std::vector<SnowEvent>& Journal() const { return m_journal; }
        bool Dirty() const { return m_dirty; }
    private:
        float X(uint32_t x) const { return (float(x) - (m_settings.columns - 1) * .5f) * m_settings.cell; }
        float Y(uint32_t y) const { return (float(y) - (m_settings.rows - 1) * .5f) * m_settings.cell; }
        SnowSettings m_settings;
        std::vector<float> m_base, m_heights;
        std::vector<SnowVertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::vector<SnowEvent> m_journal;
        uint64_t m_sequence = 0;
        double m_time = 0;
        bool m_dirty = false;
    };
}
