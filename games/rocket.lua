
log("testing rocket game")

-- set physics
x = 64
y = 64
dy = 0
dx = 0
r = 0

stars = {}

for i=1,30 do 
    star = {
        x = math.random(0, 128),
        y = math.random(0, 128),
    }
    table.insert(stars, star)
end

text(255,255,255,255)

running = true

function _draw()

    if not running then
        if btn(UP) then
            running = true
            x = 64
            y = 64
            dy = 0
            dx = 0
            r = 0
        end
        return
    end

    cls()

    -- gravity
    dy = dy + 0.01

    -- in the air
    if y < 90 and running then

        -- control
        if btn(LEFT) then
            r = r - 2.5
        end
        if btn(RIGHT) then
            r = r + 2.5
        end
        if btn(UP) then
            dx = dx + 0.025 * math.sin(math.rad(r))
            dy = dy - 0.025 * math.cos(math.rad(r))
        end

    -- on the ground
    else 

        dx = 0
        dy = 0

        if btn(UP) then
            dx = dx + 0.025 * math.sin(math.rad(r))
            dy = dy - 0.025 * math.cos(math.rad(r))
        end

    end

    -- physics
    y = y + dy
    x = x + dx

    -- wrap
    if x < -10 then
        x = 128 + 10
    end 
    if x > 128 + 10 then
        x = -10
    end

    -- draw stars
    stroke(1, 255, 255, 255, 255)
    for _, star in ipairs(stars) do
        -- move stars
        star.x = star.x - 0.2
        if star.x < -10 then
            star.x = 128 + 10
            star.y = math.random(0, 128)
        end

        line(star.x, star.y, star.x, star.y)
    end

    -- draw planet
    sprite(0, 106, 128, 22, 0, 106, 128, 22)

    -- log("x: " .. x .. ", y: " .. y .. ", r: " .. r .. ", dx: " .. dx .. ", dy: " .. dy)
    
    -- draw rocket
    if btn(UP) and running then
        if(millis() % 100 < 50) then
            sprite(52, 23, 12, 26, x-6, y, 12, 26, r)
        else
            sprite(66, 23, 12, 26, x-6, y, 12, 26, r)
        end
    else
        sprite(80, 23, 12, 26, x-6, y, 12, 26, r)
    end

    -- collision with planet
    if y >= 90 and (dy > 0.3 or math.abs(dx) > 0.3 or math.abs(r) > 18) then
        running = false
        log("game over")
        sprite(97, 15, 23, 24, x-11, y, 23, 24)
        cursor(64 - (10*4)/2, 60)
        text(255,255,255,255)
        print("game over")
    end

    cursor(4, 114)
    fill(0, 0, 0, 200)
    stroke(255, 255, 255, 255, 255)
    print("x:".. math.floor(x) .. "\ny:" .. math.floor(y))

end