; equals_test
import 'lib_src/print.jl'

class Animal {
    field name, age
    def new(name, age) {
        self.name = name
        self.age = age
    }
    
    def eq(other) {
        return self.name == other.name & self.age == other.age
    }
}

class Dog : Animal {
    field breed
    def new(name, age, breed) : (name, age) {
        self.breed = breed
    }
}

class Cat : Animal {
    field size
    def new(name, age, size) : (name, age) {
        self.size = size
    }
}

def test_is(obj, cls) {
    print_strln(if obj is cls 'Success' else 'Failure')
}

def test_isnt(obj, cls) {
    print_strln(if obj isnt cls 'Success' else 'Failure')
}

def main {
    dog1 = Dog.new('dog1', 4, 'Daschund')
    cat1 = Cat.new('cat1', 1, 3)
    
    test_is(dog1, Dog)
    test_is(dog1, Animal)
    
    test_is(cat1, Cat)
    test_is(cat1, Animal)
    
    test_isnt(dog1, Cat)
    test_isnt(cat1, Dog)
    
    println(hash_uint32_t_(5))
    println(hash_uint32_t_(7.2))
    println(hash_uint32_t_('abcdef'))
    println(hash_uint32_t_([1, 2, 3, 4, 5, 6]))
    println(hash_uint32_t_([1, 2, 3, 4, 5]))
    println(hash_uint32_t_([1, 2, 3, 4]))
    println(hash_uint32_t_([1, 2, 3]))
    println(hash_uint32_t_([1, 2]))
    println(hash_uint32_t_([1]))
    println(hash_uint32_t_([]))    
    
}