def Merge(a:list[str] , low:int, high:int, mid:int)->None:
    i:int = low
    k:int = 0
    j:int = mid + 1
    temp:list[str] = ["d","d","d","x","x"]
    i = low
    k = 0
    j = mid + 1
 
    while i <= mid and j <= high :
        if a[i] < a[j] :
            temp[k] = a[i]
            k+=1
            i+=1
        else:
            temp[k] = a[j]
            k+=1
            j+=1
 
    while i <= mid :
        temp[k] = a[i]
        k+=1
        i+=1
 
    while j <= high:
        temp[k] = a[j]
        k+=1
        j+=1

    for i in range(low,high+1):
        a[i] = temp[i-low]
    return
 
def MergeSort(a:list[str], low:int, high:int)->None:
    mid:int
    if low < high :
        mid=(low+high)/2
        MergeSort(a, low, mid)
        MergeSort(a, mid+1, high)
        Merge(a, low, high, mid)
    return

 
def main():
    i:int
    arr:list[str] = ["aarajeev","aabdf","zdfd","zadfd","divyansh"]
    n:int = len(arr)
    for i in range(n):
        print(arr[i])
        print("\n")

    MergeSort(arr, 0, n-1)
    
    print('\n------\nSorted Data:\n------\n')
    for i in range(n):
        print(arr[i])
        print(i)
        
    return 0
    
if __name__ == "__main__":
	main()
