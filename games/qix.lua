
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
function is_line(x2, y2)
    for _, line in ipairs(lines) do
        if is_between_points(x2, y2, line) then
            return true
        end
    end
    return false
end

-- raycasting algorithm to check if a point is inside a polygon
-- since there are only horizontal and vertical lines, we only have if check vertical line
-- ignore the border line
function is_claimed(x, y)
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

-- take a partial poligon and the players position
-- draw lines between the points
function draw_partial_polygon(partial_polygon, x, y)
    stroke(1, rgba(0, 255, 255, 255))
    for i = 1, #partial_polygon do
        local p1 = partial_polygon[i]
        local p2 = partial_polygon[i % #partial_polygon + 1]
        if i ~= #partial_polygon then
            line(p1.x, p1.y, p2.x, p2.y)
        end
    end
    if #partial_polygon > 0 then
        local last_point = partial_polygon[#partial_polygon]
        line(last_point.x, last_point.y, x, y)
    end
end

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

        if not is_line(self.x + self.dx, self.y + self.dy) then
            self.x = self.x + self.dx
            self.y = self.y + self.dy
        end

    end,

    is_colliding_with_player = function (self, player)
        for i = 1, #player.partial_polygon do
            local p1 = player.partial_polygon[i]
            local p2
            if i == #player.partial_polygon then
                p2 = {x=player.x, y=player.y}
            else
                p2 = player.partial_polygon[i+1]
            end
            if is_between_points(self.x, self.y, {x1=p1.x, y1=p1.y, x2=p2.x, y2=p2.y}) then
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
    dx = 0,
    dy = 0,
    partial_polygon = {},
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

    move = function(self)
        -- read input into a single-axis requested direction
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
            -- apply requested direction
            if rdx ~= 0 or rdy ~= 0 then
                -- only track turns once we've left the line
                if self.left_line then
                    local axis_changed = (self.dx ~= 0 and rdy ~= 0) or (self.dy ~= 0 and rdx ~= 0)
                    if axis_changed then
                        table.insert(self.partial_polygon, {x=self.x, y=self.y})
                    end
                end
                self.dx = rdx
                self.dy = rdy
            end

            -- no direction yet, wait for input
            if self.dx == 0 and self.dy == 0 then
                draw_partial_polygon(self.partial_polygon, self.x, self.y)
                return
            end

            local nx, ny = self.x + self.dx, self.y + self.dy

            -- still on the line, just slide along it
            if not self.left_line then
                if is_line(nx, ny) then
                    self.x = nx
                    self.y = ny
                    return
                else
                    -- actually leaving the line now, record departure point
                    self.left_line = true
                    table.insert(self.partial_polygon, {x=self.x, y=self.y})
                end
            end

            -- check if we've returned to a line
            if self.left_line and is_line(nx, ny) then
                self.free = false
                self.left_line = false
                self.x = nx
                self.y = ny

                -- add the trail lines to the field
                table.insert(self.partial_polygon, {x=self.x, y=self.y})
                for i = 1, #self.partial_polygon - 1 do
                    local p1 = self.partial_polygon[i]
                    local p2 = self.partial_polygon[i + 1]
                    table.insert(lines, {x1=p1.x, y1=p1.y, x2=p2.x, y2=p2.y})
                end
                split_line(self.partial_polygon[1].x, self.partial_polygon[1].y)
                split_line(self.partial_polygon[#self.partial_polygon].x, self.partial_polygon[#self.partial_polygon].y)
                self.partial_polygon = {}
                self.dx = 0
                self.dy = 0
                qix_polygon = find_polygon_lines(q.x, q.y)
                return
            end

            -- move freely
            self.x = nx
            self.y = ny
            draw_partial_polygon(self.partial_polygon, self.x, self.y)
        else
            -- line movement: stop on B
            if btn(B) then
                self.dx = 0
                self.dy = 0
                return
            end

            -- try requested direction first
            if (rdx ~= 0 or rdy ~= 0) and is_line(self.x + rdx, self.y + rdy) then
                self.x = self.x + rdx
                self.y = self.y + rdy
                self.dx = rdx
                self.dy = rdy
                return
            end

            -- fall back to current direction (slide along line)
            if (self.dx ~= 0 or self.dy ~= 0) and is_line(self.x + self.dx, self.y + self.dy) then
                self.x = self.x + self.dx
                self.y = self.y + self.dy
            else
                self.dx = 0
                self.dy = 0
            end
        end
    end
}

s = Stix
q = Qix

game_over = false

function _draw() 

    cls()

    stroke(1, rgba(255, 255, 255, 255))
    for _, l in ipairs(lines) do
        line(l.x1, l.y1, l.x2, l.y2)
    end

    for y = 10, 118 do
        for x = 10, 118 do
            if is_claimed(x, y) then
                pset(x, y, rgba(0, 255, 255, 50))
            end
        end
    end

    s:move()
    s:draw()
    q:move()
    q:draw()

    -- qix_polygon = find_polygon_lines(q.x, q.y)
    -- log("polygon lines: " .. #qix_polygon)

    stroke(1, rgba(255, 0, 0, 255))
    for _, l in ipairs(qix_polygon) do
        line(l.x1, l.y1, l.x2, l.y2)
    end

    if q:is_colliding_with_player(s) then
        game_over = true
        log("Game Over")
    end

    if game_over then
        cursor(0, 0)
        text(rgb(255, 255, 255))
        print("Game Over")
    end
end