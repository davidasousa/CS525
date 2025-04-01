import matplotlib.pyplot as plt

# Read data from file
filename = 'part1res.txt'
times = []
threads = []

with open(filename, 'r') as file:
    for line in file:
        if 'Sec' in line and 'Threads' in line:
            # Split the line into time and threads parts
            time_part, threads_part = line.split(':')
            
            # Extract time value
            time_sec = float(time_part.split('Sec')[0].strip())
            times.append(time_sec)
            
            # Extract threads value
            thread_count = int(threads_part.split('Threads')[0].strip())
            threads.append(thread_count)

# Create scatter plot
plt.figure(figsize=(10, 6))
plt.scatter(threads, times, color='blue', alpha=0.7)
plt.title('Execution Time vs Number of Threads')
plt.xlabel('Number of Threads')
plt.ylabel('Execution Time (Seconds)')
plt.grid(True, linestyle='--', alpha=0.5)

# Annotate some points for better readability
for i in range(0, len(threads), 3):  # Annotate every 3rd point
    plt.annotate(f'{threads[i]}', (threads[i], times[i]), textcoords="offset points", xytext=(0,10), ha='center')

plt.tight_layout()
plt.show()
