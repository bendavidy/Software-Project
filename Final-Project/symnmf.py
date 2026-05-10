import sys
import numpy as np
import mysymnmf
from sklearn.metrics import silhouette_score


MAX_ITER = 300
EPS = 1e-4


def read_points(file_name):
    '''
    Reads points from the input files into a Python list
    '''
    points = np.loadtxt(file_name, delimiter=",", dtype=float) # np array
    return points.tolist() # python list 


def init_h(W, k):
    '''
    Initializes H_0 from a "random" uniform distribution
    '''
    np.random.seed(1234)
    n = len(W) # num of rows in matrix W
    m = np.mean(W) 
    upper_bound = 2 * np.sqrt(m / k)
    H = np.random.uniform(0, upper_bound, size=(n, k)) # creates matrix n x k s.t each elem in [0, upper_bound]
    return H.tolist() # numpy arrays --> list of lists


def print_matrix(matrix):
    '''
    Prints the matrix onscreen, formatted to 4 digits after the floating point
    '''
    for row in matrix:
        print(",".join(f"{float(value):.4f}" for value in row))

def nmf_labels_from_h(H):
    '''
    Translates the H association matrix into a label array A such that the i'th data point belongs to the A[i]'th cluster
    '''
    return np.argmax(np.array(H), axis=1)


def main():
    try:
        if len(sys.argv) != 4:
            raise Exception()

        k = int(sys.argv[1])
        goal = sys.argv[2]
        file_name = sys.argv[3]

        X = read_points(file_name)
        
        if k < 2 or len(X) <= k:
            raise Exception()
        
        if goal == "sym":
            result = mysymnmf.sym(X)

        elif goal == "ddg":
            diag = mysymnmf.ddg(X)
            n = len(diag)
            result = [[0.0] * n for i in range(n)]
            for i in range(n):
                result[i][i] = float(diag[i])

        elif goal == "norm":
            result = mysymnmf.norm(X)
            if not result:
                raise Exception

        elif goal == "symnmf":
            W = mysymnmf.norm(X)
            if not W:
                raise Exception
            H0 = init_h(W, k)
            result = mysymnmf.symnmf(H0, W, len(X),k)

            labels = nmf_labels_from_h(result)
            score = silhouette_score(np.array(X), labels)

        else:
            raise Exception()

        print_matrix(result)
        
    except Exception:
        print("An Error Has Occurred")


if __name__ == "__main__":
    main()