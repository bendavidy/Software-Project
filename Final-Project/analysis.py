import sys
import math
import numpy as np
from sklearn.metrics import silhouette_score
import mysymnmf
from symnmf import read_points, init_h, nmf_labels_from_h

MAX_ITER = 300
EPS = 1e-4

def safe_silhouette(X, labels):
    '''
    Launches sklearn.metrics.silhouette_score() if we have more than a single unique label, otherwise returns None.
    '''
    unique = len(set(labels))
    if unique < 2:
        return None
    return silhouette_score(np.array(X), labels)


def find_minimal_dist_index(i, elements, centroids, K):
    '''
    Kmeans function: finds the index of the centroid that has the min distance to i'th data point
    '''
    elem = elements[i] 
    min_dist = math.inf 
    min_dist_index = 0
    for j in range(K):
        dist = math.dist(elem, centroids[j]) 
        if dist < min_dist:
            min_dist = dist
            min_dist_index = j
    return min_dist_index



def update_assignment(elements, centroids, assignments, N, K):
    '''
    Kmeans function: assigns for each data point the cluster it belongs to
    '''
    for i in range(N):
        assignments[i] = find_minimal_dist_index(i, elements, centroids, K) 


def update_centroids(elements, centroids, assignments, N, K, d):
    '''
    Kmeans function: recomputes each centroid from the points currently assigned to its cluster
    '''
    for j in range(K):
        j_cluster = [i for i in range(N) if assignments[i] == j]
        size = len(j_cluster)
        if size == 0:
            continue
        for l in range(d):
            centroids[j][l] = sum(elements[index][l] for index in j_cluster) / size 


def kmeans_labels(elements, K, max_iter, eps):
    '''
    Kmeans function: runs the kmeans algorithm and returns the resulting assignments for the clusters.
    assignments[i] is the cluster of the i'th data point.
    '''
    N = len(elements) # number of data points
    d = len(elements[0]) # each point dimension

    assignments = [0 for i in range(N)] # [0 0 0 0 0 0 0 0 0 .... 0] N times
    centroids = [elements[i][:] for i in range(K)] #make the initial centroids the first k data points 

    for i in range(max_iter): # In this project's case max_iter = 300
        old_centroids = [c[:] for c in centroids] # go over each c in centroids and make a copy of it and put in old_centroids

        update_assignment(elements, centroids, assignments, N, K) # assign for each data point the cluster it belongs to in this iteration
        update_centroids(elements, centroids, assignments, N, K, d) # update the centroids according to the data points that are currently in each cluster

        max_shift = float("-inf") # Until convergence
        for j in range(K):
            shift = math.dist(old_centroids[j], centroids[j])
            if shift > max_shift:
                max_shift = shift

        if max_shift < eps:
            break

    return assignments


def main():
    try:
        if len(sys.argv) != 3:
            raise Exception()

        k = int(sys.argv[1])
        file_name = sys.argv[2]

        X = read_points(file_name)
        N = len(X)

        if not (1 < k < N): 
            raise Exception()

        W = mysymnmf.norm(X)
        H0 = init_h(W, k)
        H_final = mysymnmf.symnmf(H0, W, N, k)

        nmf_labels = nmf_labels_from_h(H_final)
        nmf_score = safe_silhouette(X, nmf_labels)

        kmeans_result_labels = kmeans_labels(X, k, MAX_ITER, EPS)
        kmeans_score = safe_silhouette(X, kmeans_result_labels)

        if nmf_score is None or kmeans_score is None:
            raise Exception()

        print(f"nmf: {nmf_score:.4f}")
        print(f"kmeans: {kmeans_score:.4f}")

    except Exception:
        print("An Error Has Occurred")


if __name__ == "__main__":
    main()