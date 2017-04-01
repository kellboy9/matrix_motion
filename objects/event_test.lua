event_test = {
	inputs = {
		whoup = false,
		like = false
	},
	likes = 0
}

function event_test:update() 
	if self.inputs.whoup then
		print("SMASH THAT MF BUTTON")
	end
	if self.inputs.like then
		self.likes = self.likes + 1
		print(self.likes .. " likes smashed!")
		print("#SMAESHED")
	end
end
