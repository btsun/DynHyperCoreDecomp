# General Info
Implementations of the algorithms described in "Fully Dynamic Approximate $k$-Core Decomposition in Hypergraphs" (Bintao Sun, T-H. Hubert Chan and Mauro Sozio; TKDD 2020).

Please feel free to use the codes as you wish.

Available algorithms:
- Incremental.cpp: our insertion-only approximation algorithm.
- FullyDynamic.cpp: our fully dynamic approximation algorithm.
- FullyDynamicThresholdIndexing.cpp: the fully dynamic approximation algorithm using the threshold indexing approach.
- FullyDynamicExactXYPrune.cpp: the fully dynamic exact algorithm generalized from the results of Li et al. ("Efficient Core Maintenance in Large Dynamic Graphs", TKDE 2014).
- IncrementalExactOrderBasedNormal.cpp: the insertion-only exact algorithm of Zhang et al. ("A Fast Order-Based Approach for Core Maintenance", ICDE 2017), which only works for normal graphs.
- FullyDynamicExactOrderBasedNormal.cpp: the fully dynamic algorithm of Zhang et al. ("A Fast Order-Based Approach for Core Maintenance", ICDE 2017), which only works for normal graphs.

An implementation of the hypergraph class is included in Hypergraph.cpp. The hyperedges are unweighted, but parallel hyperedges (i.e., hyperedges with identital set of endpoints) are allowed. In all the above algorithms, even those handles only normal graphs, we use this class to store graphs.

# Usage
To compile and run, follow the instructions in the corresponding source codes.

# Remark
The programs are developed on Windows and the codes that measure memory consumption are not cross-platform.

To exclude the space used to store the graph, you need to build another executable that only reads the graph updates but does not process them. The relevant codes are commented out in the source code. Basically one need to change FullyDynamic::insertEdge/deleteEdge to Hypergraph::insertEdge/deleteEdge in FullyDynamic::run().

# Input Format
The input file that describes (hyper)edge insertions and deletions should contain an update in each line.
- Hyperedge insertion: + [node IDs] timestamp
- Hyperedge deletion: - [node IDs]

So be careful that the last number of a line beginning with "+" is not an endpoint of the inserted hyperedge.

The timestamps are actually useless (for our purpose). You will need them only when you want to trace the insertion time of hyperedges.

When deleting a hyperedge, make sure that exactly the same sequence appeared in an insertion update before.
Therefore, it is recommended that in each line, the node IDs are sorted in some particular order, e.g., increasing order.

An input example, smallSampleInput.txt, is provided.

# Output Format
The formats of output files are described below. Some relevant codes that outputs specific information are commented out in the source code files (beginning with "Block:"); uncomment them to use them. Not all output files are used for report in the paper.

- [StatIncrementalCoreValue.txt]
Each block begins with a milestone indicating the number of updates processed.
Then pairs (a, b) follow, meaning that b nodes have core value a. A "-1" terminates this.
After that, there are also pairs (a, b) in the same form, indicating that b nodes have estimated core value a.
Finally, a "-1" terminates this and a new block (if exist) comes.

- [StatIncrementalDetail.txt]
Let k be the number of milestones.
Totally 2 * k lines.
Odd line: maximum ratio; even line: average ratio.
The t-th number on the i-th row of maximum ratio is the maximum ratio of b[t][u] at milestone i.
The same for average ratios.

- [StatIncrementalTime.txt]
Time spent to deal with the updates between two milestones (there are 100,000 updates between two milestones).

- [StatFullyDynamicCoreValue.txt & StatFullyDynamicDetail.txt & StatFullyDynamicTime.txt]
Same as the files for the incremental algorithm.

- [StatFullyDynamicExactTime.txt & StatFullyDynamicExactOrderBasedTime.txt]
The format is the same as StatFullyDynamicTime.txt, but they are for different algorithms.

- [StatHyperAndNormalCoreValue.txt]
The k-core decomposition results under the hypergraph model and the normal graph model, after all edge updates are processed.
Each line represents one node and contains 4 numbers: the node's ID, its exact hypergraph core value, its approximate hypergraph core value and its exact normal graph core value.

- [LargeTauIsUnnecessaryIncremental.txt]
tau; and then maxErr[t], avgErr[t], for t = 1 to tau.

# Contributor
Bintao Sun (btsun@connect.hku.hk)
