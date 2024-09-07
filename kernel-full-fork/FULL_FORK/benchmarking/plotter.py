import matplotlib.pyplot as plt

# Function to read data from file
def read_data(file_name):
    with open(file_name, 'r') as file:
        lines = file.readlines()
    return [list(map(int, line.split("TIME_INFO:")[1].split())) for line in lines]

# Read data from files
n = 2
Nthreads = 10
num_threads = []
num_threads_per_func = []
for a in range(Nthreads):
    num_threads.append((a + 1) * n)
    num_threads_per_func.append(a + 1)
    
data = {num: read_data(f"time_info_{num}.txt") for num in num_threads_per_func}

# Calculate t6 - t1 for all lines in each file
t6_t1 = [sum(line[5] - line[0] for line in data[num]) / len(data[num]) for num in num_threads_per_func]
t4_t3 = [sum(line[3] - line[2] for line in data[num]) / len(data[num]) for num in num_threads_per_func]
t2_t0 = [sum(line[2] - line[0] for line in data[num]) / len(data[num]) for num in num_threads_per_func]

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(num_threads, t6_t1, marker='x', label='Complete time', color='b')
plt.plot(num_threads, t4_t3, marker='x', label='One fork time', color='r')
plt.plot(num_threads, t2_t0, marker='x', label='Signal Time', color='g')
plt.xlabel('Number of Threads')
plt.ylabel('Time Difference (ns)')
plt.title('Average Timing Data vs. Number of Threads')
plt.legend()
plt.grid(True)
plt.show()
