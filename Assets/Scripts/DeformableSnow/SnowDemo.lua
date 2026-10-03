-- Add a Lua Script component to an entity with DeformableSnow and assign this file.
-- Draws three animated lanes on a horizontal snow surface; no physics actors needed.
local SnowDemo = {
    Properties = {
        SnowSurface = { default = EntityId(), description = "Snow entity; empty uses this entity" },
        StepInterval = { default = 0.12, description = "Seconds between samples" },
        Loop = { default = true, description = "Clear the snow and repeat" },
        RestartDelay = { default = 12.0, description = "Pause for natural recovery before drawing again; use at least the surface's track lifetime" }
    }
}

function SnowDemo:OnActivate()
    self.surface = self.Properties.SnowSurface
    if not self.surface:IsValid() then
        self.surface = self.entityId
    end
    self.sample = 0
    self.reportedDeformation = false
    self.started = false
    self.wait = 0.25 -- Wait for the snow component to activate before sending requests.
    self.handler = TickBus.Connect(self)
end

function SnowDemo:Stamp(world, x, y, kind, radius)
    local position = world * Vector3(x, y, 0)
    local height = SnowSurfaceRequestBus.Event.GetHeight(self.surface, position)
    if height == nil or height ~= height then
        return false -- Outside the surface.
    end
    position.z = height
    local forward = world:GetBasisY()
    return SnowSurfaceRequestBus.Event.Stamp(self.surface, position, kind,
        radius * world:GetUniformScale(), math.atan(forward.y, forward.x))
end

function SnowDemo:OnTick(deltaTime, timePoint)
    self.wait = self.wait - deltaTime
    if self.wait > 0 then return end

    if self.sample > 40 then
        if not self.Properties.Loop then
            self:OnDeactivate()
            return
        end
        self.sample = 0
    end
    if not self.started then
        SnowSurfaceRequestBus.Event.Clear(self.surface)
        self.started = true
    end

    local world = TransformBus.Event.GetWorldTM(self.surface)
    local y = -4 + self.sample * 0.2
    local probe = world * Vector3(0, y, 0)
    local before = not self.reportedDeformation
        and SnowSurfaceRequestBus.Event.GetHeight(self.surface, probe)
    local accepted = self:Stamp(world, 0, y, 1, 0.65)
    if self.sample % 2 == 0 then
        local side = (self.sample / 2) % 2 == 0 and -0.13 or 0.13
        self:Stamp(world, -2 + side, y, 0, 0.36)
    end
    if self.sample % 3 == 0 then
        self:Stamp(world, 2, y, 2, 0.66)
    end
    if not accepted then
        Debug.Warning("SnowDemo: stamp rejected. Check SnowSurface and its size.")
        self:OnDeactivate()
        return
    end

    if not self.reportedDeformation then
        local after = SnowSurfaceRequestBus.Event.GetHeight(self.surface, probe)
        if before and after and after < before then
            Debug.Log(string.format("SnowDemo: deformation verified; vertices=%d; depth=%.4f",
                SnowSurfaceRequestBus.Event.GetVertexCount(self.surface), before - after))
            self.reportedDeformation = true
        else
            Debug.Warning("SnowDemo: stamp accepted but height did not decrease.")
            self:OnDeactivate()
            return
        end
    end

    self.sample = self.sample + 1
    self.wait = self.sample > 40 and math.max(0, self.Properties.RestartDelay)
        or math.max(0.02, self.Properties.StepInterval)
end

function SnowDemo:OnDeactivate()
    if self.handler then
        self.handler:Disconnect()
        self.handler = nil
    end
end

return SnowDemo
