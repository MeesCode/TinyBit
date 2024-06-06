

y = 64
dy = 0
start = false

pipe_v_dist = 30
pipe_h_dist = 70

start_time = millis()
running_time = 0
points = 0

pipes = {}

pipes[1] = -200
for i=2, 3 do
	pipes[i] = random(30, 85)
end


function reset()
	y = 64
	dy = 0
	start = false
	running_time = 0
	points = 0
	
	-- don't show first pipe
	pipes[1] = -200
end

function _draw()

	-- process input

	if btn(UP) then
		dy = -1.6

		-- start game on first up press
		if not start then
			start = true
			start_time = millis()
		end
	end

	if btn(Z) then
		reset()
	end

	-- physics

	y = y + dy

	if start then
		dy = dy + 0.08
		running_time = millis() - start_time
	end

	-- draw moving background

	sprite(0, 0, 92, 128, -((math.floor(millis()/90)) % 92), 0, 92, 128)
	sprite(0, 0, 92, 128, -((math.floor(millis()/90)) % 92) + 92, 0, 92, 128)
	sprite(0, 0, 92, 128, -((math.floor(millis()/90)) % 92) + 92*2, 0, 92, 128)

	-- draw bird

	sp = (millis() / 250)//1 % 3
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
		sprite(95, 52, 28, 15, 20 + (points-1)*pipe_h_dist + (pipe_h_dist*i) - running_time/50, pipes[i] - pipe_v_dist, 28, 13)
		sprite(95, 51, 28, 1, 20 + (points-1)*pipe_h_dist + (pipe_h_dist*i) - running_time/50, pipes[i] - pipe_v_dist-60, 28, 60)
		-- bottom
		sprite(95, 69, 28, 15, 20 + (points-1)*pipe_h_dist + (pipe_h_dist*i) - running_time/50, pipes[i] + pipe_v_dist, 28, 13)
		sprite(95, 82, 28, 1, 20 + (points-1)*pipe_h_dist + (pipe_h_dist*i) - running_time/50, pipes[i] + pipe_v_dist+12, 28, 60)

	end

	-- check if bird and pipe overlap

	-- hitboxes
	-- stroke(0,0,0,0,0)
	-- fill(255, 0, 0, 255)
	-- rect(22 + (points-1)*pipe_h_dist + (pipe_h_dist*2) - running_time/50, pipes[2] + pipe_v_dist, 26, 1)
	-- 
	-- fill(0, 0, 255, 255)
	-- rect(22 + (points-1)*pipe_h_dist + (pipe_h_dist*2) - running_time/50, pipes[2] - pipe_v_dist + 12, 26, 1)
	-- 
	-- fill(0,0,0,0)
	-- stroke(1, 0,255,0,255)
	-- rect(20, y, 17, 12)

	collision_pipe_x = 22 + (points-1)*pipe_h_dist + (pipe_h_dist*2) - running_time/50
	if (y + 12 >= pipes[2] + pipe_v_dist or y - 12 <= pipes[2] - pipe_v_dist) and (37 >= collision_pipe_x) then
		reset()
	end

	-- bird passed pipe
	-- generate new pipe and award point
	if points*pipe_h_dist + pipe_h_dist - running_time/50 < -30 then
		pipes[1] = pipes[2]
		pipes[2] = pipes[3]
		pipes[3] = random(30, 85)
		points = points + 1
		print(points)
	end

end