import 'lib_src/print.jl'
import 'lib_src/array.jl'


class Owner {
    field name, pets
    
    def new(name) {
        self.name = name
        self.pets = []
    }
    
    def adopt(pet)
        self.pets:0 <- pet
    
}
class Doge {
	field name, breed, age
	
	def new(name, breed, age) {
        self.name = name
        self.breed = breed
        self.age = age
	}
	
	def speak print_strln('Woof!') 
}

class Kitty {
    field size
    
    def new(size)
        self.size = size
        
    def speak print_strln('Meowwww!')
}

def doge_info(doge) {
    ;print_str(doge.name)
	;print_str(' is a ')
	;print_str(doge.breed)
	;print_str(' at age ')
	;print(doge.age)
    ;print_strln('.')
    print_strln(flatten([doge.name,' is a ',doge.breed,' at age ',[doge.age],'.']))
}

def main {
    for i=0, i<|Doge.methods|, i=i+1
        println(Doge.methods[i])
    
    owner1 = Owner.new('owner1')
    
	doge1 = Doge.new('doge1', 'Labrador', 2)
	doge2 = Doge.new('doge2', 'Dachshund', 5)
	kitty1 = Kitty.new(3)

    tmp = owner1;
    
    if owner1 == tmp print_strln('True')
    else print_strln('False')
    
    doge_info(doge1)
    
    print_strln((((Doge.class).class).class).name)
    
    owner1.adopt(doge1)
    owner1.adopt(doge2)
    owner1.adopt(kitty1)
    
    for i=0, i < |owner1.pets|, i=i+1
        (owner1.pets[i]).speak()
    
}

