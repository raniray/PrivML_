/*
 * Copyright 2013-2015 Raphael Bost, Raluca Ada Popa
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

#include <gmpxx.h>
#include "../UTIL/mpz_class.hh"
#include "../UTIL/math_util.hh"
#include <vector>
#include <mpfr.h>
#include "gmp.h"
#include "../LIB/gmp-6.1.2/gmp-impl.h"
#include "../LIB/gmp-6.1.2/longlong.h"


#define HANDLE_NEGATIVE_EXPONENT 1


using namespace std;

// Garner's Algorithm for CRT, cf. Handbook of Applied Cryptography - 14.71
void mpz_t_crt(mpz_t x, const mpz_ptr *v, const mpz_ptr *m, size_t s)
{
    mpz_t u;
    mpz_t C[s];
    size_t i, j;
    
    mpz_init(u);
    for (i=1; i<s; i++) {
        mpz_init(C[i]);
        mpz_set_ui(C[i], 1);
        for (j=0; j<i; j++) {
            mpz_invert(u, m[j], m[i]);
            mpz_mul(C[i], C[i], u);
            mpz_mod(C[i], C[i], m[i]);
        }
    }
    mpz_set(u, v[0]);
    mpz_set(x, u);
    for (i=1; i<s; i++) {
        mpz_sub(u, v[i], x);
        mpz_mul(u, u, C[i]);
        mpz_mod(u, u, m[i]);
        for (j=0; j<i; j++) {
            mpz_mul(u, u, m[j]);
        }
        mpz_add(x, x, u);
    }
    
    for (i=1; i<s; i++) mpz_clear(C[i]);
    mpz_clear(u);
}

mpz_class mpz_class_log2_10 (mpz_class op )
{
    mpfr_t rop, res, res2, rounder;
    mpz_t b;
    mpz_class a;

    mpfr_init(rop);
    mpfr_init(res);
    mpfr_init(res2);
    mpfr_init(rounder);
    mpz_init (b);

    mpfr_set_ui (rounder, 100, MPFR_RNDN);
    mpfr_set_z (rop,op.get_mpz_t(),MPFR_RNDN);
    mpfr_log2 (res, rop, MPFR_RNDN);
    mpfr_mul (res2, res, rounder, MPFR_RNDN);
    mpfr_get_z (b,res2, MPFR_RNDN);
    a= mpz_class(b);

    mpfr_clear(rop);
    mpfr_clear(res);
    mpfr_clear(res2);
    mpfr_clear(rounder);
    mpz_clear(b);
    mpfr_free_cache();

    return a;
}


mpz_class mpz_class_log (mpz_class op, int lambda)
{
    mpfr_t rop, res, res2, rounder; //res2;
    mpz_t b;
    mpz_class a;
    mpfr_init(rounder);
    mpfr_set_ui (rounder,lambda, MPFR_RNDN);

    mpfr_init(rop);
    mpfr_init(res);
    mpz_init (b);
    mpfr_init (res2);

    mpfr_set_z (rop,op.get_mpz_t(),MPFR_RNDN);
    mpfr_log (res, rop, MPFR_RNDN);
    mpfr_mul (res2, res, rounder, MPFR_RNDN);
    mpfr_get_z (b,res2, MPFR_RNDN);

    a= mpz_class(b);
    mpfr_clear(rop);
    mpfr_clear(res);
    mpfr_clear(res2);
    mpfr_clear(rounder);
    mpz_clear(b);
    mpfr_free_cache();

    return a;
}


mpz_class mpz_class_exp_mul (mpz_class op, int prec) {
    mpfr_t rop, res, res2, rounder, m;
    mpz_t b;
    mpz_class a;
    mpfr_init(rop);
    mpfr_init(m);
    mpfr_init(res);
    mpfr_init(res2);
    mpfr_init(rounder);
    mpz_init (b);

    mpfr_set_ui (rounder, prec, MPFR_RNDN);
    mpfr_set_ui (m, 100000, MPFR_RNDN);

    mpfr_div (rop, rop, m , MPFR_RNDN);

    mpfr_set_z (rop,op.get_mpz_t(),MPFR_RNDN);

    mpfr_exp(res, rop, MPFR_RNDN);

    mpfr_mul (res2, res, rounder, MPFR_RNDN);
    mpfr_get_z (b,res2, MPFR_RNDN);
    a= mpz_class(b);

    mpfr_clear(rop);
    mpfr_clear(res);
    mpfr_clear(res2);
    mpfr_clear(rounder);
    mpfr_clear(m);
    mpz_clear(b);
    mpfr_free_cache();

    return a;
}


mpz_class mpz_class_exp(mpz_class op, int prec) {
    mpfr_t rop, res, res2, rounder;
    mpz_t b;
    mpz_class a;
    mpfr_init(rop);
    mpfr_init(res);
    mpfr_init(res2);
    mpfr_init(rounder);
    mpz_init (b);

    mpfr_set_ui (rounder, prec, MPFR_RNDN);
    mpfr_set_z (rop,op.get_mpz_t(),MPFR_RNDN);
    mpfr_exp(res, rop, MPFR_RNDN);

    mpfr_mul (res2, res, rounder, MPFR_RNDN);
    mpfr_get_z (b,res2, MPFR_RNDN);
    a= mpz_class(b);

    mpfr_clear(rop);
    mpfr_clear(res);
    mpfr_clear(res2);
    mpfr_clear(rounder);
    mpz_clear(b);
    mpfr_free_cache();

    return a;
}

mpz_class mpz_class_crt(const vector<mpz_class> &v, const vector<mpz_class> &m)
{
    mpz_class u,x;
    size_t s = m.size();
    vector<mpz_class> C(s);
    size_t i, j;
    
    for (i=1; i<s; i++) {
        C[i] = 1;
        for (j=0; j<i; j++) {
            mpz_class_invert(u,m[j],m[i]);
            C[i] *= u;
            C[i] %= m[i];
        }
    }
    u = v[0];
    x = u;
    
    for (i=1; i<s; i++) {
        u = v[i] - x;
        u *= C[i];
        u %= m[i];
        
        for (j=0; j<i; j++) {
            u *= m[j];
        }
        x+= u;
    }
    return x;
}



void  fastmul2(mpz_class &c, mpz_class &a, mpz_class &b)
{
    //mpz_t w,u, v;

    mpz_ptr w; mpz_srcptr u; mpz_srcptr v;
    w= c.get_mpz_t();

    u = a.get_mpz_t();
    v =  b.get_mpz_t();

    mp_size_t usize;
    mp_size_t vsize;
    mp_size_t wsize;
    mp_size_t sign_product;
    mp_ptr up, vp;
    mp_ptr wp;
    mp_ptr free_me;
    size_t free_me_size;
    mp_limb_t cy_limb;
    TMP_DECL;

    usize = SIZ (u);
    vsize = SIZ (v);
    sign_product = usize ^ vsize;
    usize = ABS (usize);
    vsize = ABS (vsize);

    mpz_init2(w, usize + vsize);



    if (usize < vsize)
    {
        MPZ_SRCPTR_SWAP (u, v);
        MP_SIZE_T_SWAP (usize, vsize);
    }

    if (vsize == 0)
    {
        SIZ (w) = 0;
        return;
    }




#ifdef HAVE_NATIVE_mpn_mul_2
    if (vsize <= 2)
    {
        wp = MPZ_REALLOC (w, usize+vsize);
        if (vsize == 1)
            cy_limb = mpn_mul_1 (wp, PTR (u), usize, PTR (v)[0]);
        else
        {
            cy_limb = mpn_mul_2 (wp, PTR (u), usize, PTR (v));
            usize++;
        }
        wp[usize] = cy_limb;
        usize += (cy_limb != 0);
        SIZ (w) = (sign_product >= 0 ? usize : -usize);
        return;
    }
#else
        if (vsize == 1)
    {
      wp = MPZ_REALLOC (w, usize+1);
      cy_limb = mpn_mul_1 (wp, PTR (u), usize, PTR (v)[0]);
      wp[usize] = cy_limb;
      usize += (cy_limb != 0);
      SIZ (w) = (sign_product >= 0 ? ((int) usize) : -(int) usize); // (int)  added by PM
      return;
    }
#endif


    TMP_MARK;
    free_me = NULL;
    up = PTR (u);
    vp = PTR (v);
    wp = PTR (w);

    /* Ensure W has space enough to store the result.  */
    wsize = usize + vsize;
    if (ALLOC (w) < wsize)
    {
        if (wp == up || wp == vp)
        {
            free_me = wp;
            free_me_size = (size_t) ALLOC (w); // (size_t) added by PM
        }
        else
            (*__gmp_free_func) (wp, (size_t) ALLOC (w) * GMP_LIMB_BYTES);

        ALLOC (w) = (int) wsize; // (int) added by PM
        wp = (mp_ptr) (*__gmp_allocate_func) ((size_t) wsize * GMP_LIMB_BYTES);
        PTR (w) = wp;
    }
    else
    {
        /* Make U and V not overlap with W.  */
        if (wp == up)
        {
            /* W and U are identical.  Allocate temporary space for U.  */
            up = TMP_ALLOC_LIMBS (usize);
            /* Is V identical too?  Keep it identical with U.  */
            if (wp == vp)
                vp = up;
            /* Copy to the temporary space.  */
            MPN_COPY (up, wp, usize);
        }
        else if (wp == vp)
        {
            /* W and V are identical.  Allocate temporary space for V.  */
            vp = TMP_ALLOC_LIMBS (vsize);
            /* Copy to the temporary space.  */
            MPN_COPY (vp, wp, vsize);
        }
    }

    if (up == vp)
    {
        mpn_sqr (wp, up, usize);
        cy_limb = wp[wsize - 1];
    }
    else
    {
        //mpn_fft_mul(wp, up, usize, vp, vsize);
        mpn_mul_basecase(wp, up, usize, vp, vsize);
    }

    wsize -= cy_limb == 0;

    SIZ (w) = sign_product < 0 ? ((int) -wsize) : (int) wsize; // (int) added by PM
    if (free_me != NULL)
        (*__gmp_free_func) (free_me, free_me_size * GMP_LIMB_BYTES);
    TMP_FREE;
        }



void  fastmul(mpz_class &c, mpz_class &a, mpz_class &b)
{
    //mpz_t w,u, v;

    mpz_ptr w; mpz_srcptr u; mpz_srcptr v;
    w= c.get_mpz_t();

    u = a.get_mpz_t();
    v =  b.get_mpz_t();

    mp_size_t usize;
    mp_size_t vsize;
    mp_size_t wsize;
    mp_size_t sign_product;
    mp_ptr up, vp;
    mp_ptr wp;
    mp_ptr free_me;
    size_t free_me_size;
    mp_limb_t cy_limb;
    TMP_DECL;

    usize = SIZ (u);
    vsize = SIZ (v);
    sign_product = usize ^ vsize;
    usize = ABS (usize);
    vsize = ABS (vsize);

    mpz_init2(w, usize + vsize);



    if (usize < vsize)
    {
        MPZ_SRCPTR_SWAP (u, v);
        MP_SIZE_T_SWAP (usize, vsize);
    }

    if (vsize == 0)
    {
        SIZ (w) = 0;
        return;
    }




#ifdef HAVE_NATIVE_mpn_mul_2
    if (vsize <= 2)
    {
        wp = MPZ_REALLOC (w, usize+vsize);
        if (vsize == 1)
            cy_limb = mpn_mul_1 (wp, PTR (u), usize, PTR (v)[0]);
        else
        {
            cy_limb = mpn_mul_2 (wp, PTR (u), usize, PTR (v));
            usize++;
        }
        wp[usize] = cy_limb;
        usize += (cy_limb != 0);
        SIZ (w) = (sign_product >= 0 ? usize : -usize);
        return;
    }
#else
        if (vsize == 1)
    {
      wp = MPZ_REALLOC (w, usize+1);
      cy_limb = mpn_mul_1 (wp, PTR (u), usize, PTR (v)[0]);
      wp[usize] = cy_limb;
      usize += (cy_limb != 0);
      SIZ (w) = (sign_product >= 0 ? ((int) usize) : -(int) usize); // (int)  added by PM
      return;
    }
#endif


    TMP_MARK;
    free_me = NULL;
    up = PTR (u);
    vp = PTR (v);
    wp = PTR (w);

    /* Ensure W has space enough to store the result.  */
    wsize = usize + vsize;
    if (ALLOC (w) < wsize)
    {
        if (wp == up || wp == vp)
        {
            free_me = wp;
            free_me_size = (size_t) ALLOC (w); // (size_t) added by PM
        }
        else
            (*__gmp_free_func) (wp, (size_t) ALLOC (w) * GMP_LIMB_BYTES);

        ALLOC (w) = (int) wsize; // (int) added by PM
        wp = (mp_ptr) (*__gmp_allocate_func) ((size_t) wsize * GMP_LIMB_BYTES);
        PTR (w) = wp;
    }
    else
    {
        /* Make U and V not overlap with W.  */
        if (wp == up)
        {
            /* W and U are identical.  Allocate temporary space for U.  */
            up = TMP_ALLOC_LIMBS (usize);
            /* Is V identical too?  Keep it identical with U.  */
            if (wp == vp)
                vp = up;
            /* Copy to the temporary space.  */
            MPN_COPY (up, wp, usize);
        }
        else if (wp == vp)
        {
            /* W and V are identical.  Allocate temporary space for V.  */
            vp = TMP_ALLOC_LIMBS (vsize);
            /* Copy to the temporary space.  */
            MPN_COPY (vp, wp, vsize);
        }
    }

    if (up == vp)
    {
        mpn_sqr (wp, up, usize);
        cy_limb = wp[wsize - 1];
    }
    else
    {
        //mpn_mul_basecase(wp, up, usize, vp, vsize);
        //mpn_mul(wp, up, usize, vp, vsize);
        mpn_fft_mul(wp, up, usize, vp, vsize);
    }

    wsize -= cy_limb == 0;

    SIZ (w) = sign_product < 0 ? ((int) -wsize) : (int) wsize; // (int) added by PM
    if (free_me != NULL)
        (*__gmp_free_func) (free_me, free_me_size * GMP_LIMB_BYTES);
    TMP_FREE;
}


void fastpow(mpz_class &d, mpz_class &a, mpz_class &b_, mpz_class &c)
{

    mpz_class r_;
    mpz_ptr r = r_.get_mpz_t();

    mpz_srcptr b; mpz_srcptr e; mpz_srcptr m;
    b = a.get_mpz_t();
    e =  b_.get_mpz_t();
    m = c.get_mpz_t();
    //cout << v << endl;

    mp_size_t n, nodd, ncnt;
    int cnt;
    mp_ptr rp, tp;
    mp_srcptr bp, ep, mp;
    mp_size_t rn, bn, es, en, itch;
    mpz_t new_b;			/* note: value lives long via 'b' */
    TMP_DECL;

    n = ABSIZ(m);
    if (UNLIKELY (n == 0))
        DIVIDE_BY_ZERO;

    mp = PTR(m);

    TMP_MARK;


    es = SIZ(e);
    if (UNLIKELY (es <= 0))
    {
        if (es == 0)
        {
            /* b^0 mod m,  b is anything and m is non-zero.
               Result is 1 mod m, i.e., 1 or 0 depending on if m = 1.  */
            SIZ(r) = n != 1 || mp[0] != 1;
            MPZ_NEWALLOC (r, 1)[0] = 1;
            TMP_FREE;	/* we haven't really allocated anything here */
            return ;//mpz_class(r);
        }
#if HANDLE_NEGATIVE_EXPONENT
MPZ_TMP_INIT (new_b, n + 1);
      if (UNLIKELY (! mpz_invert (new_b, b, m)))
	DIVIDE_BY_ZERO;
      b = new_b;
      es = -es;
#else
        DIVIDE_BY_ZERO;
#endif
    }
    en = es;

    bn = ABSIZ(b);

    if (UNLIKELY (bn == 0))
    {
        SIZ(r) = 0;
        TMP_FREE;
        return;// mpz_class(r);
    }

    ep = PTR(e);

    /* Handle (b^1 mod m) early, since mpn_pow* do not handle that case.  */
    if (UNLIKELY (en == 1 && ep[0] == 1))
    {
        rp = TMP_ALLOC_LIMBS (n);
        bp = PTR(b);
        if (bn >= n)
        {
            mp_ptr qp = TMP_ALLOC_LIMBS (bn - n + 1);
            mpn_tdiv_qr (qp, rp, 0L, bp, bn, mp, n);
            rn = n;
            MPN_NORMALIZE (rp, rn);

            if (rn != 0 && SIZ(b) < 0)
            {
                mpn_sub (rp, mp, n, rp, rn);
                rn = n;
                MPN_NORMALIZE_NOT_ZERO (rp, rn);
            }
        }

        else
        {
            if (SIZ(b) < 0)
            {
                mpn_sub (rp, mp, n, bp, bn);
                rn = n;
                MPN_NORMALIZE_NOT_ZERO (rp, rn);
            }
            else
            {
                MPN_COPY (rp, bp, bn);
                rn = bn;
            }
        }
        goto ret;
    }

    /* Remove low zero limbs from M.  This loop will terminate for correctly
    represented mpz numbers.  */
    ncnt = 0;
    while (UNLIKELY (mp[0] == 0))
    {
        mp++;
        ncnt++;
    }
    nodd = n - ncnt;
    cnt = 0;
    if (mp[0] % 2 == 0)
    {
        mp_ptr newmp = TMP_ALLOC_LIMBS (nodd);
        count_trailing_zeros (cnt, mp[0]);
        mpn_rshift (newmp, mp, nodd, cnt);
        nodd -= newmp[nodd - 1] == 0;
        mp = newmp;
        ncnt++;
    }

    if (ncnt != 0)
    {
        /* We will call both mpn_powm and mpn_powlo.  */
        /* rp needs n, mpn_powlo needs 4n, the 2 mpn_binvert might need more */
        mp_size_t n_largest_binvert = MAX (ncnt, nodd);
        mp_size_t itch_binvert = mpn_binvert_itch (n_largest_binvert);
        itch = 3 * n + MAX (itch_binvert, 2 * n);
    }
    else
    {
        /* We will call just mpn_powm.  */
        mp_size_t itch_binvert = mpn_binvert_itch (nodd);
        itch = n + MAX (itch_binvert, 2 * n);
    }

    tp = TMP_ALLOC_LIMBS (itch);

    rp = tp;  tp += n;

    bp = PTR(b);
    mpn_powm (rp, bp, bn, ep, en, mp, nodd, tp);

    rn = n;

    if (ncnt != 0)
    {
        mp_ptr r2, xp, yp, odd_inv_2exp;
        unsigned long t;
        int bcnt;

        if (bn < ncnt)
        {
            mp_ptr newbp = TMP_ALLOC_LIMBS (ncnt);
            MPN_COPY (newbp, bp, bn);
            MPN_ZERO (newbp + bn, ncnt - bn);
            bp = newbp;
        }

        r2 = tp;

        if (bp[0] % 2 == 0)
        {
            if (en > 1)
            {
                MPN_ZERO (r2, ncnt);
                goto zero;
            }

            ASSERT (en == 1);
            t = (ncnt - (cnt != 0)) * GMP_NUMB_BITS + cnt;

            /* Count number of low zero bits in B, up to 3.  */
            bcnt = (0x1213 >> ((bp[0] & 7) << 1)) & 0x3;
            /* Note that ep[0] * bcnt might overflow, but that just results
               in a missed optimization.  */
            if (ep[0] * bcnt >= t)
            {
                MPN_ZERO (r2, ncnt);
                goto zero;
            }
        }

        mpn_powlo (r2, bp, ep, en, ncnt, tp + ncnt);

        zero:
        if (nodd < ncnt)
        {
            mp_ptr newmp = TMP_ALLOC_LIMBS (ncnt);
            MPN_COPY (newmp, mp, nodd);
            MPN_ZERO (newmp + nodd, ncnt - nodd);
            mp = newmp;
        }

        odd_inv_2exp = tp + n;
        mpn_binvert (odd_inv_2exp, mp, ncnt, tp + 2 * n);

        mpn_sub (r2, r2, ncnt, rp, nodd > ncnt ? ncnt : nodd);

        xp = tp + 2 * n;
        mpn_mullo_n (xp, odd_inv_2exp, r2, ncnt);

        if (cnt != 0)
            xp[ncnt - 1] &= (CNST_LIMB(1) << cnt) - 1;

        yp = tp;
        if (ncnt > nodd)
            mpn_mul (yp, xp, ncnt, mp, nodd);
        else
            mpn_mul (yp, mp, nodd, xp, ncnt);

        mpn_add (rp, yp, n, rp, nodd);

        ASSERT (nodd + ncnt >= n);
        ASSERT (nodd + ncnt <= n + 1);
    }


    MPN_NORMALIZE (rp, rn);

    if ((ep[0] & 1) && SIZ(b) < 0 && rn != 0)
    {
        mpn_sub (rp, PTR(m), n, rp, rn);
        rn = n;
        MPN_NORMALIZE (rp, rn);
    }

    ret:
        MPZ_NEWALLOC (r, rn);
        SIZ(r) = rn;
        MPN_COPY (PTR(r), rp, rn);

        TMP_FREE;


    d = mpz_class(r);

    //return d;
}



void fastpow2(mpz_class &d, mpz_class &a, mpz_class &b_, mpz_class &c)
{

    mpz_class r_;
    mpz_ptr r = r_.get_mpz_t();

    mpz_srcptr b; mpz_srcptr e; mpz_srcptr m;
    b = a.get_mpz_t();
    e =  b_.get_mpz_t();
    m = c.get_mpz_t();
    //cout << v << endl;

    mp_size_t n, nodd, ncnt;
    int cnt;
    mp_ptr rp, tp;
    mp_srcptr bp, ep, mp;
    mp_size_t rn, bn, es, en, itch;
    mpz_t new_b;			/* note: value lives long via 'b' */
    TMP_DECL;

    n = ABSIZ(m);
    if (UNLIKELY (n == 0))
        DIVIDE_BY_ZERO;

    mp = PTR(m);

    TMP_MARK;


    es = SIZ(e);
    if (UNLIKELY (es <= 0))
    {
        if (es == 0)
        {
            /* b^0 mod m,  b is anything and m is non-zero.
               Result is 1 mod m, i.e., 1 or 0 depending on if m = 1.  */
            SIZ(r) = n != 1 || mp[0] != 1;
            MPZ_NEWALLOC (r, 1)[0] = 1;
            TMP_FREE;	/* we haven't really allocated anything here */
            return ;//mpz_class(r);
        }
#if HANDLE_NEGATIVE_EXPONENT
        MPZ_TMP_INIT (new_b, n + 1);
        if (UNLIKELY (! mpz_invert (new_b, b, m)))
            DIVIDE_BY_ZERO;
        b = new_b;
        es = -es;
#else
        DIVIDE_BY_ZERO;
#endif
    }
    en = es;

    bn = ABSIZ(b);

    if (UNLIKELY (bn == 0))
    {
        SIZ(r) = 0;
        TMP_FREE;
        return;// mpz_class(r);
    }

    ep = PTR(e);

    /* Handle (b^1 mod m) early, since mpn_pow* do not handle that case.  */
    if (UNLIKELY (en == 1 && ep[0] == 1))
    {
        rp = TMP_ALLOC_LIMBS (n);
        bp = PTR(b);
        if (bn >= n)
        {
            mp_ptr qp = TMP_ALLOC_LIMBS (bn - n + 1);
            mpn_tdiv_qr (qp, rp, 0L, bp, bn, mp, n);
            rn = n;
            MPN_NORMALIZE (rp, rn);

            if (rn != 0 && SIZ(b) < 0)
            {
                mpn_sub (rp, mp, n, rp, rn);
                rn = n;
                MPN_NORMALIZE_NOT_ZERO (rp, rn);
            }
        }

        else
        {
            if (SIZ(b) < 0)
            {
                mpn_sub (rp, mp, n, bp, bn);
                rn = n;
                MPN_NORMALIZE_NOT_ZERO (rp, rn);
            }
            else
            {
                MPN_COPY (rp, bp, bn);
                rn = bn;
            }
        }
        goto ret;
    }

    /* Remove low zero limbs from M.  This loop will terminate for correctly
    represented mpz numbers.  */
    ncnt = 0;
    while (UNLIKELY (mp[0] == 0))
    {
        mp++;
        ncnt++;
    }
    nodd = n - ncnt;
    cnt = 0;
    if (mp[0] % 2 == 0)
    {
        mp_ptr newmp = TMP_ALLOC_LIMBS (nodd);
        count_trailing_zeros (cnt, mp[0]);
        mpn_rshift (newmp, mp, nodd, cnt);
        nodd -= newmp[nodd - 1] == 0;
        mp = newmp;
        ncnt++;
    }

    if (ncnt != 0)
    {
        /* We will call both mpn_powm and mpn_powlo.  */
        /* rp needs n, mpn_powlo needs 4n, the 2 mpn_binvert might need more */
        mp_size_t n_largest_binvert = MAX (ncnt, nodd);
        mp_size_t itch_binvert = mpn_binvert_itch (n_largest_binvert);
        itch = 3 * n + MAX (itch_binvert, 2 * n);
    }
    else
    {
        /* We will call just mpn_powm.  */
        mp_size_t itch_binvert = mpn_binvert_itch (nodd);
        itch = n + MAX (itch_binvert, 2 * n);
    }

    tp = TMP_ALLOC_LIMBS (itch);

    rp = tp;  tp += n;

    bp = PTR(b);
    mpn_powm (rp, bp, bn, ep, en, mp, nodd, tp);

    rn = n;

    if (ncnt != 0)
    {
        mp_ptr r2, xp, yp, odd_inv_2exp;
        unsigned long t;
        int bcnt;

        if (bn < ncnt)
        {
            mp_ptr newbp = TMP_ALLOC_LIMBS (ncnt);
            MPN_COPY (newbp, bp, bn);
            MPN_ZERO (newbp + bn, ncnt - bn);
            bp = newbp;
        }

        r2 = tp;

        if (bp[0] % 2 == 0)
        {
            if (en > 1)
            {
                MPN_ZERO (r2, ncnt);
                goto zero;
            }

            ASSERT (en == 1);
            t = (ncnt - (cnt != 0)) * GMP_NUMB_BITS + cnt;

            /* Count number of low zero bits in B, up to 3.  */
            bcnt = (0x1213 >> ((bp[0] & 7) << 1)) & 0x3;
            /* Note that ep[0] * bcnt might overflow, but that just results
               in a missed optimization.  */
            if (ep[0] * bcnt >= t)
            {
                MPN_ZERO (r2, ncnt);
                goto zero;
            }
        }

        mpn_powlo (r2, bp, ep, en, ncnt, tp + ncnt);

        zero:
        if (nodd < ncnt)
        {
            mp_ptr newmp = TMP_ALLOC_LIMBS (ncnt);
            MPN_COPY (newmp, mp, nodd);
            MPN_ZERO (newmp + nodd, ncnt - nodd);
            mp = newmp;
        }

        odd_inv_2exp = tp + n;
        mpn_binvert (odd_inv_2exp, mp, ncnt, tp + 2 * n);

        mpn_sub (r2, r2, ncnt, rp, nodd > ncnt ? ncnt : nodd);

        xp = tp + 2 * n;
        mpn_mullo_n (xp, odd_inv_2exp, r2, ncnt);

        if (cnt != 0)
            xp[ncnt - 1] &= (CNST_LIMB(1) << cnt) - 1;

        yp = tp;
        if (ncnt > nodd)
            mpn_fft_mul(yp, xp, ncnt, mp, nodd);
        else
            mpn_fft_mul (yp, mp, nodd, xp, ncnt);

        mpn_add (rp, yp, n, rp, nodd);

        ASSERT (nodd + ncnt >= n);
        ASSERT (nodd + ncnt <= n + 1);
    }


    MPN_NORMALIZE (rp, rn);

    if ((ep[0] & 1) && SIZ(b) < 0 && rn != 0)
    {
        mpn_sub (rp, PTR(m), n, rp, rn);
        rn = n;
        MPN_NORMALIZE (rp, rn);
    }

    ret:
    MPZ_NEWALLOC (r, rn);
    SIZ(r) = rn;
    MPN_COPY (PTR(r), rp, rn);

    TMP_FREE;


    d = mpz_class(r);

    //return d;
}




                FixedPointExp::FixedPointExp(mpz_t& g, mpz_t& p, int fieldsize)
{
    mpz_init(m_g);
    mpz_init(m_p);
    mpz_set(m_g, g);
    mpz_set(m_p, p);
    
    
    m_isInitialized = false;
    m_numberOfElements = fieldsize;
    m_table = NULL;
    init();
}

FixedPointExp::~FixedPointExp() {
    if (m_isInitialized) {
        delete[] m_table;
    }
}

void FixedPointExp::init() {
    
    m_table = (mpz_t*) malloc(sizeof(mpz_t) * m_numberOfElements);
    for(int i = 0; i < m_numberOfElements; i++)
    {
        mpz_init(m_table[i]);
    }
    
    // m_table[0] = m_g;
    mpz_set(m_table[0], m_g);
    for (unsigned u=1; u<m_numberOfElements; ++u) {
        mpz_mul(m_table[u], m_table[u-1], m_table[u-1]);
        mpz_mod(m_table[u], m_table[u], m_p);
        //mpz_powm_ui(m_table[u], m_table[u-1], 2, m_p);
        //SqrMod(m_table[u], m_table[u-1], m_p);
    }
    m_isInitialized = true;
    
    //   for (unsigned u=0; u<m_numberOfElements; ++u) {
    //     cout << "table[" << u << "] = " << m_table[u] << endl;
    //     ZZ res;
    //     ZZ ex = power_ZZ(2,u);
    //     PowerMod(res, m_g, ex, m_p);
    //     cout << "    (Should be = " << res << ")" << endl;
    //   }
}

void FixedPointExp::powerMod(mpz_t& result, mpz_t& e) {
    mpz_set_ui(result, 1);
    for (unsigned u=0; u<m_numberOfElements; u++) {
        //if (bit(e,u)) {
        if(mpz_tstbit(e, u))
        {
            mpz_mul(result, result, m_table[u]);
            mpz_mod(result, result, m_p);
        }
        //MulMod(result, result, m_table[u], m_p);
    }
}


