
print(SCREEN_HEIGHT)

function _draw()
	sprite(0, 0, 256, 256, millis()/10, 256, 256, 256)
	sprite(0, 0, 100, 256, 0, 0, 100, 256)
	sprite(0, 0, 256, 256, 256, 0, 256, 256, millis()/10, 0)

	stroke(5, 255, 0, 0, 255)
	fill(0, 255, 0, 100)
	rect(300, 50, 50, 50)
end