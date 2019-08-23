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


fft_len = np.abs(np.fft.rfft(data_len[:, 3]))**2
fft_vel = np.abs(np.fft.rfft(data_vel[:, 3]))**2
fft_vA2 = np.abs(np.fft.rfft(data_vA2[:, 3]))**2

ev_to_au = 0.03674930495

len_omega = 0.0
vel_omega = 0.0
vA2_omega = 0.0

for item in open(file_len, 'r').readlines():
    if "OPT_OMEGA_EV" in item:
            len_omega = float(item.split()[-1]) * ev_to_au

for item in open(file_vel, 'r').readlines():
    if "OPT_OMEGA_EV" in item:
            vel_omega = float(item.split()[-1]) * ev_to_au

for item in open(file_velA, 'r').readlines():
    if "OPT_OMEGA_EV" in item:
            vA2_omega = float(item.split()[-1]) * ev_to_au

fftfreq_len = np.fft.rfftfreq(data_len[:, 0].size, data_len[1, 0] - data_len[0, 0]) / len_omega
fftfreq_vel = np.fft.rfftfreq(data_vel[:, 0].size, data_vel[1, 0] - data_vel[0, 0]) / vel_omega
fftfreq_vA2 = np.fft.rfftfreq(data_vA2[:, 0].size, data_vA2[1, 0] - data_vA2[0, 0]) / vA2_omega


plt.clf()
plt.yscale('log')
plt.plot(fftfreq_len, fft_len, linewidth=0.7, color='b', label="length")
plt.plot(fftfreq_vel, fft_vel, linewidth=0.7, color='r', label="velocity")
plt.plot(fftfreq_vA2, fft_vA2, linewidth=0.7, color='g', label="velocity_A^2")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("fft_d2.png", dpi=resolution)

plt.clf()
plt.yscale('log')
plt.plot(fftfreq_len, fft_len*fftfreq_len**2, linewidth=0.7, color='b', label="length")
plt.plot(fftfreq_vel, fft_vel*fftfreq_vel**2, linewidth=0.7, color='r', label="velocity")
plt.plot(fftfreq_vA2, fft_vA2*fftfreq_vA2**2, linewidth=0.7, color='g', label="velocity_A^2")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("fft_w2d2.png", dpi=resolution)

plt.clf()
plt.yscale('log')
plt.plot(fftfreq_len, fft_len*fftfreq_len**4, linewidth=0.7, color='b', label="length")
plt.plot(fftfreq_vel, fft_vel*fftfreq_vel**4, linewidth=0.7, color='r', label="velocity")
plt.plot(fftfreq_vA2, fft_vA2*fftfreq_vA2**4, linewidth=0.7, color='g', label="velocity_A^2")
plt.title(name)
plt.grid()
plt.legend()
plt.savefig("fft_w4d2.png", dpi=resolution)
