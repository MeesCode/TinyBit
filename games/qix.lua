
polygons = {}

polygons[1] = {
    {x=10, y=10},
    {x=118, y=10},
    {x=118, y=118},
    {x=10, y=118},
}

-- polygons[2] = {
--     {x=64, y=10},
--     {x=64, y=118},
--     {x=10, y=118},
--     {x=10, y=10},
-- }

-- check if a point is between two other points (inclusive)
-- only works for horizontal or vertical lines, not diagonal lines
function is_between_points(pointx, pointy, x1, y1, x2, y2)
    return (pointx == x1 and pointx == x2 and (pointy >= y1 and pointy <= y2 or pointy >= y2 and pointy <= y1)) or
           (pointy == y1 and pointy == y2 and (pointx >= x1 and pointx <= x2 or pointx >= x2 and pointx <= x1))
end

-- check if a point is on a line
function is_line(x2, y2)
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

-- find a complete polygon by taking a list of point forming a partial polygon
-- and the list of all existing polygons. Do a breadth first search to find all 
-- points that are connected to the partial polygon 
-- return the complete polygon as a list of points
function find_complete_polygon(partial_polygon, polygons)
    local complete_polygon = {}
    local visited = {}
    local queue = {}

    -- add all points in the partial polygon to the queue and mark them as visited
    for _, point in ipairs(partial_polygon) do
        table.insert(queue, point)
        visited[point] = true
    end

    while #queue > 0 do
        local point = table.remove(queue, 1)
        table.insert(complete_polygon, point)

        -- check all polygons to find points that are connected to the current point
        for _, polygon in ipairs(polygons) do
            for _, p in ipairs(polygon) do
                if is_between_points(p.x, p.y, point.x, point.y, point.x, point.y) and not visited[p] then
                    table.insert(queue, p)
                    visited[p] = true
                end
            end
        end

    end
    return complete_polygon
end

-- raycasting algorithm to check if a point is inside a polygon
-- since there are only horizontal and vertical lines, we only have if check vertical line
function is_inside_polygons(x, y)
    local inside = false
    for n, polygon in ipairs(polygons) do
        if n == 1 then
            -- skip the first polygon since it is the outer border
            goto continue
        end
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
        ::continue::
    end
    return inside
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
        if not btn(A) and is_line(self.x, self.y) then
            self.free = false
        end
        if self.free then
            if btn(UP) or btn(DOWN) then
                self.dx = 0
            elseif btn(LEFT) or btn(RIGHT) then
                self.dy = 0
            end
            if not is_inside_polygons(self.x + self.dx, self.y + self.dy) then
                self.x = self.x + self.dx
                self.y = self.y + self.dy
            else
                self.dx = 0
                self.dy = 0
            end
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

function draw_polygons(polygon)
    stroke(1, rgba(255, 255, 255, 255))
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