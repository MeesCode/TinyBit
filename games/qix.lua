
polygons = {}

polygons[1] = {
    {x=10, y=10},
    {x=118, y=10},
    {x=118, y=118},
    {x=10, y=118},
}

polygons[2] = {
    {x=64, y=10},
    {x=64, y=118},
    {x=10, y=118},
    {x=10, y=10},
}

-- check if a point is between two other points (inclusive)
-- only works for horizontal or vertical lines, not diagonal lines
function is_between_points(pointx, pointy, x1, y1, x2, y2)
    return (pointx == x1 and pointx == x2 and (pointy >= y1 and pointy <= y2 or pointy >= y2 and pointy <= y1)) or
           (pointy == y1 and pointy == y2 and (pointx >= x1 and pointx <= x2 or pointx >= x2 and pointx <= x1))
end

-- check if a point is on a line
function is_valid_move(x2, y2)
    for _, polygon in ipairs(polygons) do
        for i = 1, #polygon do
            local p1 = polygon[i]
            local p2 = polygon[i % #polygon + 1]
            if is_between_points(x2, y2, p1.x, p1.y, p2.x, p2.y) then
                return true
            end
        end
    end
    return false
end

-- raycasting algorithm to check if a point is inside a polygon
-- since there are only horizontal and vertical lines, we only have if check vertical line
function is_inside_polygons(x, y)
    local inside = false
    for _, polygon in ipairs(polygons) do
        for i = 1, #polygon do
            local p1 = polygon[i]
            local p2 = polygon[i % #polygon + 1]
            -- check for two points in the polygon if the point lies between the y values of the two points
            if (p1.y >= y and p2.y <= y) or (p1.y <= y and p2.y >= y) then
                -- if polygon line is to the right of the point, flip the inside variable
                if x > p1.x then
                    inside = not inside
                end
            end
        end
    end
    return inside
end

-- the player
Stix = {
    x = 64,
    y = 118,
    dx = 0,
    dy = 0,
    draw = function(self)
        stroke(1, rgba(255, 255, 255, 255))
        line(self.x-2, self.y-2, self.x + 2, self.y + 2)
        line(self.x-2, self.y+2, self.x + 2, self.y - 2)
    end,
    move = function(self)
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
        -- two directional input, try to move in the direction of the new input first, then try the other direction if the first direction is blocked
        if self.dx ~= 0 and self.dy ~= 0 then
            -- when horizontal user input, try horizontal move first, then vertical move
            if btn(LEFT) or btn(RIGHT) then
                if is_valid_move(self.x + self.dx, self.y) then
                    self.x = self.x + self.dx
                    self.dy = 0
                    return
                elseif is_valid_move(self.x, self.y + self.dy) then
                    self.y = self.y + self.dy
                    self.dx = 0
                    return
                end
            end
            -- when vertical user input, try vertical move first, then horizontal move
            if btn(UP) or btn(DOWN) then
                if is_valid_move(self.x, self.y + self.dy) then
                    self.y = self.y + self.dy
                    self.dx = 0
                    return
                elseif is_valid_move(self.x + self.dx, self.y) then
                    self.x = self.x + self.dx
                    self.dy = 0
                    return
                end
            end
        else
            -- no new input, just try to move in the current direction
            if is_valid_move(self.x + self.dx, self.y + self.dy) then
                self.x = self.x + self.dx
                self.y = self.y + self.dy
                return
            end
        end
    end
}

s = Stix

function draw_polygons(polygon)
    for _, polygon in ipairs(polygons) do
        for _, point in ipairs(polygon) do
            poly_add(point.x, point.y)
        end
        draw_polygon()
        poly_clear()
    end
end

function _draw() 

    cls()

    draw_polygons(polygons)

    s:move()
    s:draw()
    
end