import matplotlib.pyplot as plt
import numpy as np
permutations = counterSet.keys()
counts = counterSet.values()

x = np.arange(len(counts))
plt.bar(x, counts)
plt.xticks(x, permutations)
plt.xlabel('permutations')
plt.ylabel('counts')
plt.title('Shuffle result')
plt.show()