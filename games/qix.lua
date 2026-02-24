
lines = {
    {x1=10, y1=10, x2=118, y2=10},
    {x1=118, y1=10, x2=118, y2=118},
    {x1=118, y1=118, x2=10, y2=118},
    {x1=10, y1=118, x2=10, y2=10},
}

function is_equal_lines(line1, line2)
    return (line1.x1 == line2.x1 and line1.y1 == line2.y1 and line1.x2 == line2.x2 and line1.y2 == line2.y2) or
           (line1.x1 == line2.x2 and line1.y1 == line2.y2 and line1.x2 == line2.x1 and line1.y2 == line2.y1)
end

function split_line(x, y)
    for i, line in ipairs(lines) do
        if is_between_points(x, y, line) then
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

-- find polygon lines that have a start point or end point that is between the start and end point of the line formed by (x1, y1) and (x2, y2)
function find_intersecting_lines(x, y)
    local intersecting_lines = {}
    for _, line in ipairs(lines) do
        if is_between_points(x, y, line) then
            table.insert(intersecting_lines, line)
        end
    end
    return intersecting_lines
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
    local intersecting_lines = find_intersecting_lines(x, y)
    local count = 0
    for _, line in ipairs(intersecting_lines) do
        if line.x1 == line.x2 and line.x1 > x then
            count = count + 1
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

    for _, l in ipairs(lines) do
        stroke(1, rgba(random(0, 255), random(0, 255), random(0, 255), 255))
        line(l.x1, l.y1, l.x2, l.y2)
    end

    s:move()
    s:draw()
    q:move()
    q:draw()

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