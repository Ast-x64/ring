/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.] */

#include <openssl/bn.h>

#include <assert.h>
#include <string.h>

#include "internal.h"


#define BN_MUL_RECURSIVE_SIZE_NORMAL 16
#define BN_SQR_RECURSIVE_SIZE_NORMAL BN_MUL_RECURSIVE_SIZE_NORMAL


static void bn_mul_normal(BN_ULONG *r, BN_ULONG *a, int na, BN_ULONG *b,
                          int nb) {
  BN_ULONG *rr;

  if (na < nb) {
    int itmp;
    BN_ULONG *ltmp;

    itmp = na;
    na = nb;
    nb = itmp;
    ltmp = a;
    a = b;
    b = ltmp;
  }
  rr = &(r[na]);
  if (nb <= 0) {
    (void)bn_mul_words(r, a, na, 0);
    return;
  } else {
    rr[0] = bn_mul_words(r, a, na, b[0]);
  }

  for (;;) {
    if (--nb <= 0) {
      return;
    }
    rr[1] = bn_mul_add_words(&(r[1]), a, na, b[1]);
    if (--nb <= 0) {
      return;
    }
    rr[2] = bn_mul_add_words(&(r[2]), a, na, b[2]);
    if (--nb <= 0) {
      return;
    }
    rr[3] = bn_mul_add_words(&(r[3]), a, na, b[3]);
    if (--nb <= 0) {
      return;
    }
    rr[4] = bn_mul_add_words(&(r[4]), a, na, b[4]);
    rr += 4;
    r += 4;
    b += 4;
  }
}

int BN_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx) {
  int ret = 0;
  int top, al, bl;
  BIGNUM *rr;

  al = a->top;
  bl = b->top;

  if ((al == 0) || (bl == 0)) {
    BN_zero(r);
    return 1;
  }
  top = al + bl;

  BN_CTX_start(ctx);
  if ((r == a) || (r == b)) {
    if ((rr = BN_CTX_get(ctx)) == NULL) {
      goto err;
    }
  } else {
    rr = r;
  }
  rr->neg = a->neg ^ b->neg;

  if (bn_wexpand(rr, top) == NULL) {
    goto err;
  }
  rr->top = top;
  bn_mul_normal(rr->d, a->d, al, b->d, bl);

  bn_correct_top(rr);
  if (r != rr && !BN_copy(r, rr)) {
    goto err;
  }
  ret = 1;

err:
  BN_CTX_end(ctx);
  return ret;
}
