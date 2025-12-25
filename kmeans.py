import sys
import math

# TODO: validate argv length and input
K = int(sys.argv[1])     
iter = int(sys.argv[2])
input_data = [ line for line in sys.stdin ]     # read input from stdio
d = len(input_data[0].split(','))
N = len(input_data)

def convert_to_float(a : list):
    for i in range(len(a)):
        a[i] = float(a[i])
    return a

# convert to a list of lists, elements[i][j] is the j'th coordinate of the i'th element
elements = [ convert_to_float(input_data[i].split(',')) for i in range(N) ]


# assignments[i] is the index of centroids we bond the i'th element to.
# in other words:   i'th element is assigned to j'th cluster IFF assignments[i]=j where centroid[j] is the j'th cluster centroid
assignments = [ 0 for i in range(N) ]

# initialize centroids:
centroids = [elements[i] for i in range(K)]    # len(centroids) = K

def find_minimal_dist_index(i):
    elem = elements[i]
    min_dist = math.inf     # 'infinite' represented by float
    for j in range(K):
        dist = math.dist(elem,centroids[j])
        if dist < min_dist:
            min_dist = dist
            min_dist_index = j
    return min_dist_index

def update_assignment():
    for i in range(N):
        j = find_minimal_dist_index(i)
        assignments[i] = j

def update_centroids():
    for j in range(K):
        j_cluster = [ i for i in range(N) if assignments[i] == j ]  # list of element indexes in j'th cluster
        for l in range(d):
            size = len(j_cluster)
            centroids[j][l] = sum([ elements[index][l] for index in j_cluster ]) / size

for i in range(iter):
    update_assignment()
    update_centroids()
    # TODO: add convergence condition
    # if convergence:
    #     break

for j in range(K):
    print(f"Centroid {j} is {centroids[j]}")