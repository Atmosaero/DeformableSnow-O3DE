#include <DeformableSnow/SnowSimulation.h>
#include <DeformableSnow/SnowContactTracker.h>
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace DeformableSnow;
static int checks = 0;
static void Check(bool value, const char* message)
{
    ++checks;
    if (!value) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
int main()
{
    SnowSimulation snow;
    Check(snow.Reset({}), "default settings");
    snow.Refresh();
    Check(snow.Vertices().size() == 92001, "UE grid dimensions");
    Check(snow.Indices().size() == 336 * 272 * 6, "topology");
    const auto center = size_t(136) * 337 + 168;
    Check(snow.Stamp({}), "foot stamp");
    auto field = snow.Snapshot();
    Check(std::abs(field.heights[center] + .12f) < 1e-6f, "foot depth in metres");
    Check(*std::max_element(field.heights.begin(), field.heights.end()) > .05f, "raised bank");
    const auto before = field.heights[center];
    snow.Stamp({.35f, 0, 0, .36f, 0, 0});
    Check(snow.Snapshot().heights[center] <= before, "bank never refills trench");
    snow.Refresh();
    for (const auto& v : snow.Vertices())
    {
        const float dot = v.normal[0]*v.tangent[0]+v.normal[1]*v.tangent[1]+v.normal[2]*v.tangent[2];
        Check(std::abs(dot) < 1e-5f, "orthogonal tangent frame");
    }
    snow.Reset({});
    snow.Stamp({0,0,0,.7f,0,1});
    Check(std::abs(snow.Snapshot().heights[center] + .16f) < 1e-6f, "rolling depth");
    snow.Reset({});
    snow.Stamp({0,0,0,.66f,0,2});
    Check(std::abs(snow.Snapshot().heights[center] + .14f) < 1e-6f, "ragdoll depth");
    snow.Advance(45);
    Check(std::abs(snow.Snapshot().heights[center] + .14f / std::exp(1.f)) < 1e-6f, "recovery half life");
    snow.Advance(1000);
    Check(snow.Dirty() && snow.Snapshot().heights[center] == 0, "last recovery step stays dirty");
    snow.Refresh();
    Check(!snow.Stamp({1e30f, 0, 0, .36f, 0, 0}), "far stamp safely rejected before integer cast");
    Check(!snow.Stamp({0,0,5,.36f,0,0}), "airborne stamp rejected");
    Check(!snow.Stamp({0,0,0,-1,0,0}), "negative radius rejected");
    Check(!snow.Stamp({0,0,0,.36f,0,3}), "invalid kind rejected");
    Check(!snow.Stamp({std::numeric_limits<float>::quiet_NaN(),0,0,.36f,0,0}), "NaN rejected");
    auto invalid = SnowSettings{}; invalid.columns = 0;
    Check(!snow.Reset(invalid), "invalid dimensions rejected");
    SnowSimulation server, client;
    server.Reset({}); client.Reset({});
    server.Stamp({});
    server.Advance(.25f);
    server.Stamp({1,0,0,.66f,1,2});
    for (const auto& event : server.Journal()) Check(client.Replay(event), "ordered replay");
    Check(client.Snapshot().heights == server.Snapshot().heights, "replica equals authority");
    const auto duplicate = server.Journal().back();
    Check(client.Replay(duplicate), "duplicate is idempotent");
    auto gap = duplicate; gap.sequence += 2;
    Check(!client.Replay(gap), "gap requires snapshot");
    server.Advance(23);
    Check(client.Restore(server.Snapshot()), "late join full snapshot");
    Check(client.Snapshot().heights == server.Snapshot().heights, "late join retains aged tracks");
    auto bad = server.Snapshot(); bad.heights[0] = std::numeric_limits<float>::infinity();
    Check(!client.Restore(bad), "bad snapshot rejected atomically");
    Check(client.Snapshot().heights == server.Snapshot().heights, "invalid snapshot does not mutate field");
    SnowContactTracker feet, rolling;
    auto foot = feet.Update({}, 1, true);
    Check(foot.size() == 1 && foot[0].y > 0, "first foot");
    Check(feet.Update({}, 0, true).empty(), "stationary contact does not stamp repeatedly");
    foot = feet.Update({.4f,0,0,.36f,0,0}, 1, true);
    Check(foot.size() == 1 && foot[0].y < 0, "alternating foot");
    Check(feet.Update({1,0,0,.36f,0,0}, 1, false).empty(), "airborne contact");
    rolling.Update({0,0,0,.7f,0,1}, 1, true);
    Check(rolling.Update({1,0,0,.7f,0,1}, 1, true).size() == 5, "continuous rolling trail");
    Check(rolling.Update({5,0,0,.7f,0,1}, 1, true).size() == 1, "teleport does not bridge trail");
    std::cout << checks << " checks passed\n";
}
