

y = 64
dy = 0
start = false

pipe_v_dist = 30
pipe_h_dist = 70

start_time = millis()
running_time = 0

pipes = {}
for i=1, 10 do
    pipes[i] = random(40, 80)
end

function reset()
	y = 64
	dy = 0
	start = false
	running_time = 0
end

function _draw()

	-- process input

	if btn(UP) then
		dy = -1.8

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

	sprite(0, 0, 92, 128, -((millis()/70) % 92), 0, 92, 128)
	sprite(0, 0, 92, 128, -((millis()/70) % 92) + 92, 0, 92, 128)
	sprite(0, 0, 92, 128, -((millis()/70) % 92) + 92*2, 0, 92, 128)

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

	for i = 1,10 do

		-- top
		sprite(95, 52, 28, 15, 20 + pipe_h_dist*i - running_time/50, pipes[i] - pipe_v_dist, 28, 13)
		sprite(95, 51, 28, 1, 20 + pipe_h_dist*i - running_time/50, pipes[i] - pipe_v_dist-60, 28, 60)
		-- bottom
		sprite(95, 69, 28, 15, 20 + pipe_h_dist*i - running_time/50, pipes[i] + pipe_v_dist, 28, 13)
		sprite(95, 82, 28, 1, 20 + pipe_h_dist*i - running_time/50, pipes[i] + pipe_v_dist+12, 28, 60)

	end

end