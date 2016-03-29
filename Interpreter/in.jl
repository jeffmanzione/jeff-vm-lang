import 'lib_src/math.jl'
import 'lib_src/print.jl'
import 'lib_src/sort.jl'
import 'lib_src/array.jl'

def helper1
def helper2

def main {

    println(flatten([[1, 2], [3, 4], [5, 6]]))

    arr = [1, 5, 3]
    {a, b, arr[1]} = (1, 2, 3)

    print_str('a=')
    println(a)
    print_str('b=')
    println(b)
    print_str('arr[1]=')
    println(arr[1])
    
    i = 1
    println(i)        ; 1
    println(i++)    ; 1
    println(++i)    ; 3
    println(i)        ; 3
    unsorted = 'floccinaucinihilipilification'    
    qsort(unsorted)
    print_strln(unsorted)
        
    unsorted = [9,8,7,6,5,4,3,2,1]
    unsorted:2 <- 10
    print_str('Unsorted: ')
    println(unsorted)
    qsort(unsorted)
    print_str('Sorted:   ')
    println(unsorted)
}
    
def main_5 {
    arr = ['abc', 'def', 'ghi', 'jkl', 'mno', 'qrs', 'tuv', 'wxyz']
    print_strln(arr[2])
    println(arr)
}

;def main {
;    if False println(1)
;    else println(0)
;    
;    a = [[1, 2], [3, 4], [5, 6]]
;    b = 1    
;    
;    println(a)
;    println(a[1])
;    println(a[1][1])
;    b = [0, 9, 2, 4]
;    println(b)
;    x <- b:1
;    println(x)
;    3 -> b:2
;    ; not allowed: b:2 <- 3
;    b[0] = 1
;    println(b)
;    
;    print(flatten(a), a, 5, 8)
;}
 
; main
;def main_3 {
;    a = [1, 2, 3, 4, 5]
;    b = [-3, -2, -1, 0]
;    
;    a :<= 6
;    a :<= 7
;    a :<= 8
;    0 =>: a
;    
;    print(a)
;    ;for i=0, i < |a|, i=i+1
;    ;    print(a[i])
;
;    x <=: a
;    print(x)
;    a :=> x
;    print(x)
;    
;    ;for i=0, i < |a|, i=i+1
;    ;    print(a[i])
;    
;    print(a, b)
;    
;    ;b :>> a
;    ;print(|a|)
;    ;print(|b|)
;    
;    ;a[2] = 9
;    ;print(a[2] + 1)
;    ;print([1, 2, 3, 4][2])
;}

def main_2 {
    print(sqrt(2.0))
    print(pow(4,1) as Int )
    print(pow(4,2) as Float )
    print(pow(4,3) as Int )
    print(pow(4,4) as Int )
    print(pow(4,5.551))
}

def main_1 {

    for x = 1, sqr(x) < 50, x++
        print(sqr(x))
    
    x = 1 ; set x to 1
    
    while sqr(x) < 50
        print(sqr(x++))
    
    helper1
    helper2
    
    if x > 5 exit(1)
    
    print(5)
}

def helper1 {
    print(sqr(4))
    print(pow(4,1))
    print(pow(4,2))
    print(pow(4,3))
    print(pow(4,4))
    
    foo = 1.1
    bar = 5f
    print(foo * bar)
    
    print(sqr(2.5))
}

def helper2 {
    foo = 1
    bar = 2
    
    foo = foo + bar
    bar = foo
    
    {baz1, baz2, baz3} = func(bar-3)    ; call func()
    print(baz1)    
    print(baz2)    
    print(baz3)    
    
    {baz1, baz2, baz3} = (1, 2, 3)
    print(baz1)    
    print(baz2)   
    print(baz3)    
    
    print(foo + bar)
    
    fun_if(4)
}

def func(x) {
    print(x)
    print(5)
    100, 200, 300
}

def fun_if(y) {
    if y > 5
        print(-5)
    else if y > 3
        print(-3)
    else
        print(-2)
}
