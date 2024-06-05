

y = 64
dy = 0
start = false

function _draw()

	-- process input

	if btn(UP) then
		dy = -2.2
		start = true
	end

	if btn(Z) then
		y = 64
		dy = 0
		start = false
	end

	-- physics

	y = y + dy

	if start then
		dy = dy + 0.10
		-- print(dy)
	end

	cls()

	-- draw moving background

	sprite(0, 0, 92, 128, -((millis()/60) % 92), 0, 92, 128)
	sprite(0, 0, 92, 128, -((millis()/60) % 92) + 92, 0, 92, 128)
	sprite(0, 0, 92, 128, -((millis()/60) % 92) + 92*2, 0, 92, 128)

	-- draw bird

	sp = (millis() / 250)//1 % 3
	-- print(sp)

	if sp == 0 then
		sprite(101, 5, 17, 12, 40, y, 17, 12)
	elseif sp == 1 then
		sprite(101, 18, 17, 12, 40, y, 17, 12)
	elseif sp == 2 then
		sprite(101, 31, 17, 12, 40, y, 17, 12)
	end

	-- draw pipes



end