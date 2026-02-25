
lines = {
    {x1=10, y1=10, x2=118, y2=10},
    {x1=118, y1=10, x2=118, y2=118},
    {x1=118, y1=118, x2=10, y2=118},
    {x1=10, y1=118, x2=10, y2=10},
}

qix_polygon = {
    {x1=10, y1=10, x2=118, y2=10},
    {x1=118, y1=10, x2=118, y2=118},
    {x1=118, y1=118, x2=10, y2=118},
    {x1=10, y1=118, x2=10, y2=10},
}

tune = [[
X:1
T:by Fabian Fabro (FirahFabe)
L:1/4
Q:1/4=130
M:4/4
K:C
V:1
.B,/.B,/ B,/4A,/4 z/4 B,/4 z/4 D/4 z/4 B,/4 z/4 ^F,/4A,/ | .B,/.B,/ B,/4A,/4 z/4 B,/4 z/4 F/4 z/4 E/4 z/4 D/4A,/ |
.B,/.B,/ B,/4A,/4 z/4 B,/4 z/4 D/4 z/4 B,/4 z/4 ^F,/4A,/ | .B,/.B,/ .B,/A,/4B,/4 z2 ||  ^f3/2 =f/- f z/ d/4e/4 | =f/>e/- e/d/ ^c/>d/- d/e/ | ^f3/2 b/- b B/^c/ |
d/>e/- e/d/ ^c/>a/- a/g/ |  [Bd^f]3/2 [Bd=f]/- [Bdf] z/ d/4e/4 | [G^c=f]/>e/- e/d/ [^Ac]/>d/- d/e/ | [B^f]3/2 [=fb]/- [fb] b/^c'/ |[bd']/>e'/- e'/g/ ^f/>d'/- d'/e'/ |
[Bd^f]3/2 [Bd=f]/- [Bdf] z/ d/4e/4 | [GB=f]/>e/- e/d/ [G^c]/>d/- d/e/ | [Bd^f]3/2 [^c=fb]/- [cfb] B/c/ | d/>e/- e/d/ ^c/>a/- a/g/ |  [Bd^f]3/2 [Bd=f]/- [Bdf] z/ d/4e/4 |
[d^f]3/2 [=fb]/- [fb] B/^c/ | d/>g/- g/^f/ =f/>d'/- d'/^a/ |]
V:2
.B,,/.B,,/ B,,/4A,,/4 z/4 B,,/4 z/4 D,/4 z/4 B,,/4 z/4 ^F,,/4A,,/ | .B,,/.B,,/ B,,/4A,,/4 z/4 B,,/4 z/4 F,/4 z/4 E,/4 z/4 D,/4A,,/ |
.B,,/.B,,/ B,,/4A,,/4 z/4 B,,/4 z/4 D,/4 z/4 B,,/4 z/4 ^F,,/4A,,/ | .B,,/.B,,/ .B,,/A,,/4B,,/4 z2 ||  [B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z | [B,,B,]2 [^F,,^F,]2 | [B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z |
[^C,^C]2 [^F,,^F,]2 |  [B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z | [B,,B,]2 [^F,,^F,]2 | [B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z | [^C,^C]2 [^F,,^F,]2 |
[B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z | [B,,B,]2 [^F,,^F,]2 | [B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z | [^C,^C]2 [^F,,^F,]2 |  [B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z |
[B,,^F,B,]3/2 [^G,,D,=F,]/- [G,,D,F,] z | [^C,^C]2 [^F,,^F,]2 |]
]]

-- check if two lines are the same line (regardless of direction)
function is_equal_lines(line1, line2)
    return (line1.x1 == line2.x1 and line1.y1 == line2.y1 and line1.x2 == line2.x2 and line1.y2 == line2.y2) or
           (line1.x1 == line2.x2 and line1.y1 == line2.y2 and line1.x2 == line2.x1 and line1.y2 == line2.y1)
end

-- split a line into two lines at the point (x, y)
function split_line(x, y)
    for i, line in ipairs(lines) do
        -- if the point is on the line, split it into two lines with the point as the shared endpoint
        -- but only if the point is not already an endpoint of the line, otherwise we would end up with duplicate lines
        if is_between_points(x, y, line) and not ((x == line.x1 and y == line.y1) or (x == line.x2 and y == line.y2)) then
            table.remove(lines, i)
            table.insert(lines, {x1=line.x1, y1=line.y1, x2=x, y2=y})
            table.insert(lines, {x1=x, y1=y, x2=line.x2, y2=line.y2})
            return
        end
    end 
end

-- check if a point is between two other points (inclusive)
-- only works for horizontal or vertical lines, not diagonal lines
function is_between_points(x, y, line)
    return (x == line.x1 and x == line.x2 and ((y >= line.y1 and y <= line.y2) or (y >= line.y2 and y <= line.y1))) or
           (y == line.y1 and y == line.y2 and ((x >= line.x1 and x <= line.x2) or (x >= line.x2 and x <= line.x1)))
end

-- right-hand turn priority for clockwise traversal
-- in screen coords (y down), CW rotation: (x,y) -> (-y, x)
function turn_priority(hx, hy, dx, dy)
    if dx == -hy and dy == hx then return 1 end -- right turn
    if dx == hx and dy == hy then return 2 end  -- straight
    if dx == hy and dy == -hx then return 3 end -- left turn
    return 4                                     -- u-turn
end

-- from a point (x, y) find the polygon that contains it using CW wall following
-- cast a ray right to find the nearest vertical wall, then follow edges using the right-hand rule
function find_polygon_lines(x, y)
    local polygon_lines = {}

    -- cast ray right: find the nearest vertical line segment that the ray crosses
    local hit_line = nil
    local hit_x = nil
    for _, l in ipairs(lines) do
        if l.x1 == l.x2 and l.x1 > x then
            local min_y = l.y1 < l.y2 and l.y1 or l.y2
            local max_y = l.y1 > l.y2 and l.y1 or l.y2
            if y >= min_y and y <= max_y then
                if hit_x == nil or l.x1 < hit_x then
                    hit_x = l.x1
                    hit_line = l
                end
            end
        end
    end

    if not hit_line then return polygon_lines end

    -- start CW traversal: go downward along the hit line
    -- the closing vertex is the upward endpoint (where the cycle will complete)
    local vx, vy, close_vx, close_vy
    if hit_line.y1 >= hit_line.y2 then
        vx, vy = hit_line.x1, hit_line.y1
        close_vx, close_vy = hit_line.x2, hit_line.y2
    else
        vx, vy = hit_line.x2, hit_line.y2
        close_vx, close_vy = hit_line.x1, hit_line.y1
    end

    table.insert(polygon_lines, hit_line)
    local hx, hy = 0, 1 -- heading DOWN
    local prev_line = hit_line

    for _ = 1, 1000 do
        if vx == close_vx and vy == close_vy then
            break
        end

        local best_line = nil
        local best_priority = 5
        local best_ox, best_oy = nil, nil
        local best_dx, best_dy = nil, nil

        for _, l in ipairs(lines) do
            if not is_equal_lines(l, prev_line) then
                local ox, oy = nil, nil
                if l.x1 == vx and l.y1 == vy then
                    ox, oy = l.x2, l.y2
                elseif l.x2 == vx and l.y2 == vy then
                    ox, oy = l.x1, l.y1
                end
                if ox then
                    local dx = ox - vx
                    local dy = oy - vy
                    if dx ~= 0 then dx = dx / math.abs(dx) end
                    if dy ~= 0 then dy = dy / math.abs(dy) end
                    local p = turn_priority(hx, hy, dx, dy)
                    if p < best_priority then
                        best_priority = p
                        best_line = l
                        best_ox, best_oy = ox, oy
                        best_dx, best_dy = dx, dy
                    end
                end
            end
        end

        if not best_line then break end

        table.insert(polygon_lines, best_line)
        vx, vy = best_ox, best_oy
        hx, hy = best_dx, best_dy
        prev_line = best_line
    end

    return polygon_lines
end

-- check if a point is on a line
function is_line(x2, y2, l)
    for _, line in ipairs(l) do
        if is_between_points(x2, y2, line) then
            return true
        end
    end
    return false
end

-- raycasting algorithm to check if a point is inside a polygon
-- since there are only horizontal and vertical lines, we only have if check vertical line
-- ignore the border line
function is_unclaimed(x, y)
    local count = 0
    for _, line in ipairs(qix_polygon) do
        if line.x1 == line.x2 and line.x1 > x then
            local min_y = line.y1 < line.y2 and line.y1 or line.y2
            local max_y = line.y1 > line.y2 and line.y1 or line.y2
            if y >= min_y and y < max_y then
                count = count + 1
            end
        end
    end
    return count % 2 == 1
end

-- draw partial polygon (list of line segments)
function draw_partial_polygon(partial_polygon)
    stroke(1, rgba(0, 255, 255, 255))
    for _, l in ipairs(partial_polygon) do
        line(l.x1, l.y1, l.x2, l.y2)
    end
end

fill_crossings = {}

function fill_polygon(polygon, color)
    local pixels = 0
    local min_y, max_y = 999, -999
    for _, l in ipairs(polygon) do
        if l.y1 < min_y then min_y = l.y1 end
        if l.y2 < min_y then min_y = l.y2 end
        if l.y1 > max_y then max_y = l.y1 end
        if l.y2 > max_y then max_y = l.y2 end
    end

    local crossings = fill_crossings
    stroke(1, color)
    for y = min_y, max_y - 1 do
        local n = 0
        for _, l in ipairs(polygon) do
            if l.x1 == l.x2 then -- only vertical edges create crossings
                local ey_min = l.y1 < l.y2 and l.y1 or l.y2
                local ey_max = l.y1 > l.y2 and l.y1 or l.y2
                if y >= ey_min and y < ey_max then
                    n = n + 1
                    crossings[n] = l.x1
                end
            end
        end
        -- clear stale entries from previous iteration
        for i = n + 1, #crossings do crossings[i] = nil end
        table.sort(crossings)
        for i = 1, n - 1, 2 do
            pixels = pixels + crossings[i+1] - crossings[i]
            line(crossings[i], y, crossings[i+1] - 1, y)
        end
    end
    return pixels
end

-- the enemy
Sparx = {
    x = 64,
    y = 10,
    dx = 0,
    dy = 0,

    draw = function(self)
        stroke(1, rgba(255, 255, 0, 255))
        line(self.x-2, self.y-2, self.x + 2, self.y + 2)
        line(self.x-2, self.y+2, self.x + 2, self.y - 2)
    end,

    move = function(self)
        -- check for valid directions (permanent lines + player trail)
        local trail = s.partial_polygon
        local valid_directions = {}
        if (is_line(self.x + 1, self.y, lines) or is_line(self.x + 1, self.y, trail)) and not (self.dx == -1 and self.dy == 0) then table.insert(valid_directions, {dx=1, dy=0}) end
        if (is_line(self.x - 1, self.y, lines) or is_line(self.x - 1, self.y, trail)) and not (self.dx == 1 and self.dy == 0) then table.insert(valid_directions, {dx=-1, dy=0}) end
        if (is_line(self.x, self.y + 1, lines) or is_line(self.x, self.y + 1, trail)) and not (self.dx == 0 and self.dy == -1) then table.insert(valid_directions, {dx=0, dy=1}) end
        if (is_line(self.x, self.y - 1, lines) or is_line(self.x, self.y - 1, trail)) and not (self.dx == 0 and self.dy == 1) then table.insert(valid_directions, {dx=0, dy=-1}) end

        -- pick one of the valid directions at random
        if #valid_directions == 0 then
            -- dead end: reverse direction (allow U-turn)
            self.dx = -self.dx
            self.dy = -self.dy
        else
            local dir = valid_directions[random(1, #valid_directions)]
            self.dx = dir.dx
            self.dy = dir.dy
        end

        self.x = self.x + self.dx
        self.y = self.y + self.dy
    end,

    is_colliding_with_player = function (self, player)
        if (player.x == self.x and player.y == self.y) or (player.x == self.x - self.dx and player.y == self.y - self.dy) then
            return true
        end
        return false
    end
}

-- the enemy
Qix = {
    x = 64,
    y = 64,
    dx = 0,
    dy = 0,

    draw = function(self)
        stroke(1, rgba(255, 0, 255, 255))
        line(self.x-2, self.y-2, self.x + 2, self.y + 2)
        line(self.x-2, self.y+2, self.x + 2, self.y - 2)
    end,

    move = function(self)
        -- simple random movement
        if math.random() < 0.2 then
            if math.random() then
                local direction = math.random(4)
                if direction == 1 then
                    self.dx = 1
                    self.dy = 0
                elseif direction == 2 then
                    self.dx = -1
                    self.dy = 0
                elseif direction == 3 then
                    self.dx = 0
                    self.dy = 1
                elseif direction == 4 then
                    self.dx = 0
                    self.dy = -1
                end
            end
        end

        if not is_line(self.x + self.dx, self.y + self.dy, lines) then
            self.x = self.x + self.dx
            self.y = self.y + self.dy
        end

    end,

    is_colliding_with_player = function (self, player)
        for _, l in ipairs(player.partial_polygon) do
            if is_between_points(self.x, self.y, l) then
                return true
            end
        end
        return false
    end
}

-- the player
Stix = {
    x = 64,
    y = 118,
    fx = 64.0,
    fy = 118.0,
    dx = 0,
    dy = 0,
    speed = 0.6,
    partial_polygon = {},  -- list of {x1, y1, x2, y2} line segments
    free = false,
    left_line = false,

    draw = function(self)
        if self.free then
            stroke(1, rgba(255, 0, 0, 255))
        else
            stroke(1, rgba(255, 255, 255, 255))
        end
        line(self.x-2, self.y-2, self.x + 2, self.y + 2)
        line(self.x-2, self.y+2, self.x + 2, self.y - 2)
    end,

    -- check if a point is on the trail (excluding the live segment being extended)
    is_on_trail = function(self, px, py)
        for i = 1, #self.partial_polygon - 1 do
            if is_between_points(px, py, self.partial_polygon[i]) then
                return true
            end
        end
        return false
    end,

    move = function(self)
        local rdx, rdy = 0, 0
        if btn(UP) then rdy = -1
        elseif btn(DOWN) then rdy = 1
        elseif btn(LEFT) then rdx = -1
        elseif btn(RIGHT) then rdx = 1
        end

        -- start free mode
        if btn(A) and not self.free then
            self.free = true
            self.left_line = false
        end

        if self.free then
            -- cancel free mode if haven't left the line yet
            if not self.left_line and not btn(A) then
                self.free = false
                self.dx = 0
                self.dy = 0
                return
            end

            -- apply requested direction
            if rdx ~= 0 or rdy ~= 0 then
                if self.left_line then
                    local axis_changed = (self.dx ~= 0 and rdy ~= 0) or (self.dy ~= 0 and rdx ~= 0)
                    if axis_changed then
                        -- round position at the turn point
                        self.fx = math.floor(self.fx + 0.5)
                        self.fy = math.floor(self.fy + 0.5)
                        self.x = self.fx
                        self.y = self.fy
                        -- finalize the current trail line endpoint
                        if #self.partial_polygon > 0 then
                            local last = self.partial_polygon[#self.partial_polygon]
                            last.x2 = self.x
                            last.y2 = self.y
                        end
                        -- start a new trail line from the turn point
                        table.insert(self.partial_polygon, {x1=self.x, y1=self.y, x2=self.x, y2=self.y})
                    end
                end
                self.dx = rdx
                self.dy = rdy
            end

            -- no direction yet, wait for input
            if self.dx == 0 and self.dy == 0 then
                draw_partial_polygon(self.partial_polygon)
                return
            end

            -- still on the line, slide at integer speed
            if not self.left_line then
                local nx, ny = self.x + self.dx, self.y + self.dy
                if is_line(nx, ny, lines) then
                    self.x = nx
                    self.y = ny
                    self.fx = nx
                    self.fy = ny
                    return
                else
                    if not is_unclaimed(nx, ny) then
                        return
                    end
                    -- leaving the line, start first trail line
                    self.left_line = true
                    table.insert(self.partial_polygon, {x1=self.x, y1=self.y, x2=self.x, y2=self.y})
                end
            end

            -- free movement at reduced speed
            local nfx = self.fx + self.dx * self.speed
            local nfy = self.fy + self.dy * self.speed

            -- check if we crossed an integer boundary (speed < 1 so at most one per frame)
            local crossed = false
            local check_x, check_y = self.x, self.y

            if self.dx > 0 then
                local cx = math.floor(nfx)
                if cx > self.fx then crossed = true; check_x = cx end
            elseif self.dx < 0 then
                local cx = math.ceil(nfx)
                if cx < self.fx then crossed = true; check_x = cx end
            end
            if self.dy > 0 then
                local cy = math.floor(nfy)
                if cy > self.fy then crossed = true; check_y = cy end
            elseif self.dy < 0 then
                local cy = math.ceil(nfy)
                if cy < self.fy then crossed = true; check_y = cy end
            end

            if crossed then
                -- self-collision (can't cross own trail)
                if self:is_on_trail(check_x, check_y) then
                    draw_partial_polygon(self.partial_polygon)
                    return
                end

                -- check if we've returned to a permanent line
                if is_line(check_x, check_y, lines) then
                    -- snap to the integer landing point
                    self.fx = check_x
                    self.fy = check_y
                    self.x = check_x
                    self.y = check_y
                    -- finalize trail endpoint
                    if #self.partial_polygon > 0 then
                        local last = self.partial_polygon[#self.partial_polygon]
                        last.x2 = self.x
                        last.y2 = self.y
                    end

                    self.free = false
                    self.left_line = false
                    -- add trail lines to the field
                    for _, tl in ipairs(self.partial_polygon) do
                        table.insert(lines, tl)
                    end
                    split_line(self.partial_polygon[1].x1, self.partial_polygon[1].y1)
                    local last_trail = self.partial_polygon[#self.partial_polygon]
                    split_line(last_trail.x2, last_trail.y2)
                    self.partial_polygon = {}
                    self.dx = 0
                    self.dy = 0
                    qix_polygon = find_polygon_lines(q.x, q.y)
                    return
                end

                -- can't enter claimed territory
                if not is_unclaimed(check_x, check_y) then
                    draw_partial_polygon(self.partial_polygon)
                    return
                end
            end

            -- move
            self.fx = nfx
            self.fy = nfy
            self.x = math.floor(self.fx + 0.5)
            self.y = math.floor(self.fy + 0.5)
            -- update live trail endpoint
            if #self.partial_polygon > 0 then
                local last = self.partial_polygon[#self.partial_polygon]
                last.x2 = self.x
                last.y2 = self.y
            end
            draw_partial_polygon(self.partial_polygon)
        else
            -- line movement at integer speed
            if btn(B) then
                self.dx = 0
                self.dy = 0
                return
            end

            if (rdx ~= 0 or rdy ~= 0) and is_line(self.x + rdx, self.y + rdy, lines) then
                self.x = self.x + rdx
                self.y = self.y + rdy
                self.fx = self.x
                self.fy = self.y
                self.dx = rdx
                self.dy = rdy
                return
            end

            if (self.dx ~= 0 or self.dy ~= 0) and is_line(self.x + self.dx, self.y + self.dy, lines) then
                self.x = self.x + self.dx
                self.y = self.y + self.dy
                self.fx = self.x
                self.fy = self.y
            else
                self.dx = 0
                self.dy = 0
            end
        end
    end
}

s = Stix
q = Qix
sp = Sparx

function reset()
    lines = {
        {x1=10, y1=10, x2=118, y2=10},
        {x1=118, y1=10, x2=118, y2=118},
        {x1=118, y1=118, x2=10, y2=118},
        {x1=10, y1=118, x2=10, y2=10},
    }

    qix_polygon = {
        {x1=10, y1=10, x2=118, y2=10},
        {x1=118, y1=10, x2=118, y2=118},
        {x1=118, y1=118, x2=10, y2=118},
        {x1=10, y1=118, x2=10, y2=10},
    }
    
    s.x, s.y, s.fx, s.fy, s.dx, s.dy, s.partial_polygon, s.free, s.left_line = 64, 118, 64.0, 118.0, 0, 0, {}, false, false
    q.x, q.y, q.dx, q.dy = 64, 64, 0, 0
    sp.x, sp.y, sp.dx, sp.dy = 64, 10, 0, 0
end

music(tune)
function _draw() 

    cls()

    stroke(1, rgba(255, 255, 255, 255))
    fill(rgba(255, 0, 0, 200))
    sprite(10, 10, 109, 109, 10, 10, 109, 109)
    local claimed = fill_polygon(qix_polygon, rgba(0, 0, 0, 255))

    cursor(10, 120)
    fill(rgba(0, 0, 0, 255))
    text(rgb(255, 255, 255))
    local score = 100 - math.floor(claimed * 100 / (108*108))
    print("Claimed: " .. score .. "%")

    stroke(1, rgba(255, 255, 255, 255))
    for _, l in ipairs(lines) do
        line(l.x1, l.y1, l.x2, l.y2)
    end

    if q:is_colliding_with_player(s) or sp:is_colliding_with_player(s) then
        fill(0, rgba(0, 0, 0, 200))
        stroke(1, rgba(255, 255, 255, 255))
        rect(20, 55, 88, 18)
        cursor(25, 60)
        print([[
Game over!
Press B to restart.]])
        if btn(B) then
            reset()
        end
        return
    end

    if score >= 90 then
        fill(0, rgba(0, 0, 0, 200))
        stroke(1, rgba(255, 255, 255, 255))
        rect(20, 55, 88, 18)
        cursor(25, 60)
        print([[
You win! 
Press B to restart.]])
        if btn(B) then
            reset()
        end
        return
    end

    s:move()
    s:draw()
    q:move()
    q:draw()
    sp:move()
    sp:draw()

end
