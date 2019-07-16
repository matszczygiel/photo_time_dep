#!/usr/bin/python3

import sys
import numpy as np
import matplotlib.pyplot as plt

infile = sys.argv[1]
data = np.loadtxt(infile, skiprows=17)
plt.plot(data[:, 0], data[:, 3])
plt.savefig("res.png")
