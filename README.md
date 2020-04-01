# Team13MerkleTrees

### Assignment 1

#### We will be: 

1. Implementing a sequential version of a Merkle Tree. The goal is to implement a straightforward single-threaded version of the container.
2. Use the MRLOCK algorithms to convert the sequential implementation in a multithreaded implementation. One possible strategy to do so is to map each shared variable as a resource. 

- You can find a C++ implementation of MRLOCK, here:
http://cse.eecs.ucf.edu/gitlab/deli/mrlock
- Here is a link to the paper describing MRLOCK and how it works:
https://link.springer.com/chapter/10.1007/978-3-319-03850-6_19

* Do not worry about garbage collection and make sure to pre-allocate all elements that you
might need in your tests.

* Write a main program where you will spawn 4 threads and each will execute a random
mix of operations on your multi-threaded MRLOCK version of the data structure.
* Write a brief report that will include: 
a) a quick summary of the data structure and its
key methods and properties, b) discuss the technical details of your implementation
including the use of MRLOCK and the key algorithms of your implementation, and c)
include a list of references including a citation to the MRLOCK algorithm, your main topic
paper, and other relevant approaches. 

#### The paper we are focusing on: 
* https://people.eecs.berkeley.edu/~kubitron/courses/cs262a-F18/projects/reports/project1_report_ver3.pdf

#### What we read on: 
* https://www.geeksforgeeks.org/introduction-to-merkle-tree/
* https://hackernoon.com/merkle-trees-181cb4bc30b4

#### How to compile: 
##### SequentialMerkleTree.cpp
* g++ SequentialMerkleTree.cpp 
* ./a.out 

##### ConcurrentMerkleTree.cpp
* g++ ConcurrentMerkleTree.cpp mrlockable.cc lockablebase.cc -pthread
* ./a.out 

#### Our Published paper on Research Gate 
* https://www.researchgate.net/publication/340106020_Exploration_of_Concurrent_Merkle_Trees_using_MRLock

### Assignment 2

#### We will be: 

1. The goal is to implement our own version of the algorithms described in the main paper, and to re-implement the data structure with its key multi-threading properties in terms of progress, correctness, efficiency, and thread-safety. 
2. Run performance tests and compare the execution time of this version of the data structure with the MRLOCK-based version from Project Assignment 1. 

#### What we read on: 
* https://medium.com/@kelvinfichter/whats-a-sparse-merkle-tree-acda70aeb837

#### How to compile: 
##### AngelaMerkleTree.cpp
###### On Windows 
* g++ AngelaMerkleTree.cpp -pthread
* ./a.out 
###### On Mac 
* clang++ -std=c++11 AngelaMerkleTree.cpp 
* ./a.out
