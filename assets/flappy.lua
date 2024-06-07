
-- variables for physics
y = 64
dy = 0

-- game started
start = false

-- variables for pipes
pipe_v_dist = 30
pipe_h_dist = 70
pipe_x = 90
pipes = {}

points = 0

-- reset the game to the initial state
function reset()
	y = 64
	dy = 0
	start = false
	points = 0
	pipe_x = 90

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
	if btn(UP) then
		dy = -1.6
		start = true
	end

	-- reset with Z button
	if btn(Z) then
		reset()
	end

	-- physics
	y = y + dy

	-- update bird acceleration
	if start then
		dy = dy + 0.08
		pipe_x = pipe_x - 0.4
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

	-- draw bird
	local sp = (millis() // 250) % 3
	if sp == 0 then
		sprite(101, 5, 17, 12, 20, y, 17, 12)
	elseif sp == 1 then
		sprite(101, 18, 17, 12, 20, y, 17, 12)
	elseif sp == 2 then
		sprite(101, 31, 17, 12, 20, y, 17, 12)
	end

	-- draw pipes
	for i = 1,3 do
		-- top 
		sprite(97, 52, 26, 15, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] - pipe_v_dist, 26, 13)
		sprite(97, 51, 26, 1, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] - pipe_v_dist-60, 26, 60)
		-- bottom
		sprite(97, 69, 26, 15, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] + pipe_v_dist, 26, 13)
		sprite(97, 82, 26, 1, pipe_x//1 + (pipe_h_dist*(i-1)), pipes[i] + pipe_v_dist+12, 26, 60)
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
		print(points)
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
			if num == 1 then
				total_points_w = total_points_w + 5
			else 
				total_points_w = total_points_w + 8
			end
			temp_points = temp_points//10
		end

		-- set rightmost drawing position
		local points_x = 64 + total_points_w//2

		-- print points on screen
		temp_points = points
		while temp_points > 0 do
			local num = temp_points % 10

			if num == 1 then
				num_x = 2
				num_w = 3
			end
			if num >= 2 then
				num_x = 7 + 8*(num-2)
				num_w = 6
			end
			if num == 0 then
				num_x = 71
				num_w = 6
			end

			points_x = points_x - num_w - 2
			temp_points = temp_points//10
			sprite(num_x, 2, num_w, 8, points_x, 2, num_w, 8)
		end
	end

end