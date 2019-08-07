#!/usr/bin/python3
import sys
import numpy as np
import matplotlib.pyplot as plt

name = sys.argv[1]

file_len = "res_len.out"
file_vel = "res_vel.out"

data_len = np.loadtxt(file_len, skiprows=21)
data_vel = np.loadtxt(file_vel, skiprows=21)

plt.clf()
plt.plot(data_len[:, 0], data_len[:, 3], linewidth=0.7, color='b', label="length")
plt.plot(data_vel[:, 0], data_vel[:, 3], linewidth=0.7, color='r', label="velocity")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("dip.png")

plt.clf()
plt.plot(data_len[:, 0], data_len[:, 4], linewidth=0.7, color='b', label="length")
plt.plot(data_vel[:, 0], data_vel[:, 4], linewidth=0.7, color='r', label="velocity")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("norm.png")

plt.clf()
plt.plot(data_len[:, 0], data_len[:, 5], linewidth=0.7, color='b', label="length")
plt.plot(data_vel[:, 0], data_vel[:, 5], linewidth=0.7, color='r', label="velocity")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("energy.png")
