player = {
	inputs = {
		move_up = false,
		move_down = false,
		move_left = false,
		move_right = false,
		mousemotion = {x=0,y=0,xrel=0,yrel=0} --x,y,xrel,yrel
	},
	health = 50
}
function player:update()
	self.health = self.health - math.random(0, 5)
	print(self.health)
	if self.health == 0 then
		print("this instance is fucking dead my guy")
	end

	--handle events
	if self.input.move_up then
		print("smash that button")
	end
	if self.input.move_down then
		print("like")
	end

	print("mouse condition: " .. self.input.mousemotion.x .. self.input.mousemotion.y)
end
