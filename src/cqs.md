# Finding Optimal Cyclic Quorum Systems by Exhaustive Search and Deep Reinforcement Learning

## 1. Introduction

In the realm of distributed computing and related fields, ensuring coordination, consistency, and efficient resource utilization among multiple independent entities is paramount. Quorum systems have emerged as a fundamental concept for achieving such goals, particularly in scenarios requiring mutual exclusion or the reliable processing of distributed data. A quorum system is essentially a collection of subsets of participating entities (often referred to as sites or processes), where any two subsets (quorums) must have at least one entity in common. This non-empty intersection property is crucial for guaranteeing that any two operations requiring a quorum will interact at some common point, thereby allowing for conflict detection and resolution.

While the basic quorum concept provides a framework for coordination, designing efficient quorum systems that are fair and scalable presents significant challenges. Ideally, a quorum system should possess properties like equal work (each quorum is of the same size) and equal responsibility (each entity is included in the same number of quorums). Achieving these properties, especially while minimizing quorum size to reduce communication costs, is a key area of research. The theoretical lower bound for the size of quorums in a system with $N$ or $P$ sites, satisfying certain symmetric properties, is approximately $O(\sqrt{N})$ or $O(\sqrt{P})$. Finding systems that meet this lower bound is often equivalent to constructing finite projective planes, which do not exist for all system sizes, making the search for optimal quorums a complex problem.

**Cyclic Quorum Systems (CQS)** represent a structured approach to constructing quorum systems that often achieve or come very close to these desirable properties. CQS are based on concepts from combinatorial theory, particularly cyclic block designs and cyclic difference sets. A defining characteristic of CQS is that the entire set of quorums can be generated cyclically from a single base quorum. This inherent structure simplifies the description and management of the quorum system.

The utility of Cyclic Quorum Systems extends beyond their original application in distributed mutual exclusion. Recent research has demonstrated their effectiveness in managing complex distributed computations and optimizing resource usage in data-intensive tasks. This includes their application in:
*   **Distributed All-Pairs Algorithms:** CQS have been successfully applied to problems requiring computations between all possible pairs of data elements in a large dataset, offering significant improvements in memory efficiency and workload distribution.
*   **Wireless Sensor Networks (WSNs):** A variant called CQS-Pair has been developed for heterogeneous wakeup scheduling in WSNs, enabling nodes with different power-saving requirements to maintain connectivity.
*   **Attention Computation in Deep Learning:** CQS-Attention is a novel sequence parallelism scheme leveraging CQS theory to scale the standard self-attention computation for very long sequences in transformer models, addressing the significant memory bottleneck.

This paper provides a survey of Cyclic Quorum Systems, drawing upon the provided source material to explore their theoretical foundations, properties, and applications in these diverse domains. We will delve into the combinatorial concepts underlying CQS construction, examine how they enable efficient distributed algorithms, discuss their role in heterogeneous WSNs, and highlight their recent use in scaling attention mechanisms for long sequences.
(???)


## 2. Preliminaries

### Quorum Systems

A quorum system provides a decentralized method for ensuring coordination and consistency in a distributed environment without requiring global knowledge or centralized control. Let $U = \{P_1, P_2, \dots, P_N\}$ be the set of $N$ sites (or processes) in a distributed system.

**Definition 1. Quorum System**
A quorum system $\mathcal{Q}$ over $U$ is a collection of non-empty subsets of $U$, called quorums, which satisfies the **intersection property**: $\forall G, H \in \mathcal{Q} : G \cap H \neq \emptyset$.

This property is fundamental as it guarantees that any two operations requiring permission from a quorum will consult at least one common site, which can then be used to mediate conflicts or ensure consistency.

For distributed algorithms like mutual exclusion, certain additional properties are often desirable for fairness and efficiency:
*   **A1. Site Inclusion:** Each site $P_i$ is contained in its own quorum $S_i$. This property might be implicitly assumed or explicitly defined depending on the specific application.
*   **A2. Non-empty Intersection:** As stated above, any two quorums must intersect. This is the defining property of a quorum system.
*   **A3. Equal Work:** All quorums in the system have the same size. If $S_i$ is the quorum for site $P_i$, then $|S_i| = d$ for all $i \in \{1, 2, \dots, N\}$, where $d$ is an integer less than $N$. The size of a quorum is often denoted by $d$ or $k$.
*   **A4. Equal Responsibility:** Each site is contained in the same number of quorums. Specifically, for all $P_i \in U$, $P_i$ is contained in $d$ quorums $S_j$.

A set of quorums satisfying properties A3 and A4 is called **symmetric**. The goal in designing efficient quorum systems for applications like distributed mutual exclusion is to minimize the quorum size $d$ while maintaining these properties.

Maekawa showed that for a fixed quorum size $d$, the maximum possible number of sites $N$ for which a symmetric quorum system exists is $d(d-1) + 1$, assuming any two quorums intersect at exactly one site. This implies a theoretical lower bound on the quorum size:
$d \geq \sqrt{N}$. More precisely, the lower bound is $\lceil \sqrt{N} \rceil$. When considering a system with $P$ processes or nodes, the lower bound is similarly $O(\sqrt{P})$.

Finding quorum systems that satisfy these properties and approach the minimal size is challenging. The problem of finding such systems for $N = d(d-1) + 1$ is equivalent to finding finite projective planes of order $d-1$, and the existence of such planes is not guaranteed for all orders (e.g., order 10). Exhaustive search methods to find optimal quorum systems are computationally intractable for large $N$. This difficulty has motivated the development of systematic construction methods, such as those based on grid structures or combinatorial designs like difference sets.

Grid-based quorum systems, for instance, logically organize sites in a grid (e.g., square or triangle). In a square grid system with $N$ sites ($N$ being a perfect square), a quorum for a site includes all sites in the same row and column, resulting in a quorum size of $2\sqrt{N} - 1$. While simple and geometrically evident, this is roughly twice the theoretical lower bound and often leads to pairs of quorums intersecting at two sites. Variations like "triangle" grid systems aim to reduce the quorum size, but may struggle with maintaining the equal responsibility property.

The challenges associated with finding truly optimal, symmetric quorum systems for arbitrary numbers of sites highlight the value of structured approaches that leverage combinatorial properties, such as the Cyclic Quorum Systems discussed next.

### Cyclic Quorum Systems (CQS)

Cyclic Quorum Systems (CQS) provide a systematic way to construct symmetric quorum systems based on the principles of cyclic block design and cyclic difference sets. The defining feature of a CQS is that the entire collection of quorums can be generated by cyclically shifting the elements of a single base quorum.

Let $U = \{0, 1, \dots, N-1\}$ be the set of $N$ sites, viewed as elements in the additive group $\mathbb{Z}_N$.

**Definition 2. Cyclic Quorum System**
A group of cyclic quorums is a set of $N$ quorums $\{B_0, B_1, \dots, B_{N-1}\}$ where each quorum $B_i$ is derived from a base quorum $B_0 = \{a_1, a_2, \dots, a_k\} \subseteq \{0, 1, \dots, N-1\}$ by adding $i$ modulo $N$ to each element: $B_i = \{a_1 + i, a_2 + i, \dots, a_k + i\} \pmod N$.

For this collection $\{B_0, \dots, B_{N-1}\}$ to be a valid quorum system, it must satisfy the non-empty intersection property: $B_i \cap B_j \neq \emptyset$ for all $i, j \in \{0, 1, \dots, N-1\}$. Furthermore, the cyclic construction automatically ensures the equal work ($|B_i| = d$ for all $i$) and equal responsibility (each element appears in $d$ quorums) properties.

The existence and properties of cyclic quorum systems are deeply connected to the theory of **difference sets**.

**Definition 3. Cyclic $(N, d, \lambda)$-Difference Set**
A set $M = \{a_1, a_2, \dots, a_d\} \subseteq \{0, 1, \dots, N-1\}$ is called a cyclic $(N, d, \lambda)$-difference set if for every $m \not\equiv 0 \pmod N$, there are exactly $\lambda$ ordered pairs $(a_i, a_j)$ with $a_i, a_j \in M$ such that $a_i - a_j \equiv m \pmod N$.

A related concept, particularly relevant for CQS, is the relaxed difference set.

**Definition 4. Relaxed Cyclic $(N, d)$-Difference Set**
A set $M = \{a_1, a_2, \dots, a_d\} \subseteq \{0, 1, \dots, N-1\}$ is called a relaxed (cyclic) $(N, d)$-difference set if for every $m \not\equiv 0 \pmod N$, there exists at least one ordered pair $(a_i, a_j)$ with $a_i, a_j \in M$ such that $a_i - a_j \equiv m \pmod N$.

The connection between cyclic quorum systems and relaxed difference sets is formalized in Theorem 2 and Theorem 3 of (Theorem 2 and Theorem 3 in and Definition 2 and Theorem 3 in). These theorems establish that a set $A = \{a_0, \dots, a_d\} \pmod P$ is a relaxed $(P, d)$-difference set if and only if the cyclic quorum set defined by $S_i = \{a_0 + i, \dots, a_d + i\} \pmod P$ satisfies the non-empty intersection property. The intuition relies on the fact that the intersection property $S_i \cap S_j \neq \emptyset$ for all $i, j$ is equivalent to requiring that for any difference $m = j - i \pmod P$, there exist elements $a_u, a_v \in A$ such that $a_u + i \equiv a_v + j \pmod P$, which simplifies to $a_u - a_v \equiv j - i \pmod P \equiv m \pmod P$. This means every possible non-zero difference modulo $N$ must be formed by at least one pair of elements in the base quorum's set.

Finding a base quorum $B_0$ that forms a cyclic quorum system with the minimum size $d$ is equivalent to finding a relaxed $(N, d)$-difference set with minimum $d$. For cases where $N = d^2 - d + 1$ and $d-1$ is a prime power, the existence of cyclic $(N, d, 1)$-difference sets (known as Singer difference sets) is guaranteed, and these sets form cyclic quorum systems where any two quorums intersect at exactly one site. Singer difference sets lead to particularly efficient CQS with no redundancy in pair coverage (discussed later). For other values of $N$, finding the optimal base quorum often requires exhaustive search, although methods based on the Multiplier Theorem can speed up the construction of certain types of difference sets. The Multiplier Theorem provides conditions under which a prime $p$ is a "multiplier" of a difference set, meaning multiplication by $p$ permutes the elements of the difference set, aiding in their discovery.

Examples of cyclic quorum systems and difference sets include:
*   For $N=7$, the set $\{1, 2, 4\} \pmod 7$ is a cyclic $(7, 3, 1)$-difference set. This is a Singer difference set ($7 = 3^2 - 3 + 1$, $3-1=2$ is prime power). The cyclic quorums are $\{1, 2, 4\}$, $\{2, 3, 5\}$, $\{3, 4, 6\}$, $\{4, 5, 0\}$, $\{5, 6, 1\}$, $\{6, 0, 2\}$, $\{0, 1, 3\}$.
*   For $N=8$, the set $\{0, 1, 2, 4\} \pmod 8$ is a base quorum. The quorums are $B_0 = \{0, 1, 2, 4\}$, $B_1 = \{1, 2, 3, 5\}$, $B_2 = \{2, 3, 4, 6\}$, $B_3 = \{3, 4, 5, 7\}$, $B_4 = \{4, 5, 6, 0\}$, $B_5 = \{5, 6, 7, 1\}$, $B_6 = \{6, 7, 0, 2\}$, $B_7 = \{7, 0, 1, 3\}$.
*   Table 1 and Table 2 in list base quorums (denoted $B_1$ using 1-based indexing for sites, $B_1=\{a_1, \dots, a_d\}$) for optimal cyclic quorum schemes for $N$ from 4 to 111, found through exhaustive search. For example, for $N=4$, the base quorum is $\{1, 2, 3\}$. For $N=10$, the base quorum is $\{1, 2, 3, 6\}$.

The existence of cyclic quorum systems with sizes close to the theoretical lower bound for arbitrary $N$, coupled with their symmetric properties, makes them attractive for managing distributed resources and computations.

**Example of a Cyclic Quorum System Construction**

Let's construct a cyclic quorum system for $N=8$ using the base quorum $B_0 = \{0, 1, 2, 4\} \pmod 8$ ccccsds. The size of the quorum is $d=4$.
The full set of 8 quorums is generated by adding $i \in \{0, 1, \dots, 7\}$ modulo 8 to the elements of $B_0$:
*   $B_0 = \{0, 1, 2, 4\} \pmod 8$
*   $B_1 = \{0+1, 1+1, 2+1, 4+1\} \pmod 8 = \{1, 2, 3, 5\} \pmod 8$
*   $B_2 = \{0+2, 1+2, 2+2, 4+2\} \pmod 8 = \{2, 3, 4, 6\} \pmod 8$
*   $B_3 = \{0+3, 1+3, 2+3, 4+3\} \pmod 8 = \{3, 4, 5, 7\} \pmod 8$
*   $B_4 = \{0+4, 1+4, 2+4, 4+4\} \pmod 8 = \{4, 5, 6, 0\} \pmod 8$
*   $B_5 = \{0+5, 1+5, 2+5, 4+5\} \pmod 8 = \{5, 6, 7, 1\} \pmod 8$
*   $B_6 = \{0+6, 1+6, 2+6, 4+6\} \pmod 8 = \{6, 7, 0, 2\} \pmod 8$
*   $B_7 = \{0+7, 1+7, 2+7, 4+7\} \pmod 8 = \{7, 0, 1, 3\} \pmod 8$

This set of 8 quorums forms a cyclic quorum system for $N=8$. Each quorum has size 4 (equal work). To verify equal responsibility, we can count how many quorums each site appears in. For example, site 0 appears in $B_0, B_4, B_6, B_7$. It appears in 4 quorums, which is equal to the quorum size $d=4$. This holds for all sites due to the cyclic construction. The intersection property can be verified by checking any two quorums. For example, $B_0 \cap B_1 = \{1, 2\} \neq \emptyset$.

The challenge remains in finding the base quorum $\{a_1, \dots, a_d\}$ for a given $N$ such that the resulting cyclic sets satisfy the intersection property and $d$ is minimized.

### CQS in Distributed Mutual Exclusion

The original context where cyclic quorum systems were formally introduced and studied was distributed mutual exclusion. In distributed mutual exclusion, sites in a network need to coordinate access to a shared resource (a critical section) such that only one site can access it at a time. Maekawa's algorithm was one of the first quorum-based algorithms that aimed to achieve symmetry (equal work and equal responsibility) and minimize the quorum size, pointing out the connection to finite projective planes. However, as discussed, finding optimal symmetric quorums for arbitrary numbers of sites proved difficult.

Luk and Wong proposed using cyclic difference sets as a basis for constructing quorum systems for distributed mutual exclusion. Their "cyclic" quorum scheme (which forms the basis for the CQS concept discussed here) constructs symmetric quorum sets for arbitrary $N$. The resulting quorum size is shown to be very close to the theoretical lower bound $\sqrt{N}$. The main challenge in this scheme is finding the optimal base quorum (relaxed difference set) through search, although the search space is smaller compared to general exhaustive searches for arbitrary quorum systems.

### Conclusion and Future Work

Cyclic Quorum Systems (CQS), rooted in the theory of combinatorial designs and difference sets, offer a structured and efficient approach to designing quorum systems with desirable properties such as non-empty intersection, equal work, and equal responsibility. Their cyclic construction from a base quorum simplifies their definition and management.

The applications of CQS span diverse areas of distributed systems and computation:
*   In **distributed mutual exclusion**, CQS provide symmetric quorum sets with sizes close to the theoretical lower bound for arbitrary numbers of sites.
*   For **distributed all-pairs algorithms**, CQS possess a unique "all-pairs property" that allows for minimal data replication per processing node ($O(N/\sqrt{P})$ memory footprint) and enables load-balanced computation. Techniques like computation management logic can further improve efficiency by eliminating redundant work without communication overhead.
*   In **wireless sensor networks**, the CQS-Pair concept extends the utility of CQS to support heterogeneous asynchronous wakeup scheduling, allowing nodes with different power-saving needs to maintain connectivity.
*   Most recently, **CQS-Attention** demonstrates the power of CQS in scaling the standard self-attention computation for infinitely long sequences in transformer models, providing a memory-efficient ($O(1/W)$ memory ratio per worker) and embarrassingly parallel solution.

The core strengths of CQS lie in their elegant mathematical structure, which translates into practical advantages such as simplified management, robustness, compatibility with other optimizations, and flexibility. While challenges remain, such as efficiently finding optimal base quorums for arbitrary system sizes, and addressing potential bottlenecks in centralized architectures, the research presented in this paper highlights the significant potential of CQS.

## Generating Fixed-Density Necklaces and Bracelets Efficiently

The generation of discrete combinatorial objects is of immense importance in both mathematics and computer science, finding wide-ranging applications in fields such as computational biology, combinatorial chemistry, operations research, and data mining. Exhaustively listing these objects allows for their study and utilization in various practical scenarios, such as the calibration of colour printers, which specifically uses exhaustive lists of bracelets. To be truly useful in applications, algorithms for generating such objects must be highly efficient, ideally running in Constant Amortized Time (CAT). A CAT algorithm is one where the total number of basic operations performed is proportional to the number of objects generated, meaning that, on average, each successive object is generated in constant time. This efficiency is extremely desirable in generation algorithms.

Among the fundamental combinatorial objects are strings and their equivalence classes under certain operations. Two key types are necklaces and bracelets, which can be defined over an alphabet of size $k$ (k-ary strings).

### Combinatorial Objects: Necklaces, Bracelets, and Fixed Density

A **k-ary necklace** is formally defined as a lexicographically minimal k-ary string that is equivalent under string rotation. This means that a string $a_1a_2 \cdots a_n$ is considered equivalent to any of its rotations, such as $a_ia_{i+1} \cdots a_n a_1 \cdots a_{i-1}$ for $1 < i \le n$. The necklace representation of an equivalence class is the lexicographically smallest string within that class. The set of all k-ary necklaces of length $n$ is denoted $N_k(n)$, and its cardinality is $N_k(n)$. For example, for $k=2$ (binary strings) and $n=4$, the set of necklaces $N_2(4)$ includes {0000, 0001, 0011, 0101, 0111, 1111}. The rotations of 0011 are {0011, 0110, 1100, 1001}, and 0011 is the lexicographically smallest among them, hence it's the necklace.

A related concept is that of **Lyndon words**, which are aperiodic necklaces. An aperiodic necklace is a necklace that is its own shortest Lyndon prefix. The set of k-ary Lyndon words of length $n$ is denoted $L_k(n)$, with cardinality $L_k(n)$. For instance, $L_2(4) = \{0001, 0011, 0111\}$.

A **prenecklace** is a prefix of some necklace. The set of all k-ary prenecklaces of length $n$ is denoted $P_k(n)$, with cardinality $P_k(n)$. For example, $P_2(4) = N_2(4) \cup \{0010, 0110\}$.

**Bracelets** are a variation of necklaces that are symmetric under both rotation and reversal. A k-ary bracelet is the lexicographically minimal string equivalent under these two operations. $B_k(n)$ represents the set of length $n$ bracelets, and $B_k(n)$ its cardinality. For a binary example ($k=2$), 001100 is a bracelet because its rotations (e.g., 011000) and reversals (e.g., 001100 reversed is 001100) are considered, and 001100 is the smallest among them.

A critical property for restricted classes of these objects is **fixed density**. A k-ary string is said to be of fixed density if the number of occurrences of symbol 0 is fixed. Let us define density, denoted by $d$, as the number of non-zero symbols. Therefore, a string of length $n$ with density $d$ has $d$ non-zero symbols and $n-d$ zero symbols. The notation for sets of objects with fixed density adds the parameter $d$:
*   $N_k(n, d)$: the set of k-ary necklaces having length $n$ and density $d$.
*   $P_k(n, d)$: the set of k-ary prenecklaces having length $n$ and density $d$.
*   $B_k(n, d)$: the set of k-ary bracelets having length $n$ and density $d$.
Their cardinalities are denoted $N_k(n, d)$, $P_k(n, d)$, and $B_k(n, d)$, respectively.

Counting the number of objects with specific symbol occurrences is possible using formulas. For necklaces with $n_i$ occurrences of symbol $i$, where $0 \le i < k$, the number $N_k(n_0, n_1, \ldots, n_{k-1})$ is given by:
$$N_k(n_0, n_1, \ldots, n_{k-1}) = \frac{1}{n} \sum_{j|\gcd(n_0,n_1,\ldots,nk−1)} \varphi(j) \frac{(n/j)!}{(n_0/j)! \cdots (n_{k-1}/j)!}$$
where $\varphi(j)$ is Euler's totient function. The number of necklaces with fixed density $d = n_1 + \cdots + n_{k-1}$ (and thus $n_0 = n-d$) is obtained by summing over all combinations of $n_1, \ldots, n_{k-1}$ that sum to $d$:
$$N_k(n, d) = \sum_{n_1+\cdots+nk−1=d} N_k(n - d, n_1, \ldots, n_{k-1})$$
For binary strings ($k=2$), these formulas simplify. The cardinality of bracelets is related to that of necklaces; specifically, $N_k(n) \le 2B_k(n)$ and $N_k(n, d) \le 2B_k(n, d)$, as there are at most two necklaces in each equivalence class of a bracelet.

### Efficient Generation Algorithms: The CAT Goal

The primary performance goal for listing combinatorial objects is achieving a CAT algorithm. This means the total computation time should be linear in the number of objects produced.

Various schemes have been developed for generating combinatorial structures like necklaces, Lyndon words, and their variants. Recursive frameworks, such as the one by Cattell et al., are often used as a basis. Algorithms exist for generating necklaces with fixed density and content. Similarly, algorithms for generating bracelets exist, including a linear algorithm by Lisonek and a CAT algorithm by Sawada. However, prior to the work presented in one source, no significant work had been done to list restricted classes of bracelets like those with fixed density. The goal of the work described in one paper is to present an algorithm for lexicographic listing of bracelets with fixed density that works for arbitrary alphabet size and generates each successive bracelet in constant amortized time.

### Generating Fixed-Density Necklaces (A Foundation)

The problem of generating fixed-density necklaces efficiently was addressed by Ruskey and Sawada. Their work provides an efficient, simple, recursive algorithm that is optimal in the sense that it runs in time proportional to the number of necklaces produced (i.e., it is CAT). Previous algorithms for this problem were not CAT.

Their approach involves modifying an existing recursive necklace generation algorithm, `Gen(t, p)`, which generates all length $n$ prenecklaces. This algorithm, based on the **Fundamental Theorem of Necklaces** (Theorem 2.1 in and), uses recursion to append characters to a prenecklace while maintaining the prenecklace property. The theorem states that for a prenecklace $\alpha = a_1a_2 \cdots a_{n-1} \in P_k(n-1)$ with $p = \text{lyn}(\alpha)$ (the length of its longest Lyndon prefix), the string $\alpha b \in P_k(n)$ if and only if $a_{n-p} \le b \le k-1$. Furthermore, $\text{lyn}(\alpha b)$ is either $p$ (if $a_{n-p} = b$) or $n$ (if $a_{n-p} < b$). $\alpha b$ is a necklace if $n \bmod \text{lyn}(\alpha b) = 0$.

The base `Gen(t, p)` algorithm generates prenecklaces in lexicographic order and can be modified to output necklaces or Lyndon words by checking the condition on $p$ in the `PrintIt(p)` function.

To generate fixed-density necklaces efficiently, Ruskey and Sawada developed a modified algorithm, `Gen2(t, p)`. This modification focuses on generating prenecklaces whose last character is non-zero, effectively incrementing the density rather than just the length in each recursive step. It uses an array `a` to store the positions of non-zero characters and an array `b` for their values. The parameter `t` represents the current density, and `at` is the length of the current prenecklace. Valid positions and values for the next non-zero character are determined to maintain the prenecklace property and lexicographic order.

Building on `Gen2`, the fixed-density necklace algorithm `GenFix(t, p)` incorporates specific optimizations for the fixed-density constraint. These optimizations include:
1.  **Restricting the position of the first non-zero character:** It must be between $n-d+1$ and $(n-1)/d+1$ inclusive.
2.  **Restricting the position of the $i$-th non-zero character:** It must be at or before the $(n-d+i)$-th position.
3.  **Stopping recursion early:** Stop generating when $d-1$ non-zero characters have been placed. The last non-zero symbol (the $d$-th one) is placed at the $n$-th position during the printing step.
4.  **Determining valid values for the $n$-th position:** This is handled in the `PrintIt(p)` function with an additional constant-time test.

The CAT nature of `GenFix` is proven by bounding the size of its computation tree, which consists of prenecklaces ending in a non-zero character with densities from 1 to $d-1$. The size of this tree, CompTree$_k(n, d)$, is bounded by a sum involving the number of such prenecklaces, denoted $P'_k(j, i)$ for length $j$ and density $i$. Lemma 4.1 shows that $P'_k(n, d) \le N_k(n, d) + L_k(n, d)$. Lemmas 4.3 and 4.4 provide bounds relating $N_k(n, d)$ and $L_k(n, d)$ to combinatorial terms and each other, such as $N_k(n, d) \le 2L_k(n, d)$ and $L_k(n, d) \le \frac{1}{n} \binom{n}{d}(k-1)^d$. Using these bounds and inductive proofs, the total computation tree size is shown to be proportional to $N_k(n, d)$, leading to the conclusion that `GenFix` is CAT (Theorem 4.7).

### Generating Fixed-Density Bracelets: Challenges and the Naive Approach

Generating bracelets with fixed density presents additional challenges compared to necklaces. A simple modification of the necklace generation algorithm is a starting point, but it does not immediately yield a CAT algorithm.

A **naive algorithm**, SimpleBFD(t, p, r), can be derived by modifying the necklace generation algorithm (like `Gen(t, p)` or a variant) to list fixed density bracelets. This algorithm needs to ensure two things:
1.  All generated prenecklaces must have a density equal to the target density $d$. This requires keeping track of the count of non-zero characters.
2.  Only bracelets are listed.

Checking if a string is a bracelet typically involves comparing it to the necklace of its reversed string. A direct implementation of this check takes $O(n)$ time for a string of length $n$, which prevents the algorithm from being CAT.

Instead of a full reversal check, the naive algorithm can utilize Lemma 3.1 from (and). This lemma provides a condition for a necklace $\alpha = a_1a_2 \cdots a_n$ to be a bracelet. If $r$ is the length of the longest prefix such that $a_1 \cdots a_r = a_r \cdots a_1$, then $\alpha$ is a bracelet if and only if $a_{r+1} \cdots a_n \le a_n \cdots a_{r+1}$ and there is no index $t$ such that $a_1 \cdots a_t > a_t \cdots a_1$.

The naive algorithm uses a function `CheckRev(t)` that compares the current prenecklace prefix $a_1 \cdots a_t$ with its reversal $a_t \cdots a_1$. `CheckRev(t)` returns 1 if the prefix is lexicographically less than its reversal, 0 if they are equal, and -1 if it is greater. If `CheckRev(t)` returns -1, further generation from this prenecklace is stopped as it cannot lead to a lexicographically minimal bracelet. If it returns 0 (equal), a parameter $r$ representing the length of the longest equal-to-reversal prefix is updated. When the prenecklace reaches length $n$, the condition $a_{r+1} \cdots a_n \le a_n \cdots a_{r+1}$ is checked.

However, the final comparison $a_{r+1} \cdots a_n \le a_n \cdots a_{r+1}$ at length $n$ still takes $O(n-r)$ time in the naive approach, which is not constant time, preventing it from being a CAT algorithm.

### Towards an Efficient Algorithm for Fixed-Density Bracelets

To achieve a CAT algorithm for generating fixed-density bracelets, the approach is to merge optimizations from two existing CAT algorithms: the fixed-density necklace generation algorithm and the general bracelet generation algorithm.

**Fixed Density Optimizations (from):** As discussed, these optimizations, used in `GenFix`, involve increasing the density (number of non-zero symbols) by one in each main recursive step instead of appending single characters. They use arrays `a` (positions of non-zeros) and `b` (values of non-zeros). Specific density constraints are applied to the positions of non-zero symbols, and recursion stops when $d-1$ non-zeros are placed, handling the last non-zero at position $n$ separately.

**Bracelet Optimizations (from):** These optimizations are aimed at making the reversal check efficient.
1.  **Limited Reverse Checks:** Instead of checking all reverse rotations, if a necklace $\alpha$ starts with $i$ identical characters $a$ followed by a different character ($a^i b \ldots$), only specific reverse rotations starting with $a^i$ need to be checked. This check can be done early, but still might take $O(t)$ work for a prenecklace of length $t$.
2.  **Incremental Reverse Check:** The final comparison $a_{r+1} \cdots a_n \le a_n \cdots a_{r+1}$ (from Lemma 3.1) is made efficient by starting the comparison once the "middle point" $\lfloor (n-r)/2 \rfloor + r$ is reached. An additional parameter, RS (Reverse Status), is used to store the intermediate comparison results. RS is updated based on comparing the current character $a_{t-1}$ with its corresponding character in the reversed substring $a_{n-t+2+r}$. This makes the comparison a constant time test per recursive call.

### Merging Optimizations and Handling Complexity

The core of the efficient algorithm, named `BraceFD(t, p, r, RS)`, lies in merging these fixed-density and bracelet optimizations. It uses the fixed-density approach of increasing density and using arrays `a` and `b`. It also incorporates the bracelet optimizations, particularly the limited reverse checks and the incremental RS update. The algorithm assumes $0 < d < n$, so strings start with 0. The limited reverse checking (Optimization 1) is applied when the prenecklace has a specific form $0^i b_{i+1} \cdots b_{a_t-i-1} 0^i b_{a_t}$. If the prefix $0^i b_{i+1} \cdots b_{a_t-i-1} 0^i$ is equal to its reversal, $r$ is updated; if the reversal is less, further computation is terminated.

A key challenge in merging the algorithms arises from the fixed-density optimization potentially increasing the prenecklace length by more than one character in a single recursive step (by appending a block of zeros). This makes the simple incremental RS update (Bracelet Optimization 2), which assumes character-by-character appending, non-constant time whenever $a_t - a_{t-1} > 1$.

To address this, the computation of RS is carefully handled. The RS value is computed only when $a_t > (n-r)/2 + r$. If $b_{a_t} \ne b_e$ (where $e = n - a_t + r + 1$), RS can be computed in unit time. If $b_{a_t} = b_e$, it requires checking a substring of zeros. Variables $s_i$ (density of $b_1 \cdots b_i$) and $l_i$ (length of zero substring starting at position $i$) are introduced. The length of the zero substring starting at $e+1$ is $l_{e+1} = a_{s_{e+1}} - a_{s_e} - 1$. RS is then updated based on $b_{a_t}$, $b_e$, and $l_{e+1}$:
$$\text{RS} = \begin{cases} \text{false} & \text{if } b_{a_t} > b_e \\ \text{true} & \text{if } b_{a_t} < b_e \text{ or } (a_t - a_{t-1} - 1 > l_{e+1} \text{ and } b_{a_t} = b_e)\end{cases}$$
This ensures the RS computation remains efficient.

### The Efficient Algorithm Structure and Correctness

The recursive scheme is `BraceFD(t, p, r, RS)`. An initialization function, `InitFixed()`, is used to set up the initial calls by placing the first non-zero symbol at valid positions (from $n-d+1$ to $(n-1)/d+1$).

The algorithm works by generating strings that belong to the set $N_k(n, d)$ in lexicographic order and eliminating those necklaces whose reversed rotations are lexicographically less than the necklace itself (thus not being the lexicographically minimal representation under reversal).

**Theorem 3.1** states that the function `InitFixed()` lists all elements of $B_k(n, d)$ exactly once in lexicographic order. This proves the correctness of the algorithm.

### Analysis of the Efficient Bracelet Algorithm (Proving CAT)

The analysis aims to show that the `BraceFD` algorithm runs in CAT. This is done by demonstrating that the total number of basic operations is proportional to the number of bracelets generated, $|B_k(n, d)|$.

The computation tree represents the recursive calls. The size of this tree for `BraceFD` is claimed to be proportional to $|B_k(n, d)|$, which is reasonable given $N_k(n, d) \le 2B_k(n, d)$ and the fixed-density necklace algorithm is CAT. Most of the work per recursive call is constant, except potentially for the `CheckRev(t)` function. Therefore, the key is to prove that the total symbol comparisons performed by `CheckRev(t)` throughout the entire algorithm run is proportional to $|B_k(n, d)|$.

The total number of prenecklaces generated by the scheme is proportional to $|B_k(n, d)|$. The cost of symbol comparisons in `CheckRev(t)` is analyzed by showing that each comparison can be mapped to a unique generated prenecklace. For binary bracelets ($k=2$), the comparison logic in `CheckRev(t)` compares $b_j$ with $b_{a_t-j}$. It stops when $j > a_t/2$ or $b_j \ne b_{a_t-j}$. Since there is at most one unequal comparison per prenecklace check, the cost is considered constant per check.

To bound the number of *equal* comparisons (which continue until the middle), a mapping function $f(\beta, j)$ is defined for binary prenecklaces $\beta$ and indices $j$ where the comparison $b_j = b_{a_t-j}$ occurs. This mapping preserves length and content. Lemma 4.1 (in, referring to a lemma in) and Lemma 4.2 prove that this mapping $f$ is one-to-one and maps to valid prenecklaces generated by the algorithm. This establishes that the number of equal comparisons for binary cases is proportional to the number of prenecklaces generated.

For the general case ($k > 2$), the binary mapping $f$ doesn't always produce a valid prenecklace. A more complex mapping $g(\beta, j)$ is introduced, which also preserves length and content. Lemma 4.3 and Lemma 4.4 prove that $g(\beta, j)$ is a valid prenecklace and the mapping $g$ is one-to-one for valid $\beta$ and $j$. This generalizes the result: the total equal comparisons across all prenecklaces generated are proportional to the number of prenecklaces.

Let $P'_k(n, d)$ be the number of prenecklaces generated by the fixed density necklace algorithm, and $C_k(i)$ be the number of equal comparisons made by `CheckRev(t)` on prenecklaces of length $i$. The total number of equal comparisons is $\sum_{i=1}^n C_k(i)$. Lemma 4.4 (in, referring to a lemma in) shows that $P'_k(n, d)$ for prenecklaces not ending in zero is less than $2N_k(n, d)$. Using bounds derived from the necklace analysis, $\sum_{i=1}^{n-1} C_k(i) \le P'_k(n, d)$ and $P'_k(n, d) \le c'N_k(n, d)$ for some constant $c'$. Combining these with $C_k(n) \le 2N_k(n, d)$ and $N_k(n, d) \le 2B_k(n, d)$, the total number of comparisons is bounded: $\sum_{i=1}^n C_k(i) \le (c' + 2)N_k(n, d) \le 2(c' + 2)B_k(n, d) = cB_k(n, d)$ for some constant $c$. This proves **Theorem 4.1**: The total equal comparisons are proportional to $|B_k(n, d)|$. Since all other work per generated prenecklace is constant, the algorithm is CAT. The algorithm is also stated to take asymptotic linear space.

### Conclusion and Future Directions

By merging sophisticated optimizations from fixed-density necklace generation and general bracelet generation, and addressing the complexities arising from their combination (particularly the incremental reverse status update), a CAT algorithm has been developed. The algorithm works for arbitrary alphabet size $k$ and lists all elements of $B_k(n, d)$ exactly once in lexicographic order. The theoretical analysis, involving detailed mapping proofs, confirms that the algorithm achieves Constant Amortized Time performance.

Future work suggested includes exploring applications of this algorithm and developing schemes for listing other restricted classes of bracelets. Open problems mentioned include generating k-ary necklaces with fixed content (where the count of *each* character is fixed) for $k>2$ and counting the number of fixed-density prenecklaces.

The described scheme has been implemented in C and is available from the authors.

In summary, the presented work provides a significant step in the field of combinatorial generation by offering a provably efficient method for listing fixed-density bracelets, objects important in various applications, thereby extending the capabilities of efficient combinatorial algorithms beyond unrestricted or fixed-content cases for this class of structures.


## Finding Difference Covers: Exhaustive Search vs. Deep Reinforcement Learning

Difference covers, also known as difference sets, are special mathematical arrangements or sets of numbers that possess a unique property: the differences between any two numbers within the set cover a wide range of values, ideally every possible remainder when divided by a given N, without excessive repetition. These mathematical structures are not merely theoretical constructs but have practical significance in various fields, including coding theory, cryptography, and signal processing. The problem involves selecting exactly D numbers from a total range of 0 to N-1. This essay explores two distinct computational approaches for finding these structures: a systematic recursive search algorithm and a trial-and-error method based on reinforcement learning. Both approaches leverage parallel processing to enhance their efficiency in tackling this combinatorial challenge.

### The Difference Cover Problem Defined

At its core, the difference cover problem is a puzzle about finding a specific subset of numbers. Given a total range of numbers from 0 to N-1 and a required set size D, the task is to select D numbers such that the positive differences between every unique pair of selected numbers modulo N include all possible values from 1 to N-1 at least once (or cover "every possible remainder when divided by N" in the RL context). The problem can be visualized as placing D markers on a circle with N points (representing 0 to N-1) such that the distances between pairs of markers cover all possible distances around the circle.

The sources specify certain constraints for valid inputs (N, D). Both N and D must be at least 3. Additionally, N cannot exceed the value D*(D-1)+1. These constraints ensure that the mathematical problem is well-defined and potentially has solutions.

Valid solutions, when found, are typically printed as sequences of the D numbers that constitute the difference cover. Progress information, such as the number of worker threads used and the remaining workload, is also displayed.

### Approach 1: Recursive Search (diff_cover.cpp / diff_cover3.cpp)

The first approach employs a **systematic, algorithmic method** to find difference covers. It is described as a "puzzle solver" that searches for these special mathematical arrangements. This method is based on a **sophisticated recursive search algorithm combined with parallel processing**. The core logic resides in the `DcGenerator` class. This approach follows a **generate-and-test methodology with intelligent pruning**.

The process involves **building potential solutions incrementally**, adding one number at a time to the current set. As each new number is added, the algorithm calculates all the differences between this new number and the previously selected numbers, updating its internal tracking of which differences have been seen.

A critical technique used is **backtracking**. This means that when the algorithm reaches a **dead end** – a partial solution that cannot possibly lead to a valid complete solution – it backs up, undoes the last choice, and tries a different one. The `step_forward` and `step_backward` functions are central to managing this process, updating and reverting the difference counts as the algorithm explores and backtracks through the search space.

**Optimization** is a key aspect of this approach to avoid wasting computational time on impossible solutions. Before exploring deeper into a potential solution path, the algorithm checks whether there are **enough unique differences** covered by the current partial set to possibly reach a valid final answer. If this condition is not met, that path is immediately abandoned (pruned). Arrays are maintained to track seen differences and count unique ones.

The sources mention arrays `a`, `b`, and `differences` within the `DcGenerator` class. Array `a` appears to store the current configuration or selected numbers, with `a` initialized to 0 and `a[D]` used as a sentinel value. Array `differences` tracks which differences have been seen, with `differences` initialized to 1 (as the difference between a number and itself is 0, and 0 is always "covered"). The `q` array is also mentioned in one source, seemingly tracking chosen elements. The `b` array is mentioned in another source as storing auxiliary information.

The `step_forward` function takes the index `t` of the newly added number (`a[t]`) and the current unique `count` of differences. It iterates through the previously selected numbers (`a[j]` for `j < t`), calculates the positive difference (`a[t] - a[j]`) and the modular negative difference (`N - (a[t] - a[j])`), takes the minimum of these two to get the 'diff', increments the count for that 'diff' in the `differences` array, and if this is the first time seeing this 'diff', increments the total unique `count`.

The `step_backward` function mirrors `step_forward`. Given the index `t` of a number being removed, it iterates through previous numbers (`a[j]` for `j < t`), calculates the same 'diff' value as in `step_forward`, and decrements the count for that 'diff` in the `differences` array. This effectively reverts the state to before the number `a[t]` was added.

**Conceptual Example of Step Forward/Backward (Illustrative, not from source trace):**

Imagine trying to find a difference cover for N=5, D=3.
Start with the first number, typically 0. `a = [0, ?, ?, ?, 5]`.
The first choice is `a`. Let's say the algorithm tries `a = 1`.
Call `step_forward(1, count)`. `a` is 1, `a` is 0.
`p_diff = 1 - 0 = 1`. `n_diff = 5 - 1 = 4`. `diff = min(1, 4) = 1`.
`differences` is incremented. If it was 0, `count` becomes 1.
Now, `a` is `[0, 1, ?, ?, 5]`.
Next, try `a`. Let's say the algorithm tries `a = 3`.
Call `step_forward(2, count)`. `a` is 3.
Check against `a=0`: `p_diff=3`, `n_diff=2`, `diff=2`. `differences` incremented. If it was 0, `count` increases.
Check against `a=1`: `p_diff=2`, `n_diff=3`, `diff=2`. `differences` incremented again.
Current set is {0, 1, 3}. Differences are 1-0=1, 3-0=3, 3-1=2. Modulo 5, these are 1, 3, 2. The negative differences are 0-1=4, 0-3=2, 1-3=3. Modulo 5, these are 4, 2, 3. The unique positive/modular differences are {1, 2, 3, 4}. We need to cover {1, 2, 3, 4} for N=5. Here, {1, 2, 3, 4} are covered. This set {0, 1, 3} is a solution.
If {0, 1, 3} wasn't a solution, and the algorithm needed to backtrack from `a=3`, it would call `step_backward(2)`.
`step_backward(2)`: `a` is 3.
Check against `a=0`: `p_diff=3`, `n_diff=2`, `diff=2`. `differences` is decremented.
Check against `a=1`: `p_diff=2`, `n_diff=3`, `diff=2`. `differences` is decremented.
The state is reverted to where only {0, 1} were chosen, and the algorithm tries a different value for `a`.

The `PrintD` function is called when the algorithm reaches a depth where D-1 numbers have been selected (i.e., position `D1`). It performs final validation checks, including ensuring the configuration is "minimal" and checking if the number of unique differences (`count`) is at least `N2` (`N / 2`). If these checks pass, the found difference cover (`a` to `a[D]`) is printed. Note that the modular differences are often considered up to N/2 because if a difference `d` exists, the difference `N-d` also exists between the same two numbers in reverse order. Covering all differences up to N/2 is sufficient to cover all N-1 non-zero differences modulo N for odd N, and N/2 for even N if the difference N/2 is covered. The threshold `N2` is used in the check. The `PrintD` function calls `step_forward` with `D1` to update the final difference counts for the complete set before checking the `N2` condition, and then `step_backward` with `D1` to revert the counts, allowing the search to continue.

Symmetry-breaking optimizations are included, particularly via the `CheckRev` function. This helps to avoid generating and checking equivalent sequences that are simply mirror images or rotations of each other, significantly reducing the search space without missing unique solutions. The `CheckRev` function compares the current partial sequence (`a` up to index `t_1`) with its reversal. It iterates from the second element (`a`) up to the midpoint (`t_1 / 2`) and compares elements `q[j]` with `q[t_1 - j]`. It returns 1 if the current sequence is lexicographically less than its reverse, -1 if greater, and 0 if they are equal up to that point. This optimization is crucial in generating bracelets with fixed density, which is related to the problem being solved. The `BraceFD` and `BraceFD11` functions seem to implement this bracelet generation logic within the difference cover search.

The search is accelerated by using **parallel processing** through a **thread pool**. The work is divided by having different threads **start their searches from different initial values**. The `InitParallel` function initializes the thread pool and enqueues tasks for different starting values `j` within a calculated range (from `(N+1)/2` down to `(N-1)/D+1`). Each task runs an instance of the `DcGenerator` with a specific starting value and calls `Gen11` (or `BraceFD11` in `diff_cover3.cpp`) to begin the recursive generation process for that search partition. Multiple CPU cores work simultaneously on these different partitions of the search space, significantly reducing the total computation time. The main thread waits for these worker threads to complete, displaying a countdown of remaining tasks.

The source also details the complexity analysis for a related problem, generating bracelets with fixed density. It discusses the goal of achieving **Constant Amortized Time (CAT)**, meaning the total time taken is proportional to the number of objects generated, so each successive object is listed in constant time on average. The paper proves that the optimized algorithm for bracelets with fixed density works in CAT. This is achieved by ensuring that the work done in each recursive call to `BraceFD` to append the next non-zero symbol is constant. While the `CheckRev` function involves comparisons, the paper argues that the total number of symbol comparisons performed by `CheckRev` across the entire generation process is proportional to the number of bracelets generated, thus maintaining the CAT complexity. This proof involves mapping comparisons to unique prenecklaces and showing the mapping is one-to-one, particularly for binary cases and then generalized for k-ary cases using a different mapping `g`. The core idea is that the number of "expensive" operations (like comparisons in `CheckRev`) is bounded by a constant times the number of objects found.

This systematic approach is described as **guaranteed** to find solutions within the defined search space if they exist and if the search is exhaustive. It relies on **explicit pruning rules derived from mathematical properties**.

### Approach 2: Reinforcement Learning (RL Implementation)

The second approach treats the problem of finding difference covers as a **strategic game** to be solved by artificial intelligence. It uses **reinforcement learning (RL)**, a technique akin to **teaching a computer through trial and error**. This is compared to learning a video game by experimenting with different strategies and improving over time.

At the heart of this RL implementation is an **artificial "brain" called a PolicyNetwork**. This is a neural network designed for decision-making. The PolicyNetwork has **three layers of artificial neurons**.
- The **first layer (Input Layer)** receives information about the **current state of the puzzle**.
- The **middle layers (Hidden Layers)** process this information.
- The **final layer (Output Layer)** makes the decision on **which number to pick next**.

The PolicyNetwork starts with random decision-making capabilities. However, through the learning process, it **learns and improves over time by remembering what worked well and what didn't**. The weights and biases of the network are initialized using **Xavier initialization**, a technique mentioned as helping to maintain variance of activations across layers for better training. ReLU (Rectified Linear Unit) activation is applied in the hidden layers.

The learning process involves the program playing the "difference cover game" many times, potentially thousands. Each attempt or game is referred to as an **episode**.

1.  **State Representation**: In each step of an episode, the AI observes the **current situation** of the puzzle. This situation includes which numbers have already been picked and which mathematical differences have been covered so far. This information is converted into a numerical format, specifically a **"state vector"**. The state vector has a size of 2*N: N elements representing which numbers are chosen (1 if chosen, 0 otherwise) and N elements representing which residues/differences are covered (1 if covered, 0 otherwise).

    **Conceptual Example of State Vector (Illustrative):**

    Imagine N=5, D=3. Suppose {0} is chosen initially.
    Chosen vector (size 5): ``
    Residues covered (size 5): `` (only difference 0 is covered initially, modulo 5)
    State vector (size 10): `[1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f]` (concatenation)

    If the AI then chooses number 1.
    Chosen vector: ``
    Differences between 0 and 1: 1-0=1 (diff 1), 0-1=-1 (mod 5 is 4).
    Residues covered: difference 0 (from 0-0, 1-1), difference 1 (from 1-0), difference 4 (from 0-1).
    Residues vector: ``
    New State vector: `[1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f]`

2.  **Decision Making**: The PolicyNetwork takes the state vector as input and performs a **forward pass**. It processes this information through its layers and calculates raw scores called **"logits"** for picking each of the N possible numbers. These logits are then converted into **probabilities** for picking each remaining number using the **Softmax function**. Numbers that the network considers more promising receive higher probabilities. Crucially, already chosen numbers are masked by setting their logits to a very low value before applying softmax, ensuring they are not selected again.

    The Softmax function is defined as:
    $$ \text{Softmax}(z_i) = \frac{e^{z_i - \max(z)}}{\sum_j e^{z_j - \max(z)}} $$
    where $z_i$ are the logits, and $\max(z)$ is the maximum logit for numerical stability. This function outputs a probability distribution over the possible actions (picking a number).

3.  **Action Selection**: The AI then **randomly selects a number (action)** based on the probabilities generated by the network. Random selection based on probabilities allows the AI to **explore different strategies** rather than always picking the seemingly best option, which is important for learning. The selection is done using a discrete distribution based on the calculated probabilities.

4.  **Reward Calculation**: After selecting a number and updating the puzzle state (marking the number as chosen, updating covered residues), the AI receives a **"reward"**. This reward is based on **how many new mathematical differences this choice covers**. More new coverage results in a better (higher) reward. This immediate reward signal guides the learning process.

5.  **Learning**: After completing a full episode (attempting to pick D numbers), the AI analyzes what happened. If the episode led to finding a solution, or if certain actions or sequences of actions resulted in good cumulative rewards, the PolicyNetwork **adjusts its internal parameters (weights and biases)** to make similar decisions more likely in the future. This adjustment involves calculating **"gradients"** – mathematical measures of how much each weight and bias should change to improve performance. These gradients are computed using a policy gradient method, working backward from the episode's outcomes and rewards. The rewards collected during the episode are typically aggregated into **"discounted returns"**, which account for future potential rewards, often using a discount factor (GAMMA). These returns are then normalized for more stable training.

The core update rule for network parameters (like weights W or biases b) based on gradients (gradW or gradB) and the learning rate is described as:
$$\text{parameter} = \text{parameter} - \text{LEARNING RATE} \times \text{gradient}$$

This update process, specifically shown for weights and biases of the three layers (`W1, b1, W2, b2, W3, b3`), is performed by the `policyNet.update` method.

To accelerate the learning process, similar to the recursive search, the program utilizes **multi-threading**. Multiple **"worker threads"** are run simultaneously. Each worker thread is like an independent AI agent trying to solve the puzzle. Crucially, **they all share the same PolicyNetwork (brain)** and learn from each other's experiences by contributing their calculated gradients to update the shared network. This parallel approach significantly speeds up the discovery process.

The `workerThread` function encapsulates the logic for a single thread. Each thread runs a loop that continues as long as no solution has been found by any thread and the total episode count across all threads is below `MAX_EPISODES`. Inside the loop, a thread increments the shared episode counter, initializes the puzzle state, interacts with the "environment" (picks numbers) for D-1 steps, stores the states, actions, and rewards experienced during the episode. After the episode, it checks if a solution was found. If yes, it uses a **mutex (`outputMutex`)** to safely print the solution and set a shared `solutionFound` flag. If not a solution, it calculates discounted returns, normalizes them, computes the gradients for the network parameters based on the policy gradient method, and then uses a **mutex (`networkMutex`)** to safely update the shared PolicyNetwork with these gradients. The mutexes (`networkMutex` and `outputMutex`) are essential for thread safety, ensuring that shared resources (the neural network parameters and the output stream) are accessed and modified by only one thread at a time.

The `findDifferenceCoverRL` function orchestrates the parallel RL process. It initializes the PolicyNetwork, creates shared atomic variables (`episodeCounter`, `solutionFound`) and the output mutex, launches the specified number of `NUM_THREADS` worker threads, and waits for them to finish using `join()`. Finally, it reports if no solution was found after exhausting the `MAX_EPISODES` limit.

This reinforcement learning approach is described as **learned**, developing a strategy through experience. Unlike the systematic search, it is **not guaranteed** to find a solution, especially within a limited number of episodes (`MAX_EPISODES`). It learns to find *a* solution, not necessarily all of them. Its success relies on the AI **"discovering" good strategies based on the reward signals** it receives during training.

### Common Elements and Key Differences

Both the recursive search and the reinforcement learning approaches share a fundamental strategy for efficiency: they **utilize parallel processing**. Both employ worker threads (or a thread pool) to significantly speed up their respective processes, whether that's exploring different starting points in a search space or running multiple learning agents simultaneously.

However, their core methodologies are distinctly different, as summarized in the sources:

| Feature           | Recursive Search                   | Reinforcement Learning            |
| :---------------- | :--------------------------------- | :-------------------------------- |
| **Approach**      | Systematic, algorithmic search     | Trial and error, learned experience |
| **Guarantee**     | Guaranteed (if search exhaustive)  | May not find solution (within episode limit) |
| **Discovery**     | Explicit pruning rules             | Discovers strategies based on rewards |
| **Core Logic**    | Recursive function calls (`GenD`, `BraceFD`) | Neural Network (`PolicyNetwork`) |

The recursive search systematically explores combinations using backtracking and predefined mathematical rules for pruning branches of the search tree that cannot lead to a solution. The RL approach, conversely, learns through interaction with the problem, adjusting a neural network's parameters based on trial and error guided by rewards.

### Applications

As mentioned earlier, the mathematical structures known as difference covers have practical applications. The sources specifically cite their use in areas like **coding theory, cryptography, and signal processing**. This underscores the relevance of developing efficient methods for finding these sets.

### Conclusion

The problem of finding difference covers is a combinatorial challenge with practical applications. The sources present two distinct, parallelized approaches to tackle this problem. The recursive search offers a systematic, guaranteed method that leverages mathematical properties and pruning to efficiently explore the solution space, enhanced by parallel processing across different search partitions. The reinforcement learning approach, on the other hand, frames the problem as a game and learns a strategy through trial and error guided by rewards, using a neural network as its decision-making core and accelerating learning through parallel worker threads sharing the same network. While the recursive method guarantees finding solutions if they exist within its search space, the RL method offers a learned approach that may discover solutions but is not guaranteed to find one within a fixed number of attempts. Both demonstrate the power of parallel computation in speeding up complex search and learning processes.



\clearpage

## References
 W.-S. Luk and T.-T. Wong, ‘‘Two new quorum based algorithms for distributed mutual exclusion,’’ in Proc. 17th Int. Conf. Distrib. Comput. Syst., 1997, pp. 100–106.

 C. J. Kleinheksel and A. K. Somani, ‘‘Scaling distributed all-pairs algorithms,’’ in Proc. Int. Conf. Inf. Sci. Appl., 2016, pp. 247–257.
 Marshall Hall, Jr., Combinatorial Theory, chapter 11, John Wiley & Sons, 1986.

 A. Reverter and E. K. Chan, ‘‘Combining partial correlation and an information theory approach to the reversed engineering of gene co-expression networks,’’ Bioinf., vol. 24, no. 21, pp. 2491–2497, 2008.

 Y. Bian and A. K. Somani, "CQS-Attention: Scaling Up the Standard Attention Computation for Infinitely Long Sequences," IEEE Access, vol. 13, pp. 3544550-3544563, 2025.

 M. Maekawa, ‘‘A $\sqrt{N}$ algorithm for mutual exclusion in decentralized systems,’’ ACM Trans. Comput. Syst. (TOCS), vol. 3, no. 2, pp. 145–159, 1985.

 C. J. Kleinheksel and A. K. Somani, ‘‘Efficient distributed all-pairs algorithms: Management using optimal cyclic quorums,’’ IEEE Trans. Parallel Distrib. Syst., vol. 29, no. 2, pp. 391–404, Feb. 2018.
 
 Shouwen Lai, Bo Zhang, Binoy Ravindran, and Hyeonjoong Cho, "CQS-Pair: 
 Cyclic Quorum System Pair for Wakeup Scheduling in Wireless Sensor Networks", Systems Software Research Group.
 
 Cory J. Kleinheksel and Arun K. Somani, "Scaling Distributed All-Pairs Algorithms: Manage Computation and Limit Data Replication with Quorums".
