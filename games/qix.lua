
lines = {
    {x1=10, y1=10, x2=118, y2=10},
    {x1=118, y1=10, x2=118, y2=118},
    {x1=118, y1=118, x2=10, y2=118},
    {x1=10, y1=118, x2=10, y2=10},
    {x1=64, y1=118, x2=64, y2=10},
}

function is_equal_lines(line1, line2)
    return (line1.x1 == line2.x1 and line1.y1 == line2.y1 and line1.x2 == line2.x2 and line1.y2 == line2.y2) or
           (line1.x1 == line2.x2 and line1.y1 == line2.y2 and line1.x2 == line2.x1 and line1.y2 == line2.y1)
end

-- check if a point is between two other points (inclusive)
-- only works for horizontal or vertical lines, not diagonal lines
function is_between_points(x, y, line)
    return (x == line.x1 and x == line.x2 and (y >= line.y1 and y <= line.y2 or y >= line.y2 and y <= line.y1)) or
           (y == line.y1 and y == line.y2 and (x >= line.x1 and x <= line.x2 or x >= line.x2 and x <= line.x1))
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

-- the player
Stix = {
    x = 64,
    y = 118,
    dx = 0,
    dy = 0,
    pdx = 0,
    pdy = 0,
    partial_polygon = {},
    free = false,

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
        self.pdx = self.dx
        self.pdy = self.dy
        
        if btn(UP) then
            self.dy = -1
        end
        if btn(DOWN) then
            self.dy = 1
        end
        if btn(LEFT) then
            self.dx = -1
        end
        if btn(RIGHT) then
            self.dx = 1
        end
        if btn(B) then
            self.dx = 0
            self.dy = 0
        end
        if btn(A) then
            self.free = true
        end
        if self.free and not btn(A) and is_line(self.x + self.dx, self.y + self.dy) then
            self.free = false
            self.x = self.x + self.dx
            self.y = self.y + self.dy
            self.dx = 0
            self.dy = 0

            -- add the lines to the fiels
            table.insert(self.partial_polygon, {x=self.x, y=self.y})
            for i = 1, #self.partial_polygon - 1 do
                local p1 = self.partial_polygon[i]
                local p2 = self.partial_polygon[i + 1]
                table.insert(lines, {x1=p1.x, y1=p1.y, x2=p2.x, y2=p2.y})
            end
            self.partial_polygon = {}
            return
        end
        if self.free then
            if btn(UP) or btn(DOWN) then
                self.dx = 0
            elseif btn(LEFT) or btn(RIGHT) then
                self.dy = 0
            end

            if self.pdx ~= self.dx or self.pdy ~= self.dy then
                table.insert(self.partial_polygon, {x=self.x, y=self.y})
            end

            self.x = self.x + self.dx
            self.y = self.y + self.dy

            draw_partial_polygon(self.partial_polygon, self.x, self.y)
            return
        end
        if not self.free then
            -- two directional input, try to move in the direction of the new input first, then try the other direction if the first direction is blocked
            if self.dx ~= 0 and self.dy ~= 0 then
                -- when horizontal user input, try horizontal move first, then vertical move
                if btn(LEFT) or btn(RIGHT) then
                    if is_line(self.x + self.dx, self.y) then
                        self.x = self.x + self.dx
                        self.dy = 0
                        return
                    elseif is_line(self.x, self.y + self.dy) then
                        self.y = self.y + self.dy
                        self.dx = 0
                        return
                    end
                end
                -- when vertical user input, try vertical move first, then horizontal move
                if btn(UP) or btn(DOWN) then
                    if is_line(self.x, self.y + self.dy) then
                        self.y = self.y + self.dy
                        self.dx = 0
                        return
                    elseif is_line(self.x + self.dx, self.y) then
                        self.x = self.x + self.dx
                        self.dy = 0
                        return
                    end
                end
            else
                -- no new input, just try to move in the current direction
                if is_line(self.x + self.dx, self.y + self.dy) then
                    self.x = self.x + self.dx
                    self.y = self.y + self.dy
                    return
                end
            end
        end
    end
}

s = Stix

function _draw() 

    cls()

    for _, l in ipairs(lines) do
        stroke(1, rgba(255, 255, 255, 255))
        line(l.x1, l.y1, l.x2, l.y2)
    end

    s:move()
    s:draw()
    
end