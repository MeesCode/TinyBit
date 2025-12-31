
text(255, 255, 255, 255)

-- variables for physics
y = 64
dy = 0
r = 0

-- game started
start = false

-- variables for pipes
pipe_v_dist = 30
pipe_h_dist = 70
pipe_x = 90
pipes = {}

log("Flappy Bird clone started")

tune = [[
	L:1/8
	Q:1/4=105
	K:G
	V:1
	[e/2c/2][ce][ec][c/2A/2][ce] g/2z3z/2 |: c/2zG/2 zE/2zAB^A/2=A | (3Geg a=f/2gec/2 d/2B/2z :|
	V:2
	E4 G4 |: C4 z4 | G4 D4 :| 
	V:1
	zg/2^f/2 =f/2^de^G/2A/2cA/2c/2=d/2 | zg/2^f/2 =f/2^dec'c'/2 c'/2z3/2 zg/2^f/2 =f/2^de^G/2A/2cA/2c/2=d/2 |
	V:2
	z G3 E4 | z G3 c4 | z G3 E4 | 
	V:1
	z^d/2z=d/2z c/2z3z/2 |]
	V:2
	z ^D3 C/z3z/2 |]
]]

point_sfx = "c/4d/4e/4g/4"
flap_sfx = "d/4"

music(tune)

-- number sprites
num_spr = {
	[1] = {x = 2, w = 5},
	[2] = {x = 7, w = 8},
	[3] = {x = 15, w = 8},
	[4] = {x = 23, w = 8},
	[5] = {x = 31, w = 8},
	[6] = {x = 39, w = 8},
	[7] = {x = 47, w = 8},
	[8] = {x = 55, w = 8},
	[9] = {x = 63, w = 8},
	[0] = {x = 71, w = 8}
}

points = 0

-- reset the game to the initial state
function reset()
	y = 64
	dy = 0
	start = false
	points = 0
	pipe_x = 90
	r = 0

	for i=1, 3 do
		pipes[i] = random(30, 85)
	end
end

-- check if two rectangles overlap
function overlap(x1, y1, w1, h1, x2, y2, w2, h2)
	return x1 < x2 + w2 and
		   x1 + w1 > x2 and
		   y1 < y2 + h2 and
		   y1 + h1 > y2
end

-- reset the game before we start
reset()

function _draw()

	-- control bird using the UP button
	if btnp(UP) then
		dy = -1.8
		start = true
		r = -30
		sfx(flap_sfx)
	end

	-- reset with Z button
	if btnp(A) then
		reset()
	end

	-- physics
	y = y + dy

	-- update bird acceleration
	if start then
		dy = dy + 0.08
		pipe_x = pipe_x - 0.4
		r = r + 1.5
	end

	-- clear previous screen
	cls()

	-- draw moving background
	stroke(0,0,0,0,0)
	fill(78, 192, 202, 255)
	rect(0, 0, 128, 76)
	sprite(0, 75, 87, 128-75, -((millis()//90) % 87), 75, 87, 128-75)
	sprite(0, 75, 87, 128-75, -((millis()//90) % 87) + 87, 75, 87, 128-75)
	sprite(0, 75, 87, 128-75, -((millis()//90) % 87) + 87*2, 75, 87, 128-75)

	-- draw pipes
	for i = 1,3 do
		-- top 
		sprite(97, 52, 26, 15, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] - pipe_v_dist, 26, 13)
		sprite(97, 51, 26, 1, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] - pipe_v_dist-60, 26, 60)
		-- bottom
		sprite(97, 69, 26, 15, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] + pipe_v_dist, 26, 13)
		sprite(97, 82, 26, 1, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] + pipe_v_dist+12, 26, 60)
	end

	-- debug info
	-- fill(0, 0, 0, 100)
	-- cursor(2, 114)
	-- print("Height: " .. math.floor(y) .. "\nPipe x: " .. math.floor(pipe_x))

	-- draw bird
	local sp = (millis() // 250) % 4
	if sp == 0 then
		sprite(101, 5, 17, 12, 20, y, 17, 12, r)
	elseif sp == 1 or sp == 3 then
		sprite(101, 18, 17, 12, 20, y, 17, 12, r)
	elseif sp == 2 then
		sprite(101, 31, 17, 12, 20, y, 17, 12, r)
	end

	-- draw hitboxes
	-- fill(0,0,0,0)
	-- stroke(1,255,0,0,255)
	-- rect(20, y, 17, 12)
	-- rect(pipe_x//1, pipes[1] - pipe_v_dist - 60, 26, 73)
	-- rect(pipe_x//1, pipes[1] + pipe_v_dist, 26, 73)

	-- check if bird and pipe overlap
	-- reset the game when a pipe is hit
	if overlap(20, y, 17, 12, pipe_x//1, pipes[1] - pipe_v_dist - 60, 26, 73) or
	   overlap(20, y, 17, 12, pipe_x//1, pipes[1] + pipe_v_dist, 26, 73) or 
	   y <= 0 or y >= 128 then
		reset()
	end

	-- generate new pipe and award point
	if pipe_x <= -26 then
		pipe_x = pipe_x + pipe_h_dist
		pipes[1] = pipes[2]
		pipes[2] = pipes[3]
		pipes[3] = random(30, 85)
		points = points + 1
		sfx(point_sfx)
		log(points)
	end

	-- draw points on screen
	-- edge case when no points
	if points == 0 then
		sprite(71, 2, 6, 8, 61, 2, 6, 8)
	else 
		local temp_points = points

		-- get total width
		local total_points_w = 0
		while temp_points > 0 do
			local num = temp_points % 10
			total_points_w = total_points_w + num_spr[num].w
			temp_points = temp_points//10
		end

		-- set rightmost drawing position
		local points_x = 64 + total_points_w//2

		-- print points on screen
		temp_points = points
		while temp_points > 0 do
			local num = temp_points % 10
			points_x = points_x - num_spr[num].w
			temp_points = temp_points//10
			sprite(num_spr[num].x, 2, num_spr[num].w, 8, points_x, 2, num_spr[num].w, 8)
		end
	end

end