#ifndef DAPPLE_TESTPACKING_H
#define DAPPLE_TESTPACKING_H

#include<iostream>
#include "../CRYPTO/DTPKC.h"
#include "../CRYPTO/EvalAdd.h"
#include "../CRYPTO/EvalSub.h"
#include "../CRYPTO/EvalDiv.h"
#include "../CRYPTO/EvalDiv100.h"
#include "../CRYPTO/EvalEntropy.h"
#include "../CRYPTO/EvalProba.h"
#include "../CRYPTO/EvalHBound.h"
#include "../CRYPTO/SkeySwitch.h"
#include "../CRYPTO/EvalCmp.h"
#include "../CRYPTO/EvalMult.h"
#include "../CRYPTO/EvalAddPow2.h"
#include "../CRYPTO/EvalStandardDiv.h"
#include "../CRYPTO/EvalSigmoid.h"
#include "../CRYPTO/EvalDotProd.h"
#include "../UTIL/mpz_class.hh"
#include "../UTIL/num_th_alg.hh"

class TESTpacking {

public : void run(bool deserialize, int keysize, int prec, int error, string filename);

};


#endif
