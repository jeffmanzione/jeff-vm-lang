
def flatten(arr) {
	result = []
	for i=0, i < |arr|, i=i+1
		for j=0, j < |arr[i]|, j=j+1
			result:(|result|) <- arr[i][j]
	result
}

def map(arr, f) {
    res = []
    for a : arr
        res :<= f(a)
    res
}

def filter(arr, f) {
    res = []
    for a : arr
        if f(a)
            res :<= a
    res
}

def collect(arr, f) {
    res = arr[0]
    for i=1, i < |arr|, i=i+1
        res = f(res, arr[i])
    res
}

def collecti(arr, f, i) {
    res = i
    for a : arr
        res = f(res, a)
        
    res
}