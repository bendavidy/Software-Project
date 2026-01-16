import numpy as np
import pandas as pd
import mykmeanssp as k  # still works with the warning - as long as we launched build_ext and the .so file is present
import sys
import math

default_iter = 300
eps = 0.001

#initialaizing inputs, need to verify later their validity.
K = int(sys.argv[1])
iter = int(sys.argv[2])
eps_str = sys.argv[3]
file1 = sys.argv[4]
file2 = sys.argv[5]


def join_and_sort(file1, file2):
    df1 = pd.read_csv(file1, header=None)
    df2 = pd.read_csv(file2, header=None)

    merged = pd.merge(df1, df2, on=0, how="inner")   # inner join on column 0 (the key)
    merged = merged.sort_values(by=0)

    return merged



def kmeans_pp(points, K):
    init_centroids = 0
    return init_centroids

points = join_and_sort(file1,file2).iloc[:,1:].to_numpy(dtype=np.float64)
print(points)
print(join_and_sort(file1,file2))
print(f"fit() returned {k.fit(5,6)}")   # currently adds the two integers in C

# Try running this regularly. 
# If "mykmeanssp module doesn't exist" - run "python3 setup.py build_ext --inplace" in the folder and try again.