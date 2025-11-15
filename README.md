# MPI-K-Means-Clustering
K-Means Clustering is an unsupervised algorithm that partitions a dataset of m points in ℝⁿ
into K clusters by iteratively assigning each point to the closest centroid and recomputing
centroids as the mean of their assigned points. The goal is to minimize the total within-cluster
variance (MSE/TWCV).


---

## Project structure

- `DataSet/` – dataset files  
- `Serial/` – serial C++ implementation  
- `Parallel/` – parallel C++ implementation using MPI  
- `Utility/` – toy dataset generators (Java)  
- `Report K-means Clustering.pdf` – theoretical and experimental report  

---

## Build & run

### Serial program


**Compile**


```
cd Serial
g++ -O3 -std=c++11 *.cpp -o K-means_sequential
```

(adjust the list of .cpp files if needed)

**Run**

```
Usage: kmeans_serial [OPTIONS]

  Run the K-Means clustering algorithm on a local dataset in serial mode.

Options:
  -d, --dataset PATH        Path to the dataset file to process (CSV-like
                            format, one point per line).  [required]

  -m, --max-iter INTEGER    Maximum number of iterations allowed.
                            [default: 5; 1<=x<=10000]
```


Example of usage:

```
cd Serial
./K-means_sequential --dataset Dataset.txt --max-iter 5
```

The program:

- reads the dataset from given path,
- runs K-Means for at most max-iter iterations (or until convergence),
- prints basic timing information to stdout.

---

### Parallel program (MPI)

**Compile**


```
cd Parallel
mpicxx -O3 -std=c++11 *.cpp -lboost_serialization -o main
```

(adapt the command depending on your MPI / Boost installation)

**Hostfile**

To run on multiple nodes you need an MPI hostfile, e.g.:

```
10.128.0.4 slots=2
10.128.0.5 slots=2
```

**Run**

```
Usage: mpirun [MPI OPTIONS] ./main [OPTIONS]

  Run the parallel MPI-based K-Means clustering algorithm.

MPI options:
  -np INTEGER               Number of MPI processes to launch.  [required]
  --hostfile PATH           Optional hostfile for multi-node execution.
  --mca ...                 Additional MPI runtime parameters.

Program options:
  -d, --dataset PATH        Path to dataset file to process (CSV-like format,
                            one point per line).  [required]

  -m, --max-iter INTEGER    Maximum number of iterations allowed.
                            [default: 5; 1<=x<=10000]
```


Example with 8 processes:

```
cd Parallel
mpirun --hostfile hostfile -np 8 ./main --dataset Dataset.txt --max-iter 5
```

The program:

- reads the dataset from given path,
- runs K-Means for at most max-iter iterations (or until convergence),
- prints basic timing information to stdout.


Make sure that -np does not exceed the total number of slots declared in the hostfile.

The master process (rank 0) handles dataset loading, centroid updates and termination checks; worker processes perform distance computations and partial reductions.


## Requirements

To compile and run the programs you need:

- A C++ compiler (e.g. `g++`) with C++11 or later
- [OpenMPI](https://www.open-mpi.org/) or an equivalent MPI implementation
- [Boost](https://www.boost.org/) (`serialization` and `iostreams` libraries) for the parallel program
- Java JDK 8+ for the dataset generator

---

## Dataset

The project works on a text-based dataset where each line represents a point,
and the columns (comma-separated) represent the features.

You can use the `Utility/DataSetGenerator.java` program to generate a toy
dataset.

### Dataset generation

Inside `DataSetGenerator.java` (or an equivalent file) you will find:

```
java
int M = 1500;  // number of rows (points)
int N = 10;    // number of columns (point dimension)
FileWriter fileWriter = new FileWriter("src/DataSet" + M + "x" + N + ".txt");
```

You can change M and N to generate datasets of different sizes and then run the generator to produce a file named, for example, DataSet1500x10.txt.

---

### Parallelization overview

The dataset X ∈ ℝ^{m×n} is loaded by the master and partitioned into P blocks, one per MPI
process. Each worker:
- receives its block of points and the current centroids,
- computes distances and assigns points to clusters,
- accumulates partial sums and counts per cluster and a local MSE.

The master then reduces these partial results (MPI_Reduce), updates global centroids and MSE,
checks the stopping criterion, and broadcasts updated centroids to all workers (MPI_Bcast).

### Theoretical performance (sketch)

Under a simple FLOP-counting model (ignoring communication), the parallel execution time
splits into a large parallel part plus a smaller serial part on the master. For large datasets
(m ≫ 1) the ideal speedup is approximately linear in the number of processes P, while the
scaleup tends to 1 when the problem size grows proportionally with P.
See `Report K-means Clustering.pdf` for full derivations.

### Experimental setup (Google Cloud)

- Dataset: 10 000 points, 10 features (m = 10 000, n = 10)
- Iterations: 5 (fixed)
- Cluster configurations:
  - Fat, Infra-Regional: 2× (16 cores, 64 GB RAM)
  - Fat, Intra-Regional: 3× (8 cores, 32 GB RAM)
  - Light, Infra-Regional: 16× (2 cores, 4 GB RAM)
  - Light, Intra-Regional: 12× (2 cores, 4 GB RAM)

**In short:** speedup grows almost linearly but remains below the ideal P; efficiency decreases
as P increases; scaleup drops faster than linearly when both problem size and P grow.
Detailed plots are in the PDF.

### Possible improvements

- Load very large datasets in chunks to avoid running out of RAM.
- Better balance the “tail” so the master does not process nearly twice the work of a worker.
- Avoid repeatedly sending invariant metadata (e.g., buffer size) in MPI messages.

