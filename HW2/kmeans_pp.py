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


centroids_idx = []


def min_dist(point,points,centroids_idx):
    centroids = points[centroids_idx] # if centroids_idx = [i1,i2,i3] then centroids is [points[i1],points[i2],points[i3]]
    dist_sq = np.sum((centroids - point) ** 2, axis=1) #if centroids = [c1,c2,c3], point = p then dist_sq = [D(c1,p)^2, D(c2,p)^2,D(c3,p)^2]
    min_dist = float(np.sqrt(np.min(dist_sq))) #easy to see. take min over centroids and then take sqrt.
    return min_dist #the minimal distance between a given point to some centorid in centroids.

                     



def join_and_sort(file1, file2):
    df1 = pd.read_csv(file1, header=None)
    df2 = pd.read_csv(file2, header=None)

    merged = pd.merge(df1, df2, on=0, how="inner")   # inner join on column 0 (the key)
    merged = merged.sort_values(by=0)

    return merged

points = join_and_sort(file1,file2).iloc[:,1:].to_numpy(dtype=np.float64)

def kmeans_pp(points, K):

    N = points.shape[0] #num of rows(points)
    np.random.seed(1234) #setting the seed
    random_first_idx = np.random.choice(N) #chooses randomly form {0,...,N-1}
    centroids_idx.append(random_first_idx) #adding the index of the chosen centroid

    while len(centroids_idx) < K:

        D = np.zeros(N, dtype=np.float64) # D = [0,..,0]. 
        for i in range(N):
            if i in centroids_idx:
                D[i] = 0.0  #later in p_arr it will help us set probability 0 because points[i] already been chosen
            else:
                D[i] = min_dist(points[i], points, centroids_idx) 
        total = D.sum() #what to do if it is 0?

        p_arr = D / total #p_arr = [p0,p1,....pN-1] weighted probabilites
        next_idx = np.random.choice(N, p=p_arr) #chooses the i index in probability of p_arr[i]
        centroids_idx.append(int(next_idx)) #adding the new centroid

    init_centroids = np.ascontiguousarray(points[centroids_idx, :], dtype=np.float64)
    return centroids_idx, init_centroids
        

print(kmeans_pp(points,K))



#print(points)
#print(join_and_sort(file1,file2))

#print(f"fit() returned {k.fit(5,6)}")   # currently adds the two integers in C

# Try running this regularly. 
# If "mykmeanssp module doesn't exist" - run "python3 setup.py build_ext --inplace" in the folder and try again.