# display an odonnell grid

import numpy as np
import matplotlib.pyplot as plt

from recomputeODonnell import ODonnellParamenters



def displayGrid(grid):
    plt.figure()
    plt.imshow(np.array(grid), interpolation='nearest')
    plt.colorbar()
    plt.title("0.0 is colliding, 1.0 is clear")
    plt.show()