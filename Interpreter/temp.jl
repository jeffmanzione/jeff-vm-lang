import lib.src.default.print
import lib.src.default.array

class Owner {
    field name, pets
    
    def new(name) {
        self.name = name
        self.pets = []
    }
    
    def adopt(pet)
        self.pets:0 <- pet
    
}

class Pet {
	field name, say_msg
    def new(name, say_msg) {
        self.name = name
        self.say_msg = say_msg
    }
    def speak print_strln(flatten([self.name,' says ',self.say_msg,'!'])) 
    
    def eq(other)
        self.name == other.name & self.say_msg == other.say_msg
        
    def to_s flatten(['Pet[', name, ']'])
    
}

class Doge : Pet {
	field name, breed, age
    
	def new(name, breed, age) : (name, 'wooofff') {
        self.breed = breed
        self.age = age
	}
    def speak {
        super.speak()
        print_strln('BBBBAAAAAARRRRRKKKK!')
    }
}

class Kitty : Pet {
    field size
    def new(name, size) : (name, 'meooowww') self.size = size
}

def doge_info(doge) {
    print_strln(flatten([doge.name,' is a ',doge.breed,' at age ',[doge.age],'.']))
}

class Test {
    field map
    
    def new(pets) {
        self.map = []
        for i=0, i<|pets|, i=i+1
            self.map:0 <- [pets[i], i]
    }
    
    def get_map() self.map
}

def main {
    methods = flatten([Doge.methods, Doge.super.methods])
    for method : methods
        println(method)
    
    owner1 = Owner.new('owner1')
	doge1  = Doge.new('doge1', 'Labrador', 2)
	doge2  = Doge.new('doge2', 'Dachshund', 5)
	kitty1 = Kitty.new('kitty1', 3)
    
    tmp = owner1
    
    if owner1 == tmp print_strln('True')
    else print_strln('False')
    
    doge_info(doge1)
    
    println(Doge.class.class.class.fields)
    ;       Doge  Pet Object (none)
    println(Doge.super.super.super)
    
    arr = [1,2,2,4]
    arr[2]++
    println(arr)
    
    owner1.adopt(doge1)
    owner1.adopt(doge2)
    owner1.adopt(kitty1)
    
    Test test1 = Test.new([doge1, doge2, kitty1])
    println(test1.get_map())
    test1.get_map()[1][0].speak()
    
    for pet : owner1.pets
        pet.speak()
    
}

