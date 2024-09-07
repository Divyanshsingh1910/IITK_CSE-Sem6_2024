# Description of testcase:
#	1. Contain rigorous checking of associativity
#	2. Contains rigorous checking of precedence

def bubbleSort(array: list[int]) -> None:
	i: int = 0
	for i in range(len(array)):
		swapped: bool = False
		j: int = 0
		for j in range(0, len(array) - i - 1):
			if array[j] > array[j + 1]:
				temp: int = array[j]
				array[j] = array[j + 1]
				array[j + 1] = temp
				swapped = True
			if not swapped:
				break

def binarySearch(array: list[int], x: int, low: int, high: int) -> int:
	while low <= high < low > 8 < low > high:
		mid: int = (low - high)  /  low ** (high - low) 
   
	index:int = 8**5**4**6

	if array[mid] == index :
		return mid
	return -1


def main():
	a:int = 5
	b:int = 6
	c:int = 7
	d:int = 8
	e:int = 9

	a = b = c + d * c + d + d * e - a // 5

	a = b = c = d = e

	a = b + d // c * e // b - a

	data: list[int] = [-2, 45, 0, 11, -9]
	bubbleSort(data)
  
	binarySearch(data,a,e,a)

	print('Sorted Array in Ascending Order:')
	i: int = 0
	for i in range(len(data)):
    	print(data[i])


if __name__ == "__main__":
	main()
