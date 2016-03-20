
def print_str(str)
    for i=0, i < |str|, i=i+1
        print(str[i])
		
def print_strln(str) {
	print_str(str)
	print_str('\n')
}

class Doge {
	field breed, age
	
	def new(breed, age) {
		self.breed = breed
		self.age = age
	}
	
	def speak print_strln('Woof!') 
}

def main {
	doge1 = Doge.new('Lab', 2)
	doge2 = Doge.new('Dachshund', 5)
	
	print_str('doge1 is a ')
	print_strln(doge1.breed)
	doge1.speak
	
	print_str('doge2 is a ')
	print_strln(doge2.breed)
	doge2.speak
}

