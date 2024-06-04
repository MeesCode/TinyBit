
stroke(2, 255, 0, 0, 255)

counter = 0
bpm_value = 50

print(MEM_SPRITESHEET_START)

x = 0
y = 0

-- backup sprites
copy(0x20400, MEM_SPRITESHEET_START, MEM_DISPLAY_SIZE)

-- set background
for y=0,SCREEN_HEIGHT-1 do
	for x=0,SCREEN_WIDTH-1 do
		fill((x*2) % 256, (y*2) % 256, (255 - y*2) % 256, 255)
		pset(x, y)
    end
end

-- backup background
copy(0x30400, MEM_DISPLAY_START, MEM_DISPLAY_SIZE)

function _music()

	-- play an few tones
	-- for i=0,8 do
	-- 	tone(counter % (B + 1), 3, 1, SIN)
	-- 
	-- 	if counter % (B + 1) == 0 then
	-- 		bpm_value = bpm_value + 10
	-- 		bpm(bpm_value)
	-- 	end
	-- 
	-- 	counter = counter + 1
	-- end

end

function _draw()

	cls()

	-- get background
	copy(MEM_SPRITESHEET_START, 0x30400, MEM_DISPLAY_SIZE)

	-- draw some sprites
	sprite(0, 0, 128, 128, (128-(millis()/10) )% 128, (128 - (millis()/10)) % 128, 128, 128)
	sprite(0, 0, 128, 128, (128-(millis()/10) )% 128 - 128, (128 - (millis()/10)) % 128 - 128, 128, 128)
	sprite(0, 0, 128, 128, (128-(millis()/10) )% 128, (128 - (millis()/10)) % 128 - 128, 128, 128)
	sprite(0, 0, 128, 128, (128-(millis()/10) )% 128 - 128, (128 - (millis()/10)) % 128, 128, 128)

	-- get sprites
	copy(MEM_SPRITESHEET_START, 0x20400, MEM_DISPLAY_SIZE)

	-- draw some sprites
	sprite(0, 0, 128, 128, x, y, 64, 64)


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