def bubbleSort(array: list[int]) -> None:
  i: int = 0
  k: int = len(array) 
  for i in range(k):
    swapped: bool = False
    j: int = 0
    for j in range(0, k - i - 1):
      if array[j] > array[j + 1]:
        temp: int = array[j]
        array[j] = array[j + 1]
        array[j + 1] = temp
        swapped = True
    if not swapped:
      break


def main():
  data: list[int] = [-2, 45,-1006, 0, 11, -9, 56,0,45]
  bubbleSort(data)

  print('Sorted Array in Ascending Order:')
  i: int = 0
  k: int = len(data)
  for i in range(k):
    print(data[i])


if __name__ == "__main__":
  main()
