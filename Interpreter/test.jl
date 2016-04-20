

class Test1 {
    field x
    def new() {}
}

def test(a,b,c) {
    a+1, b+2, c+3
}

def main {
    test1 = Test1.new()
    
    {test1.x, y, z} = test(1, 2, 3)
    ;print(test1.x)
    ;print(y)
    ;print(z)
    
    arr = [1, 2, 3, 4, 5]
    
    print((test1.x, y, z)[1])

}