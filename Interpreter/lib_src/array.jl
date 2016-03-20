
def flatten(arr) {
	result = []
	for i=0, i < |arr|, i=i+1
		for j=0, j < |arr[i]|, j=j+1
			result:(|result|) <- arr[i][j]
	result
}