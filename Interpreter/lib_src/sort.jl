def partition(a, lo, hi) {
	{p, i} = (a[hi], lo)
	for j=lo, j < hi, j=j+1
		if a[j] <= p {
			a[i] <-> a[j]
			i = i+1
		}
		
	a[i] <-> a[hi]
	i
}

def quicksort(a, lo, hi)
	if lo < hi {
		p = partition(a, lo, hi)
		quicksort(a, lo, p-1)
		quicksort(a, p+1, hi)
	}

def qsort(a) quicksort(a, 0, |a|-1)
