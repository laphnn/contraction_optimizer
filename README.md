# Contraction Optimizer
Code to perform operation count minimization for the evaluation of a large number of tensor contractions. A possible application is the efficient evaluation of correlation functions in lattice QCD calculations, and lattice-QCD terminology is used in the following.

## Installation
The only requirement is a modern C++ compiler providing the usual STL containers. The sample `build.sh` file compiles the data structures as well as a sample driver routine, which optimizes the [last example described in the algorithm section](#between-diagram-optimization).

## Benchmarks
The optimization performed here can reduce the computational complexity by an order of magnitude or more.

**Example: Evaluating four nucleon-nucleon correlators**

| Mode     | Ndil^2 | Ndil^3 | Ndil^4 |
| -------- | -----: | -----: | ----:  |
| w/o CSE  | 12,992 | 3,584  | 25,984 |
| w   CSE  | 2,352  |   64   | 1,080  |

**Example: Evaluating an I=3 three-pion correlator**

| Mode     | Ndil^2 | Ndil^3 |
| -------- |------: | -----: |
| w/o CSE  | 47,520 | 60,480 |
| w   CSE  | 450    |    360 |

In those examples the number of computationally dominant contractions with complexity `Ndil^4` have been reduced by factors of 24 and 168 respectively.

## Data Structures
### Graph
A graph is a specification of how indices of a number of tensors are to be contracted to form a result. Tensors are labeled by a zero-based index, and for each tensor pair indices to be contracted are specified as a set of index pairs. 

**Example**: A meson contraction is given by
```
map<pair<uint, uint>, set<pair<uint, uint>>> contrList;
contrList[pair(0,0)] = { {0,0}, {1,1} };
auto aGraph = Graph(contrList);
```
or equivalently, using initializer lists for more a more compact notation,
```
auto aGraph = Graph({ {{0,1}, {{0,0}, {1,1}}} });
```
Internally this information is encoded in bit arrays for performance. Internal loops of tensors are not assumed to be taken care of elsewhere and cannot be encoded in `Graph`.

### Diagram
`Graph` objects store only the topology of contractions. A diagram is a graph together with a list of global tensor indices. This global tensor index is a compound index encompassing hadron type (spatial momentum, irreducible representation [*irrep* for short], irrep row), noise combination, time slice index etc. -- i.e. all characteristics required to uniquely identify a hadron function.

**Example:** A meson correlator for a fixed time separation is given by an average over two noise combinations, each of which is encoded as an individual diagram,
```
auto aGraph = Graph({ {{0,1}, {{0,0}, {1,1}}} });
list<Diagram> diagList;
diagList.push_back(Diagram(aGraph, {0,1});
diagList.push_back(Diagram(aGraph, {2,3});
```

### ContractionOptimizer
Given such a list of diagrams, `ContractionOptimizer` identifies opportunities for a reduction of computational complexity required to evaluate all diagrams by identifying re-usable subexpressions, [c.f. the algorithm section](#algorithm).

A `ContractionOptimizer` is initialized with a large list of `Diagram`s. A subsequent call to `tune()` triggers the optimization routine, and upon completion the following information is accessible:
* `getCompStepList()` -- list of elemental pairwise tensor contractions to be performed in this order to achieve operation count reduction
* `getDiagramList()` -- transformed list of diagrams (of same length as the original diagram list) storing the global result IDs for each diagram
* `getCSECost()` -- cost to perform all required contractions re-using intermediary expressions
* `getNoCSECost()` -- same as above, but without re-use of intermediaries, so that the difference between the two is a measure of the achieved reduction of computational complexity

## Algorithm
Two classes of optimizations are employed in this code, *in-diagram optimization* and *between-diagram optimization*.

### In-Diagram Optimization
In-diagram optimization exploits the fact that for a given complete contraction the number of required operations depends on the order in which contractions are performed, cf. [Hartono et al., Automated Operation Minimization of Tensor Contraction Expressions in Electronic Structure Calculations](https://www.csc.lsu.edu/~gb/TCE/Publications/OpMin-TR0510.pdf).

**Example:** Consider the two-baryon contraction
```
auto aGraph = Graph({ 
		{{0,2}, {{0,0}, {1,1}}},
		{{1,3}, {{1,1}, {2,2}}},
		{{1,2}, {{0,2}}},
		{{0,3}, {{2,0}}},
	});
```

In the first step of evaluating this contraction there are four candidate steps that could be performed. With the index range of each contraction given by `Ndil`, the `{0,2}` and `{1,3}` contractions incur a `Ndil^4` cost (two spectator indices, two index pairs being contracted), while the `{1,2}` and `{0,3}` contractions come at a `Ndil^5` cost (four spectator indices, one index pair being contracted). We follow the greedy algorithm implemented for instance in [Numpy's optimized einsum function](https://github.com/dgasmith/opt_einsum) and suggest the `Ndil^4` contractions, which for this single diagram have equivalent utility.

### Between-Diagram Optimization
The resulting tie is broken by comparing the global utility of a proposed step across all diagrams. Computing the global utility amounts to checking in all diagrams if the proposed step occurs as a subexpression. The code effectively performs *Common-Subexpression Elimination* according to the algorithm described in [Hartono et al., Identifying Cost-Effective Common Subexpressions to Reduce Operation Count in Tensor Contraction Evaluations](https://www.csc.lsu.edu/~gb/TCE/Publications/OpMinCSE-ICCS06.pdf).

**Example:** Consider the above two-baryon contraction with an additional diagram,
```
auto graph1 = Graph({ 
		{{0,2}, {{0,0}, {1,1}}},
		{{1,3}, {{1,1}, {2,2}}},
		{{1,2}, {{0,2}}},
		{{0,3}, {{2,0}}},
	});
auto graph2 = Graph({ 
		{{0,2}, {{0,0}, {1,1}}},
		{{1,3}, {{1,2}, {2,1}}},
		{{1,2}, {{0,2}}},
		{{0,3}, {{2,0}}},
	});
list<Diagram> diagList;
diagList.push_back(Diagram(graph1, {0,1,2,3}));
diagList.push_back(Diagram(graph2, {0,1,2,4}));
```
Note that most of the involved tensors are the same between both diagrams, with only one new tensor in the second diagram because the different index contractions typically require a different noise combination.

The tie between the two locally optimal steps in the first diagram is broken by the fact that the `{0,2}` contraction can be re-used in the second diagram, but the `{1,3}` contraction cannot. This way one contraction of complexity `Ndil^4` has been saved compared to computing both diagrams individually.

## Limitations
* `Graph` does not support internal loops, i.e. reductions on just a single tensor. Those are assumed to be taken care of elsewhere, e.g. tetraquark internal loops are computed elsewhere and the result is a tensor of rank less than four. The `ContractionOptimizer` will never produce internal loops, even though that means missing out on some optimizations at lower orders of `Ndil` -- however the algorithm always yields the optimal path at the dominant order in `Ndil`.
* The index range `Ndil` is assumed to be the same everywhere.
* Due to the way the contractions are stored in bit arrays, the maximum rank of tensors that can be dealt with is `2^3-1=7`. The maximal number of tensors that can occur in one diagram (or graph rather) is `2^4 = 16`.
