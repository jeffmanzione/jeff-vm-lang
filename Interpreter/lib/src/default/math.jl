def epsilon 0.000001

def int_pow(x, n) if 1 == n x else x*int_pow(x,n-1)

def pow(x,n) {
    {result, value, power} = (1f, x as Float, n)
    while power > epsilon {
        if power % 2 == 1
            result = result*value
        {value, power} = (value*value, power/2)
    }
    result as Int
}

def sqr(x) x*x

def sqrt(n) {
    {x, y} = (n as Float, 1f)
    while (x - y) > epsilon {
        x = (x + y) / 2f
        y = n / x
    }
    x
}