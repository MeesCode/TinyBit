
log("testing rocket game")

-- set variables
x = 0
y = 0
dy = 0
dx = 0
r = 0
stars = {}
points = 0
m_timer = 0
m_x = 0
m_y = 0
m_dx = 0
m_dy = 0
running = true
landing = false
flip_timer = 0
landing_timer = 0
prev_speed = 0

point_sfx = "c/4d/4e/4g/4"

text(rgba(255, 255, 255, 255))

for i=1,30 do 
    star = {
        x = math.random(0, 128),
        y = math.random(0, 128),
    }
    table.insert(stars, star)
end

function reset()
    x = 64
    y = 99
    dy = 0
    dx = 0
    r = 0
    points = 0
    m_x = -100
    m_y = -100
    m_dx = 0
    m_dy = 0
    running = true
    m_timer = millis()
    landing = false
    flip_timer = 0
    landing_timer = 0
    log("game reset")
end

reset()
fill(rgba(0, 0, 0, 150))

function _draw()

    if not running then
        if btnp(UP) or btnp(A) then
            reset()
        end
        return
    end

    cls()

    -- gravity
    dy = dy + 0.01

    -- in the air
    if y < 99 and running then

        -- control
        if btn(LEFT) then
            r = (r - 3.5)
        end
        if btn(RIGHT) then
            r = (r + 3.5)
        end
        if btn(UP) or btn(A) then
            dx = dx + 0.025 * math.sin(math.rad(r))
            dy = dy - 0.025 * math.cos(math.rad(r))
        end

    -- on the ground
    else 


        if landing then
            points = points + 1
            landing = false
            log("landing successful! points: " .. points)
            landing_timer = millis()
            sfx(point_sfx)
        end

        dx = 0
        dy = 0

        if btn(UP) or btn(A) then
            dx = dx + 0.025 * math.sin(math.rad(r))
            dy = dy - 0.025 * math.cos(math.rad(r))
        end

    end

    if flip_timer > 0 and millis() - flip_timer < 1000 then
        cursor(x, y - 15)
        print("flip!")
    end

    if landing_timer > 0 and millis() - landing_timer < 1000 then
        cursor(x, y - 15)
        print("landing!")
    end

    -- went up high enough to start landing
    if y < 64 then
        landing = true
    end

    -- points for flip
    if math.abs(r) > 360 then
        r = r % 360
        points = points + 1
        log("flip! points: " .. points)
        flip_timer = millis()
        sfx(point_sfx)
    end

    -- physics
    y = y + dy
    x = x + dx
    m_x = m_x + m_dx
    m_y = m_y + m_dy

    -- sound effect
    speed = math.abs(dx + dy)
   log(prev_speed, speed)
    --if prev_speed > 0.5 and speed < 0.5 then
    --    log("stop music")
    --    music("")
    -- end
    if prev_speed < 0.5 and speed > 0.5 then
        music("V:NOISE C2")
        log("low")
    end 
    if prev_speed < 1.2 and speed > 1.2 then
        music("V:NOISE E2")
        log("high")
    end
    prev_speed = speed

    -- wrap
    if x < -10 then
        x = 128 + 10
    end 
    if x > 128 + 10 then
        x = -10
    end

    -- draw stars
    stroke(1, rgba(255, 255, 255, 255))
    for _, star in ipairs(stars) do
        -- move stars
        star.x = star.x - 0.2
        if star.x < -10 then
            star.x = 128 + 10
            star.y = math.random(0, 128)
        end

        line(star.x, star.y, star.x, star.y)
    end

    -- draw meteor
    sprite(14, 20, 22, 20, m_x-11, m_y-10, 22, 20, (millis() * 0.1) % 360)

    -- draw planet
    sprite(0, 106, 128, 22, 0, 106, 128, 22)

    -- log("x: " .. x .. ", y: " .. y .. ", r: " .. r .. ", dx: " .. dx .. ", dy: " .. dy)
    
    -- draw rocket
    if (btn(UP) or btn(A)) and running then
        if(millis() % 100 < 50) then
            sprite(52, 23, 12, 26, x-6, y-9, 12, 26, r)
        else
            sprite(66, 23, 12, 26, x-6, y-9, 12, 26, r)
        end
    else
        sprite(80, 23, 12, 26, x-6, y-9, 12, 26, r)
    end

    -- collision with planet or meteor
    if (y >= 99 and (dy > 0.4 or math.abs(dx) > 0.4 or (r > 20 and r < 340)) or 
        (math.sqrt(math.abs(x-m_x)*math.abs(x-m_x) + math.abs(y-m_y)*math.abs(y-m_y)) < 17)) then
        running = false
        log("game over")
        sprite(97, 15, 23, 24, x-11, y-13, 23, 24)
        cursor(64 - (10*4)/2, 60)
        print("game over")
    end

    -- draw points
    cursor(4, 122)
    stroke(255, rgba(255, 255, 255, 255))
    if points >= 0 then
        print("points:" .. points)
    end

    -- spawn a meter every 10 seconds
    if millis() - m_timer > 5000 then
        m_dx = math.random(-1, 1) * 0.7
        m_dy = math.random(-1, 1) * 0.7
        if m_dx == 0 and m_dy == 0 then
            m_dx = 0.7
            m_dy = 0.7
        end
        m_x = 64 - m_dx * 150
        m_y = 64 - m_dy * 150
        m_timer = millis()
        if millis() > 10000 then
            points = points + 1
            sfx(point_sfx)
            log("meteor avoided! points: " .. points)
        end
    end

end