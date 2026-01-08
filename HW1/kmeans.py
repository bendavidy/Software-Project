import sys
import math

default_iter = 400
eps = 0.001


# -------------- helper functions --------------

def terminate(msg):
    print(msg)
    sys.exit(1) # terminate the program

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
            if (size != 0):
                centroids[j][l] = sum([ elements[index][l] for index in j_cluster ]) / size
                #new centroid
                
def convert_to_float(a : list):
    for i in range(len(a)):
        a[i] = float(a[i])
    return a


# -------------- input validations --------------

if (len(sys.argv) not in (2,3)): # inputs like python3 something < input.txt or python3 something1 something2 something3 < input.txt will get error 
    terminate("An Error Has Occurred") # sys.argv = [elem,elem,elem] or [elem] ---> Error

try: #validate k
    K = int(sys.argv[1])
except Exception:
    terminate("Incorrect number of clusters!")

if (len(sys.argv) == 3): #if the len is 3 e.g sys.argv = [elem,elem,elem] --> we need to check that iter is valid ()
    try:
        iter = int(sys.argv[2])
    except Exception:
        terminate("Incorrect maximum iteration!")
else:   #else the len must be 2 --> iter = '' ---> we take default value 400
    iter = default_iter

if not (1 < iter < 800):
    terminate("Incorrect maximum iteration!")


input_data = [line for line in sys.stdin if line.strip()]    # read input from stdio , line.strip() will make empty lines = "" then the if statement will not include them in the list

if(len(input_data) == 0):
    terminate("An Error Has Occurred")

d = len(input_data[0].split(','))
N = len(input_data)

if not (1 < K < N):
    terminate("Incorrect number of clusters!")


# -------------- lists declerations --------------

# convert to a list of lists, elements[i][j] is the j'th coordinate of the i'th element
elements = [ convert_to_float(input_data[i].split(',')) for i in range(N) ]

# assignments[i] is the index of centroids we bond the i'th element to.
# in other words:   i'th element is assigned to j'th cluster IFF assignments[i]=j where clusters[j] is the j'th cluster
assignments = [ 0 for i in range(N) ]

# initialize centroids:
centroids = [elements[i][:] for i in range(K)]   # len(centroids) = K



# -------------- main loop --------------
for i in range(iter):

    old_centroids = [c[:] for c in centroids]
    update_assignment()
    update_centroids()
    
    max_shift = float("-inf")

    for j in range(K):
        shift = math.dist(old_centroids[j],centroids[j])
        if shift > max_shift:
            max_shift = shift

    if max_shift < eps:
        break

for j in range(K):
    print(",".join(f"{x:.4f}" for x in centroids[j]))




