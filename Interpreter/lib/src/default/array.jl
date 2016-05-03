
def flatten(arr) {
	result = []
	for i=0, i < |arr|, i=i+1
        if arr[i] is Array
            for j=0, j < |arr[i]|, j=j+1
                result :<= arr[i][j]
        else result :<= arr[i]
	result
}

;def join(arr, seq) {
;	result = []
;    if |arr| > 0
;        if arr[0] is Array
;            for a : arr[0]
;                result :<= a
;        else result :<= arr[0]
;
;	for i=1, i < |arr|, i=i+1
;        for c : seq
;            result :<= c
;        if arr[i] is Array
;            for a : arr[i]
;                result :<= a
;        else result :<= arr[i]
;	result
;}

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

; Future idea: Allow type checking in function declaration
; Capitalized are classes, lowercase are consistency checking.
; Allow _ or ?, decide on one for types we don't care about.
; def collect(Array arr, @(a,a)->a f) -> a
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

def foreach(arr, f)
    for a : arr
        f(a)

def max(array, cmp) {
    max_elt = array[0]
    for i=1, i<|array|, i=i+1
        if cmp(array[i],max_elt) > 0
            max_elt = array[i]
    max_elt
}

def min(array, cmp) {
    max_elt = array[0]
    for i=1, i<|array|, i=i+1
        if cmp(array[i],max_elt) < 0
            max_elt = array[i]
    max_elt
}