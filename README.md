# Project Title

PrivML: Towards Practical Privacy Preserving Machine Learning 
Software prototype of our submilssion to ICDCS 2022

## Description

PrivML is an outsourced PPML framework based on Homomorphic Encryption, that allows optimizing runtime and bandwidth consumption 
for widely used ML algorithms, using many techniques such as pre-computation of random values, ciphertext packing, approximate
computations,and parallel computing. Our evaluation results showed that using light-weight cryptosystems 
combined with MPC to implement PPML methods is not enough to improve the efficiency of such costly methods
and that the optimization strategies adopted in \system allow a significant performance improvement that
is between 46% and 79% time-wise, and at least 84% and up to 99% space-wise compared to the closest state-of-the-art works.

## Getting Started

### Dependencies

We implemented PrivML as a C++ library. It consists of 4600~KLOC, including PPML methods, as well as their corresponding non-privacy preserving 
versions for comparison purposes.
PrivmL uses the OMP 4.5 library for parallel computing.
The framework also relies on GMP 6.1.2, and MPFR 4.0.1 libraries to implement the proposed cryptographic building 
blocks and primitives of the DT-PKC cryptosystem.



### Installing

1/ Install GMP library
2/ Install the  NTL library
3/ Instal config4cpp library installed 
4/ Manually copy /LIB/config4cpp/lib/libconfig4cpp.a to /usr/local/lib
5/ Install openmp library : run apt-get install libomp-dev (in mac os x brew install libomp)
6/ Install MFPR library to use non-arbitrary functions over big numbers

### Executing program
run ./dapple configuration_file threadsNumbers

