
stroke(2, 255, 0, 0, 255)

counter = 0
bpm_value = 50

x = 64
y = 0

function _music()

	-- play an few tones
	-- tone(counter % (B + 1), 3, 1, SIN)

	if counter % (B + 1) == 0 then
		bpm_value = bpm_value + 10
		bpm(bpm_value)
	end

	counter = counter + 1

end

function _draw()

	-- set background
	for y=0,SCREEN_HEIGHT-1 do
		for x=0,SCREEN_WIDTH-1 do
			fill((x*2 + millis()/10) % 256, (y*2 + millis()/10) % 256, (256 - y*2 - millis()/10) % 256, 100)
			pset(x, y)
        end
	end

	-- draw some sprites
	-- sprite(0, 0, 128, 128, 0, 0, 40, 64)


	if btn(UP) then
		y = y - 1
	end
	if btn(DOWN) then
		y = y + 1
	end
	if btn(LEFT) then
		x = x - 1
	end
	if btn(RIGHT) then
		x = x + 1
	end

end