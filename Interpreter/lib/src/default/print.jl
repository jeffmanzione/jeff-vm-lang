def print_str(str)
    for i=0, i < |str|, i=i+1
        print(str[i])

def println(var) {
    if var is Object
        print_str(var.to_s())
    else print(var)
	print_str('\n')
}
		
def print_strln(str) {
	print_str(str)
	print_str('\n')
}

def print_strs(a) print_str(flatten(a))

def print_strsln(a) print_strln(flatten(a))