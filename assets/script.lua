
stroke(2, 255, 0, 0, 255)

counter = 0
bpm_value = 50

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
	for y=0,SCREEN_HEIGHT do
		for x=0,SCREEN_WIDTH do
			fill((x*2) % 256, (y*2) % 256, (255 - y*2) % 256, 255)
			pset(x, y)
        end
	end

	-- draw some sprites
	sprite(0, 0, 256, 256, millis()/100, 64, 64, 64)
	sprite(0, 0, 100, 256, 0, 0, 40, 64)
	sprite(0, 0, 256, 256, 64, 0, 64, 64, millis()/10, 0)

	-- draw a rectangle
	fill(0, 255, 0, 100)
	rect(20, 20, 30, 40)


end