import sys
import numpy as np
import mysymnmf
from sklearn.metrics import silhouette_score


MAX_ITER = 300
EPS = 1e-4


def read_points(file_name):
    points = np.loadtxt(file_name, delimiter=",", dtype=float) #np array, #does it handle spaces?
    return points.tolist() # python list 


def init_h(W, k):
    np.random.seed(1234)
    n = len(W) #num of rows in matrix W
    m = np.mean(W) 
    upper_bound = 2 * np.sqrt(m / k)
    H = np.random.uniform(0, upper_bound, size=(n, k)) #creates matrix n x k s.t each elem in [0, upper_bound]
    return H.tolist() #numpy arrays --> list of lists


def print_matrix(matrix):
    for row in matrix:
        print(",".join(f"{float(value):.4f}" for value in row))

def nmf_labels_from_h(H):
    return np.argmax(np.array(H), axis=1)


def main():
    try:
        if len(sys.argv) != 4:
            raise Exception()

        k = int(sys.argv[1])
        goal = sys.argv[2]
        file_name = sys.argv[3]

        X = read_points(file_name)
        
        if goal == "sym":
            result = mysymnmf.sym(X)

        elif goal == "ddg":
            diag = mysymnmf.ddg(X)
            n = len(diag)
            result = [[0.0] * n for i in range(n)]
            for i in range(n):
                result[i][i] = float(diag[i])
            # TODO: here result is a 1-D array representing the diagonal. need to make it a matrix before printing

        elif goal == "norm":
            result = mysymnmf.norm(X)

        elif goal == "symnmf":
            W = mysymnmf.norm(X)
            H0 = init_h(W, k)
            result = mysymnmf.symnmf(H0, W, len(X),k)

            labels = nmf_labels_from_h(result)
            score = silhouette_score(np.array(X), labels)


        else:
            raise Exception()

        print_matrix(result)
        
    except Exception:
        print("An Error Has Occurred")


if __name__ == "__main__": #if we run this script directly it will run main(), if we import this file it won't run main.
    main()