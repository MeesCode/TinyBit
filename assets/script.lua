
print(SCREEN_HEIGHT)

function _draw()

	for y=0,SCREEN_HEIGHT do
		for x=0,SCREEN_WIDTH do
			fill(x % SCREEN_WIDTH, y % SCREEN_HEIGHT, 0, 255)
			pset(x, y)
        end
	end

	sprite(0, 0, 256, 256, millis()/10, 256, 256, 256)
	sprite(0, 0, 100, 256, 0, 0, 100, 256)
	sprite(0, 0, 256, 256, 256, 0, 256, 256, millis()/10, 0)

	stroke(5, 255, 0, 0, 255)
	fill(0, 255, 0, 100)
	rect(300, 50, 50, 50)


end