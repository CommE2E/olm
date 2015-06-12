#include "axolotl/account.hh"
#include "axolotl/pickle.hh"



axolotl::LocalKey const * axolotl::Account::lookup_key(
    std::uint32_t id
) {
    for (axolotl::LocalKey const & key : one_time_keys) {
        if (key.id == id) return &key;
    }
    return 0;
}

namespace axolotl {


static std::size_t pickle_length(
    axolotl::LocalKey const & value
) {
    return axolotl::pickle_length(value.id) + axolotl::pickle_length(value.key);
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    axolotl::LocalKey const & value
) {
    pos = axolotl::pickle(pos, value.id);
    pos = axolotl::pickle(pos, value.key);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::LocalKey & value
) {
    pos = axolotl::unpickle(pos, end, value.id);
    pos = axolotl::unpickle(pos, end, value.key);
    return pos;
}


static std::size_t pickle_length(
    axolotl::SignedKey const & value
) {
    return axolotl::pickle_length((axolotl::LocalKey const &) value) + 64;
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    axolotl::SignedKey const & value
) {
    pos = axolotl::pickle(pos, (axolotl::LocalKey const &) value);
    pos = axolotl::pickle_bytes(pos, value.signature, 64);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::SignedKey & value
) {
    pos = axolotl::unpickle(pos, end, (axolotl::LocalKey &) value);
    pos = axolotl::unpickle_bytes(pos, end, value.signature, 64);
    return pos;
}


} // namespace axolotl


std::size_t pickle_length(
    axolotl::Account const & value
) {
    std::size_t length = 0;
    length += axolotl::pickle_length(value.identity_key);
    length += axolotl::pickle_length(value.last_resort_one_time_key);
    length += axolotl::pickle_length(value.one_time_keys);
    return length;
}


std::uint8_t * pickle(
    std::uint8_t * pos,
    axolotl::Account const & value
) {
    pos = axolotl::pickle(pos, value.identity_key);
    pos = axolotl::pickle(pos, value.last_resort_one_time_key);
    pos = axolotl::pickle(pos, value.one_time_keys);
    return pos;
}


std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::Account & value
) {
    pos = axolotl::unpickle(pos, end, value.identity_key);
    pos = axolotl::unpickle(pos, end, value.last_resort_one_time_key);
    pos = axolotl::unpickle(pos, end, value.one_time_keys);
    return pos;
}
