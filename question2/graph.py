import matplotlib.pyplot as plt
import numpy as np
import re

file = open('output.txt', mode = 'r')
lines = file.readlines()
file.close()

skipValues, totalTime = [], []

for line in lines:
	numbers = re.findall(r'\d+', line)
	skipValues.append(int(numbers[0]))
	totalTime.append(int(numbers[1]))

plt.plot(skipValues, totalTime)
plt.title("Skip Value Vs Total Loop Duration")
plt.xlim(1,100)
plt.ylim(20000,160000)
plt.xticks(np.arange(0, 100, 5))
plt.yticks(np.linspace(20000, 240000, 23))
plt.xlabel('Skip Value')
plt.ylabel('Total Loop Duration (micro seconds)')
plt.show()
