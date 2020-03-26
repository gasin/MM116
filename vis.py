import numpy
import matplotlib.pyplot as plt

file = open("tmp.txt", "r")

lines = [list(map(float,l.split())) for l in file.readlines()]

file2 = open("output/pn.txt", "r")

lines2 = [list(map(float,l.split())) for l in file2.readlines()]

plt.scatter([l[1] for l in lines], [l[0] for l in lines2])
plt.show()

plt.scatter([l[1] for l in lines], [l[1] for l in lines2])
plt.show()