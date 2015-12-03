#ifndef ED25519_ADDITIONS_H
#define ED25519_ADDITIONS_H

#ifdef __cplusplus
extern "C" {
#endif

void convert_curve25519_to_ed25519(
    unsigned char * public_key,
    unsigned char * signature);

void convert_ed25519_to_curve25519(
    unsigned char const * public_key,
    unsigned char * signature);

void ed25519_keypair(
    unsigned char * private_key,
    unsigned char * public_key);

#ifdef __cplusplus
}
#endif

#endif
