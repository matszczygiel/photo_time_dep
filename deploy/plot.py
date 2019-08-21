#!/usr/bin/python3
import sys
import numpy as np
import matplotlib.pyplot as plt

name = sys.argv[1]

file_len = "res_len.out"
file_vel = "res_vel.out"
file_velA = "res_velA.out"

resolution=400

data_len = np.loadtxt(file_len)
data_vel = np.loadtxt(file_vel)
data_vA2 = np.loadtxt(file_velA)

plt.clf()
plt.plot(data_len[:, 0], data_len[:, 3], linewidth=0.7, color='b', label="length")
plt.plot(data_vel[:, 0], data_vel[:, 3], linewidth=0.7, color='r', label="velocity")
plt.plot(data_vA2[:, 0], data_vA2[:, 3], linewidth=0.7, color='g', label="velocity_A^2")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("dip.png", dpi=resolution)

plt.clf()
plt.plot(data_len[:, 0], data_len[:, 4], linewidth=0.7, color='b', label="length")
plt.plot(data_vel[:, 0], data_vel[:, 4], linewidth=0.7, color='r', label="velocity")
plt.plot(data_vA2[:, 0], data_vA2[:, 4], linewidth=0.7, color='g', label="velocity_A^2")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("norm.png", dpi=resolution)

plt.clf()
plt.plot(data_len[:, 0], data_len[:, 5], linewidth=0.7, color='b', label="length")
plt.plot(data_vel[:, 0], data_vel[:, 5], linewidth=0.7, color='r', label="velocity")
plt.plot(data_vA2[:, 0], data_vA2[:, 5], linewidth=0.7, color='g', label="velocity_A^2")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("energy.png", dpi=resolution)

plt.clf()
plt.plot(data_len[:, 0], data_len[:, 6], linewidth=0.7, color='b', label="length")
plt.plot(data_vel[:, 0], data_vel[:, 6], linewidth=0.7, color='r', label="velocity")
plt.plot(data_vA2[:, 0], data_vA2[:, 6], linewidth=0.7, color='g', label="velocity_A^2")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("Hint.png", dpi=resolution)