#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>

namespace utilities{

    bool checkFileExists(std::string const &filename);

    int random(void *);

}

#if defined(Q_OS_WINRT) || defined(Q_OS_WIN32)
struct rsa_st{
    /* The first parameter is used to pickup errors where
     * this is passed instead of aEVP_PKEY, it is set to 0 */
    int pad;
    long version;
    const RSA_METHOD *meth;
    /* functional reference if 'meth' is ENGINE-provided */
    ENGINE *engine;
    BIGNUM *n;
    BIGNUM *e;
    BIGNUM *d;
    BIGNUM *p;
    BIGNUM *q;
    BIGNUM *dmp1;
    BIGNUM *dmq1;
    BIGNUM *iqmp;
    /* be careful using this if the RSA structure is shared */
    CRYPTO_EX_DATA ex_data;
    int references;
    int flags;

    /* Used to cache montgomery values */
    BN_MONT_CTX *_method_mod_n;
    BN_MONT_CTX *_method_mod_p;
    BN_MONT_CTX *_method_mod_q;

    /* all BIGNUM values are actually in the following data, if it is not
     * NULL */
    char *bignum_data;
    BN_BLINDING *blinding;
    BN_BLINDING *mt_blinding;
};

#endif

#endif // UTILITIES_H
