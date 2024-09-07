class ListOperations:
	def __init__(self):
		return;

	def is_palindrome(self, data: list[int]) -> bool:
		i: int = 0
		for i in range(len(data)):
			if data[i] != data[len(data) - 1 - i]:
				return False;
		return True

	def find_max(self, nums: list[int]) -> int:
		if len(nums) == 0:
			return 0
		max_val: int = nums[0]
		i: int = 0
		for i in range(len(nums)):
			if nums[i] > max_val:
				max_val = nums[i]
		return max_val

	def reverse_list(self, items: list[int]) -> list[int]:
		i: int = 0;
		j: int = len(items) - 1;
		while(i < j):
			temp: int = items[i]
			items[i] = items[j]
			items[j] = temp
			i += 1
			j -= 1
		return items


def main():
	checker: ListOperations = ListOperations()
	a: list[int] = [1, 2, 3, 2, 1]
	b: list[int] = [1, 1, 1, 2, 1]
	c: bool = checker.is_palindrome(a)
	d: bool = checker.is_palindrome(b)
	print(checker.is_palindrome(a))  # Output: True
	print(checker.is_palindrome(b))  # Output: False

	items: list[int] = [5, 2, 8, 1, 9]
	answer:int = checker.find_max(items)
	print(answer)  # Output: 9
	items = checker.reverse_list(items)
	i:int = 0
	for i in range(5):
		print(items[i])

    

if __name__ == "__main__":
    main()
