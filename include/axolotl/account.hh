#ifndef AXOLOTL_ACCOUNT_HH_
#define AXOLOTL_ACCOUNT_HH_

#include "axolotl/list.hh"
#include "axolotl/crypto.hh"
#include "axolotl/error.hh"

#include <cstdint>

namespace axolotl {


struct LocalKey {
    std::uint32_t id;
    Curve25519KeyPair key;
};


struct SignedKey : LocalKey {
    std::uint8_t signature[64];
};


static std::size_t const MAX_ONE_TIME_KEYS = 100;

struct Account {
    LocalKey identity_key;
    LocalKey last_resort_one_time_key;
    List<LocalKey, MAX_ONE_TIME_KEYS> one_time_keys;
    ErrorCode last_error;

    /** Number of random bytes needed to create a new account */
    std::size_t new_account_random_length();

    /** Create a new account. Returns NOT_ENOUGH_RANDOM if the number of random
     * bytes is too small. */
    std::size_t new_account(
        uint8_t const * random, std::size_t random_length
    );

    LocalKey const * lookup_key(
        std::uint32_t id
    );

    /** The number of bytes needed to persist this account. */
    std::size_t pickle_length();

    /** Persists an account as a sequence of bytes
     * Returns the number of output bytes used. */
    std::size_t pickle(
        std::uint8_t * output, std::size_t output_length
    );

    /** Loads an account from a sequence of bytes.
     * Returns 0 on success, or std::size_t(-1) on failure. */
    std::size_t unpickle(
        std::uint8_t * input, std::size_t input_length
    );
};


} // namespace axolotl

#endif /* AXOLOTL_ACCOUNT_HH_ */
