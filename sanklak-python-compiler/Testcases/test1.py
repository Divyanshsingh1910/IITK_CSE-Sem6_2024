# Non-adjacent Maximum Sum
def fib(n:int)->int:
    temp:int = 0
    if n < 0:
        return -1

    if n==1 or n==0:
        return 1
    else:
        temp = fib(n-1)
        temp += fib(n-2)
        return temp 

class Solution:
    def __init__(self):
        self.max_sum: int = 0

    def find_max_sum(self, nums: list[int]) -> int:
        length:int = len(nums)
        if length==0:
            return 0

        if length == 1:
            return nums[0]

        prev_max: int = nums[0]
        curr_max: int = nums[0]
        if curr_max < nums[1]:
            curr_max = nums[1]

        i:int = 0
        for i in range(2,length):
            temp: int = curr_max
            temp2:int = prev_max + nums[i]
            if curr_max < temp2:
                curr_max = temp2
            prev_max = temp

        return curr_max

def main() -> None:
    nums: list[int] = [1, 2, 3, 4, 5, 6, 7]
    print("Array: \n")
    n:int = len(nums)
    i:int = 0
    for i in range(n):
        print(nums[i])
    soln: Solution = Solution()
    j:int = 4
    print("5th element of the list is: ")
    print(nums[j])
    print("Fibonacci of 5th element is: ")
    temp:int = nums[j]
    temp = fib(temp)
    print(temp)

    max_sum: int = soln.find_max_sum(nums)
    print("Maximum sum of non-adjacent elements: ")
    print(max_sum)

if __name__ == "__main__":
    main()
