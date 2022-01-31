/**
** Class :  DTPKC
 * Authors : Talbi Rania
** Creation date : 31/10/2018
** Last Update : 28/11/2018
** Purpose : DT-PKC cryptosystem primitives
 *
 * Remarks :
 * This implementation only handles positive integers
 *
**/

#include <thread>
#include "DTPKC.h"
#include "../UTIL/math_util.hh"


mpz_class DTPKC :: n,DTPKC :: g, DTPKC :: n2, DTPKC :: pkw, DTPKC:: randomR, DTPKC:: randomGR, DTPKC::powPK;
int DTPKC :: size;
mpz_class DTPKC :: p, DTPKC :: q, DTPKC :: a, DTPKC :: SK, DTPKC :: SK0, DTPKC :: SK1;
int DTPKC :: blindVal;
int DTPKC :: precision;
double DTPKC:: timeMU;
double DTPKC:: timeSU;
double DTPKC:: bdwMU;
double DTPKC:: bdwSU;
int DTPKC::cmpCtr;
bool  DTPKC::optim;


mpz_class DTPKC :: R1, DTPKC :: R2, DTPKC :: R3, DTPKC :: R4;
Cipher DTPKC :: CR1, DTPKC :: CR2, DTPKC :: CR3, DTPKC :: CR4;


std::chrono::milliseconds DTPKC :: delay;


/**
** Function : keygen
 * Author : Talbi Rania
** Creation date : 31/10/2018
** Last Update : 05/11/2018 : Corrected some errors after tests
 *
** Purpose : This function allows to generate DT-PKC cryptosystem parameters.
 * For mor details please refer to [1]
 * [1] Liu, X., Deng, R. H., Choo, K. K. R., & Weng, J. (2016). An efficient privacy-preserving
 * outsourced calculation toolkit with multiple keys. IEEE Transactions on Information Forensics
 * and Security, 11(11), 2401.
 *
 * Parameters :
 * state : random seed used to generate the DT-PKC parameters.
 * nbits : bit-size of the systems's strong keys (SK0, SK1).
 * error : The certainty of the generated primes ( German primes ).
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

void
DTPKC::keygen(int pr, gmp_randstate_t state, std::chrono::milliseconds delay,  uint nbits, int error, bool optim, int bl)
{
    precision=pr;
    DTPKC::timeMU = 0.0;
    DTPKC::timeSU = 0.0;
    DTPKC::bdwMU = 0.0;
    DTPKC::bdwMU = 0.0;
    DTPKC::cmpCtr = 0;
    //
    // a : Exponent used to find a generator g with a high probability
    // s : SK1+ SK2, (SK*invlambda)%(SK*n2).
    // tita : Exponent that allows to generate the initial pkw
    // invlambda : The modulus inverse of lambda :  Used to find SK0 and SK1
    //

    mpz_class  s, tita,invlambda;
    size = nbits;
    DTPKC::delay = delay;
    DTPKC::optim = optim;

    bool valid = false;
    do {

        do {
            // Generate p and q (Germain primes)

            gen_germain_prime(p, nbits / 2, state, error);
            gen_germain_prime(q, nbits / 2, state, error);

            // Compute n and n square.
            n = p * q;
            n2 = n * n;

        } while ((nbits != (uint) mpz_sizeinbase(n.get_mpz_t(), 2)) ||
                 p == q); // We must obtain the exact nbits size and p must be different than q


        // Compute a ( random value = random value uniformly distributed in Z N square )
        gen_germain_prime(a, nbits, state, error);

        // Compute g
        g = mpz_class_powm(a, 2 * n, n2);
        g = -g % n2;

        // Compute the strong key
        SK = LCM(p - 1, q - 1) / 2;

        // Compute the modulus inverse of LCM(p-1, q-1)/2
        mpz_class_invert(invlambda, SK, n2);

        // Compute s : used to find out the partial keys s=SK1+SK2
        s = (SK * invlambda) % (SK * n2);
        SK0 = mpz_class_get_z_bits(nbits);
        SK1 = s - SK0;

        // Generate the joint key
        tita = mpz_class_get_z_bits(nbits / 4);
        pkw = mpz_class_powm(g, tita, n2);

        blindVal = bl;

        mpz_class r = mpz_class_get_z_bits(blindVal);
        Cipher c_r, S1;
        c_r = enc(r,pkw);

        S1.T1=mpz_class_powm(c_r.T1,n-1,n2*n2);
        S1.T2=mpz_class_powm(c_r.T2,n-1,n2*n2);
        S1.Pub=pkw;

        Cipher inter;
        inter.T1 = S1.T1 * c_r.T1;
        inter.T2 = S1.T2 * c_r.T2;
        inter.Pub = S1.Pub;

        if ( Sdec(inter) == 0 ) valid = true;


        gmp_randstate_t state;

        // Generate a random nupmber r
        gmp_randinit_default(state);
        gmp_randseed_ui(state,time(NULL));
        randomR=mpz_class_get_z_bits(size);
        gmp_randclear(state);

        randomGR=mpz_class_powm (g,randomR,n2);

        powPK=mpz_class_powm (pkw,randomR,n2);

    }while(!valid);
    R1= mpz_class_get_z_bits(blindVal);
    R2= mpz_class_get_z_bits(blindVal);
    R3= mpz_class_get_z_bits(blindVal);
    R4= mpz_class_get_z_bits(blindVal);

    CR1 = enc(R1, pkw);
    CR2 = enc(R2, pkw);
    CR3 = enc(R3, pkw);
    CR4 = enc(R4, pkw);


}

/**
** Function : enc
 * Author : Talbi Rania
** Creation date : 02/11/2018
** Last Update : 05/11/2018
 *
** Purpose : This function is used  to encrypt a plaintext using a given a given public key
 * For mor details please refer to [1]
 * [1] Liu, X., Deng, R. H., Choo, K. K. R., & Weng, J. (2016). An efficient privacy-preserving
 * outsourced calculation toolkit with multiple keys. IEEE Transactions on Information Forensics
 * and Security, 11(11), 2401.
 *
 * Parameters :
 * plaintext : The plaintext to be encrypted.
 * pkey : The public key used to encrypt the plaintext
 *
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

Cipher DTPKC :: enc(mpz_class plaintext, mpz_class pkey)
{
    // r :  random number used in the encryption
    // state : seed used dto generate the random parameter r
    // pkeyPowR : Holds the intermediate  value of power ( pkey, r )  mod N square = power ( g, r*tita )  mod N square

    gmp_randstate_t state;
    mpz_class pkeyPowR, r ;
    Cipher c;

    if (!optim) {

        // Generate a random nupmber r
        gmp_randinit_default(state);
        gmp_randseed_ui(state,time(NULL));
        r = mpz_class_get_z_bits(size);
        gmp_randclear(state);
        c.T2=mpz_class_powm (g,r,n2);
        pkeyPowR=mpz_class_powm (pkey,r,n2);
        c.T1 = (((1+plaintext*n))*(pkeyPowR))%n2;
    }
    else {
        r = randomR;
        c.T2=randomGR;
        pkeyPowR = powPK;
        mpz_class inter = (1+plaintext*n);
        //fastmul(c.T1, inter , (pkeyPowR) );
        c.T1 = inter * pkeyPowR;
        c.T1 =   c.T1 % n2;
    }


    // Initialize the cipher


    //  Compute power ( g, r*tita )  mod N square


    // (T1=  g, r*tita )  mod N square * (1+mN))mod N square


    // T2 = power (g, r) mod N square


    // Set the public key
    c.Pub=pkey;
    c.sign = 1;
    return c;
}

/**
** Function : getKey
 * Author : Talbi Rania
** Creation date : 02/11/2018
** Creation date : 02/11/2018dese
** Last Update : 05/11/2018
 *
** Purpose : This function is used  to generate a new pair of secrete and public encryption keys when requested by a data owner or a querier
 *
 * Parameters :
 * pkey : The public key to be generated
 * skey : The secrete key to be generated
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

void
DTPKC :: getKey(mpz_class &pkey, mpz_class &skey)
{

    // state : seed used dto generate the random parameter tita

    gmp_randstate_t state;

    // Generate the secret key
    gmp_randinit_default(state);
    gmp_randseed_ui(state,time(NULL));
    skey=mpz_class_get_z_bits(size/4);

    // Compute the public key
    pkey=mpz_class_powm (g,skey,n2);
    gmp_randclear(state);

}

/**
** Function : L(x)
 * Author : Talbi Rania
** Creation date : 02/11/2018
** Last Update : 05/11/2018
 *
** Purpose : This function is used  to compute L(x)=x-1/N : used in full and partial decrypting function
 * Parameters :
 * x : The preimage of the function L
 *
 * Unit Test : Tested.
 *
**/

mpz_class
DTPKC:: L(mpz_class x)
{
    return ((x-1)/n)%n;
}


/**
** Function : UpdatePkw
 * Author : Talbi Rania
** Creation date : 02/11/2018
** Last Update : 02/11/2018
 *
** Purpose : This function is used  to update the system's joint key
 *
 * Parameters :
 * pkey : The public key that needs to be multiplied by the joint key
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

void
DTPKC :: updatePkw(mpz_class pkey)
{
    //Multiply the joint key by the newly added public key
    pkw=pkw*pkey;
}

/**
** Function : dec
 * Author : Talbi Rania
** Creation date : 02/11/2018
** Last Update : 05/11/2018
 *
** Purpose : This function is used  to decrypt a ciphertext using the corresponding secret key
 * For mor details please refer to [1]
 * [1] Liu, X., Deng, R. H., Choo, K. K. R., & Weng, J. (2016). An efficient privacy-preserving
 * outsourced calculation toolkit with multiple keys. IEEE Transactions on Information Forensics
 * and Security, 11(11), 2401.
 *
 * Parameters :
 * c : The ciphertext to be decrypted
 * skey : The secret key used to decrypt the cipher
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

mpz_class
DTPKC :: dec(Cipher c, mpz_class skey)
{
    // D: T1 : The denominator in the decryption formulae
    // N : power (T2, tita) : The nominator in the decryption formulae

    mpz_class D, N;

    // Compute 1/power (T2, tita)
    D=mpz_class_powm(c.T2,skey,n2);
    D=mpz_class_invert(D,n2);
    N=c.T1;

    return L(N*D);

}

/**
** Function : Sdec
 * Author : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 05/11/2018
 *
** Purpose : This function is used to decrypt any cipher c using the strong key SK
 * (Not really used in DAPPLE because no party is allowed to have this key, so its
 * just used for tests)
 *
 * For mor details please refer to [1]
 * [1] Liu, X., Deng, R. H., Choo, K. K. R., & Weng, J. (2016). An efficient privacy-preserving
 * outsourced calculation toolkit with multiple keys. IEEE Transactions on Information Forensics
 * and Security, 11(11), 2401.
 *
 * Parameters :
 * c : The ciphertext to be decrypted
 * SK : The Strong Key : Backdoor
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

mpz_class
DTPKC::Sdec(Cipher c)
{
mpz_class invLambda = mpz_class_invert(SK,n2);
mpz_class powT1 = (((mpz_class_powm(c.T1,SK,n2)-1)/n)*invLambda)%n;

if (c.sign == 0 )
    powT1 = - powT1;

return powT1;

}

/**
** Function : Psdec1
 * Author : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 05/11/2018
 *
** Purpose : This function is used to partially decrypt a cipher c using the partial strong key SK1 held by U1
 *
 * For mor details please refer to [1]
 * [1] Liu, X., Deng, R. H., Choo, K. K. R., & Weng, J. (2016). An efficient privacy-preserving
 * outsourced calculation toolkit with multiple keys. IEEE Transactions on Information Forensics
 * and Security, 11(11), 2401.
 *
 * Parameters :
 * c : The ciphertext to be decrypted
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

Cipher
DTPKC::PSdec0(Cipher c)
{
   // pc : partial cipher
   Cipher pc;
   if (!optim)
   {
       //pc.T1= mpz_class_powm(c.T1,SK0,n2);
       fastpow2(pc.T1, c.T1,SK0, n2);
   }else
   {
       //fastpow(pc.T1, c.T1,SK0, n2);
       pc.T1= mpz_class_powm(c.T1,SK0,n2);
   }

   pc.T2=c.T2;
   pc.Pub=c.Pub;

   return  pc;
}


/**
** Function : Psdec1
 * Author : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 05/11/2018
 *
** Purpose : This function is used to decrypt a partial cipher "pc" using the partial strong key SK1 held by U2
 *
 * For mor details please refer to [1]
 * [1] Liu, X., Deng, R. H., Choo, K. K. R., & Weng, J. (2016). An efficient privacy-preserving
 * outsourced calculation toolkit with multiple keys. IEEE Transactions on Information Forensics
 * and Security, 11(11), 2401.
 *
 * Parameters :
 * c : The ciphertext to be decrypted
 * pc : The partial ciphertext resulting from Psdec1
 *
 * Unit Test : Tested (TEST/Test-DTPKC).
 *
**/

mpz_class
DTPKC::PSdec1(Cipher c,Cipher pc)
{
    // pc2 : partial cipher  2
    mpz_class pc2, res ;
    if (!optim ) {
     //   pc2 = mpz_class_powm(c.T1, SK1, n2);
     //   res = pc2 * pc.T1;

        fastpow2(pc2, c.T1, SK1, n2);

        fastmul2(res, pc2, pc.T1);

    }
    else {
        //fastpow(pc2, c.T1, SK1, n2);

        //fastmul(res, pc2, pc.T1);

        pc2 = mpz_class_powm(c.T1, SK1, n2);
        res = pc2 * pc.T1;

    }
    return L(res);
}

/**
** Function : serializeDtpkc
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : This function is used to save the parameters of the DTPKC cryptosystem inorder to restore it for another execution.
 *
 *
 * Parameters :
 * none
 *
 * Unit Test : Tested (TEST/TESTppVFDT).
 *
**/

int
DTPKC ::serializeDtpkc(string path) {

    // Out : pointer to the output file

    std::ofstream *Out;
    Out= new std::ofstream (path+"dtpkc.ser");

    *Out<< n.get_str(10) << endl;
    *Out<< g.get_str(10) << endl;
    *Out<< n2.get_str(10) << endl;
    *Out<< pkw.get_str(10) << endl;
    *Out<< size << endl;
    *Out<< p.get_str(10) << endl;
    *Out<< q.get_str(10) << endl;
    *Out<< a.get_str(10) << endl;
    *Out<< SK.get_str(10) << endl;
    *Out<< SK0.get_str(10) << endl;
    *Out<< SK1.get_str(10) << endl;

    Out->close();
    delete Out;
}

/**
** Function : deserializeDtpkc
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : This function is used to  restore the dtpkc parameters from a previous execution
 *
 *
 * Parameters :
 * none
 *
 * Unit Test : Tested (TEST/TESTppVFDT).
 *
**/

int
DTPKC ::deserializeDtpkc(string path) {

    // In :  pointer to the input file where the parameters of DTPKC where serialized
    // line : used to read the DTPKC.ser line by line

    std::ifstream *In;
    string line;

    try {
        In=new std::ifstream(path+"dtpkc.ser");

        getline(*In,line);
        n.set_str(line,10);

        getline(*In,line);
        g.set_str(line,10);

        getline(*In,line);
        n2.set_str(line,10);

        getline(*In,line);
        pkw.set_str(line,10);

        getline(*In,line);
        size=std::stoi(line);

        getline(*In,line);
        p.set_str(line,10);

        getline(*In,line);
        q.set_str(line,10);

        getline(*In,line);
        a.set_str(line,10);

        getline(*In,line);
        SK.set_str(line,10);

        getline(*In,line);
        SK0.set_str(line,10);

        getline(*In,line);
        SK1.set_str(line,10);

        In->close();

        DTPKC::timeMU = 0.0;
        DTPKC::timeSU = 0.0;
        DTPKC::bdwMU = 0.0;
        DTPKC::bdwMU = 0.0;
        DTPKC::cmpCtr = 0;


        delete In;


        DTPKC::optim = true;

        gmp_randstate_t state;

        // Generate a random nupmber r
        gmp_randinit_default(state);
        gmp_randseed_ui(state,time(NULL));
        DTPKC::randomR=mpz_class_get_z_bits(size);
        gmp_randclear(state);

        DTPKC::randomGR=mpz_class_powm (g,DTPKC::randomR,n2);

        DTPKC::powPK=mpz_class_powm (pkw,DTPKC::randomR,n2);

        DTPKC::R1= mpz_class_get_z_bits(110);
        DTPKC::R2= mpz_class_get_z_bits(110);
        DTPKC::R3= mpz_class_get_z_bits(110);
        DTPKC::R4= mpz_class_get_z_bits(110);

        DTPKC::CR1 = enc( DTPKC::R1, pkw);
        DTPKC::CR2 = enc( DTPKC::R2, pkw);
        DTPKC::CR3 = enc( DTPKC::R3, pkw);
        DTPKC::CR4 = enc( DTPKC::R4, pkw);


        return 1;

    }catch(std::exception e) // In case there is a problem (the file is not found)
    {

        return -1;
    }


}


void
DTPKC ::sendPara(int sockfd, sockaddr_in cliaddr) {

    // In :  pointer to the input file where the parameters of DTPKC where serialized
    // line : used to read the DTPKC.ser line by line


    char buffer[2000];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"%s", n.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", g.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", n2.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", pkw.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", std::to_string(size).c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", p.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", q.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));

    sprintf(buffer,"%s", a.get_str().c_str());
    sendto(sockfd, (const char *)buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", SK.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", SK0.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


    sprintf(buffer,"%s", SK1.get_str().c_str());
    sendto(sockfd, buffer, 2000, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));


}

/**
 *
** Function : EvalCmp_U1_step1
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : This function is used to achieve the first step of privacy preserving comparison between two ciphers ( same as EvalCmp ) :
 * I just put it here to avoid cyclic dependence between Cipher and EvalCmp.
 *
 *
 * Parameters :
 * A : First operand of the comparison
 * B : Second operand of the comparispon
 *
 * Unit Test : Tested (TEST/TESTppVFDT).
 *
**/

std::vector < Cipher>
DTPKC::EvalCmp_U1_step1(Cipher A, Cipher B)
{
    /**
     * State : seed used to generate the blinding value
     * S1 : r *A
     * S2 : r * B
     * S3 partially decrypted value of S1
     * S4 partially decrypted value of S2
     */
    auto begin = chrono::high_resolution_clock::now();

    gmp_randstate_t state;
    Cipher S1, S2, S3, S4;
    std::vector <Cipher> res(4);

    // Generate a random number for multiplicative blinding
    gmp_randinit_default(state);
    gmp_randseed_ui(state,time(NULL));

    mpz_class r=mpz_class_get_z_bits(blindVal);

    mpz_class r2 = mpz_class_get_z_bits(blindVal);
    Cipher C_R = enc(r2, A.Pub);

    gmp_randclear(state);
    // -r*A
    S1.T1 = mpz_class_powm(A.T1, r, n2);
    S1.T2 = mpz_class_powm(A.T2, r, n2);

    // A*r
    S1.T1 = S1.T1 *C_R.T1;
    S1.T2 =  S1.T2 *C_R.T2;
    S1.Pub = A.Pub;

    // -r*B
    S2.T1 = mpz_class_powm(B.T1, r, n2);
    S2.T2 = mpz_class_powm(B.T2, r, n2);

    // B*r
    S2.T1 = S2.T1 *C_R.T1;
    S2.T2 =  S2.T2 *C_R.T2;
    S2.Pub = A.Pub;

    // Partially decrypt the blinded values
    S3=PSdec0(S1);
    S4=PSdec0(S2);

    res[0]=S1;
    res[1]=S3;
    res[2]=S2;
    res[3]=S4;

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += res.size() * mpz_size(res[0].T1.get_mpz_t())+ mpz_size(res[0].T2.get_mpz_t())* sizeof(mp_limb_t);

    return res;

}


/**
 *
** Function : EvalCmp_U2_step2
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : This function is used to achieve the second step of privacy preserving comparison between two ciphers ( same as EvalCmp ) :
 * I just put it here to avoid cyclic dependence between Cipher and EvalCmp.
 *
 *
 * Parameters :
 * res1 : vector of the results of the first step of comparison sent by U1
 *
 * Unit Test : Tested (TEST/TESTppVFDT).
 *
**/


bool
DTPKC::EvalCmp_U2_step2(std::vector<Cipher> res1)
{
    /**
     * x : r*A
     * y : r*B
     */
    auto begin = chrono::high_resolution_clock::now();
    mpz_class x, y, out;

    // Decrypt the blinded values sent by U1
    x=PSdec1(res1[0],res1[1]);
    y=PSdec1(res1[2],res1[3]);


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU += duration.count();
    bdwSU += 1;

    std::this_thread::sleep_for(delay);


    return  x < y ;
}

/**
 *
** Function : EvalEq_U2_step2
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : This function is used to achieve the second step of privacy preserving equality test between two ciphers
 *
 *
 * Parameters :
 * res1 : vector of the results of the first step of comparison sent by U1
 *
 * Unit Test : Tested (TEST/TESTppVFDT).
 *
**/



bool
DTPKC::EvalEq_U2_step2(std::vector<Cipher> res1)
{
   /**
   * x : r*A
   * y : r*B
   */
    auto begin = chrono::high_resolution_clock::now();
    mpz_class x, y, out;

    // Decrypt the blinded values sent by U1
    x=PSdec1(res1[0],res1[1]);
    y=PSdec1(res1[2],res1[3]);
    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU += duration.count();
    bdwSU += 1;
    // return the equality test result
    return  x == y ;

}






Cipher DTPKC::packEnc(std::vector<std::string> plaintext, int slot, int sep, mpz_class pkey) {
    std::string space = "";

    for (int j=0; j < sep ; j++ )
        space+="0";

    std::string plain = "";

    for( std::string  i : plaintext ) {

        std::string tmp="";

        int size = i.length();

        for (int j=0; j < slot-size ; j++ )
            tmp+="0";

        tmp +=i;

        plain = space + tmp + plain;

    }

    //cout << plain << "*" << endl;
    //plain = space + plain;


    mpz_class plain_z;
    plain_z.set_str(plain,10);

    Cipher  res = DTPKC::enc(plain_z, pkey);
    res. slot = slot;
    res.sep = sep;

    return res;
}

std::vector<mpz_class> DTPKC::unpack(mpz_class c, int slot, int sep) {

    string c_s = c.get_str(10);
    vector<mpz_class> res;
    int i=c_s.length();

    while (i>0){

        string tmp;
        mpz_class tmp_z;
        if (i > slot + sep)
            tmp=c_s.substr(i - slot - sep  ,slot + sep);
        else
            tmp=c_s.substr(0,i);

        i= i - slot - sep;
        tmp_z.set_str(tmp,10);
        res.push_back(tmp_z);


    }

    return res;
}

std::vector<mpz_class> DTPKC::Unpackdec(Cipher c, int slot, int sep) {

    mpz_class tmp = DTPKC::Sdec(c);

    return unpack(tmp, slot, sep);
}


/*std::map<mpz_class, mpz_class> DTPKC::precompute(mpz_class g, int size) {

    std :: map <mpz_class,mpz_class> precompute;

    for (mpz_class i=0;i<size+1;i++)
    {
        precompute.insert(std::make_pair(i,));
    }


    return precompute;

}*/