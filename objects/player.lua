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
	if self.health <= 0 then
		--print("this instance is fucking dead my guy")
		--kill it
	end

	--handle events
	if self.inputs.move_up then
		self.parent.pos.y = self.parent.pos.y - 1
	end
	if self.inputs.move_down then
		self.parent.pos.y = self.parent.pos.y + 1
	end
	if self.inputs.move_left then
		self.parent.pos.x = self.parent.pos.x - 1
	end
	if self.inputs.move_right then
		self.parent.pos.x = self.parent.pos.x + 1
	end
end
