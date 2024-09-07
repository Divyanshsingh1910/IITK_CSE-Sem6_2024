def binarySearch(array: list[int], x: int, low: int, high: int) -> int:
  while low <= high:
    mid: int = low + (high - low) // 2
    if array[mid] == x:
      return mid
    elif array[mid] < x:
      low = mid + 1
    else:
      high = mid - 1
  return -1


def main():
  array: list[int] = [3, 4, 5, 6, 7, 8, 9]
  n:int = len(array)
  result: int = binarySearch(array, 8, 0, n - 1)

  if result != -1:
    print("Element is present at index:\n")
    print(result)
  else:
    print("Element is not present\n")


if __name__ == "__main__":
  main()
