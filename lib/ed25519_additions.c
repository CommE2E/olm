void convert_curve25519_to_ed25519(
    unsigned char * public_key,
    unsigned char * signature
) {
    fe mont_x, mont_x_minus_one, mont_x_plus_one, inv_mont_x_plus_one;
    fe one;
    fe ed_y;

    fe_frombytes(mont_x, public_key);
    fe_1(one);
    fe_sub(mont_x_minus_one, mont_x, one);
    fe_add(mont_x_plus_one, mont_x, one);
    fe_invert(inv_mont_x_plus_one, mont_x_plus_one);
    fe_mul(ed_y, mont_x_minus_one, inv_mont_x_plus_one);
    fe_tobytes(public_key, ed_y);

    public_key[31] &= 0x7F;
    public_key[31] |= (signature[63] & 0x80);
    signature[63] &= 0x7F;
}


void convert_ed25519_to_curve25519(
    unsigned char const * public_key,
    unsigned char * signature
) {
    unsigned char sign_bit = public_key[31] & 0x80;
    signature[63] &= 0x7F;
    signature[63] |= sign_bit;
}


void ed25519_keypair(
    unsigned char * private_key,
    unsigned char * public_key
) {
    ge_p3 A;
    private_key[0] &= 248;
    private_key[31] &= 63;
    private_key[31] |= 64;
    ge_scalarmult_base(&A, private_key);
    ge_p3_tobytes(public_key, &A);
}
