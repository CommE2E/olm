/* Copyright 2015 OpenMarket Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "olm/pickle.hh"


std::size_t olm::pickle_length(
    const olm::Curve25519PublicKey & value
) {
    return sizeof(value.public_key);
}


std::uint8_t * olm::pickle(
    std::uint8_t * pos,
    const olm::Curve25519PublicKey & value
) {
    pos = olm::pickle_bytes(
        pos, value.public_key, sizeof(value.public_key)
    );
    return pos;
}


std::uint8_t const * olm::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::Curve25519PublicKey & value
) {
    pos = olm::unpickle_bytes(
        pos, end, value.public_key, sizeof(value.public_key)
    );
    return pos;

}


std::size_t olm::pickle_length(
    const olm::Curve25519KeyPair & value
) {
    return sizeof(value.public_key) + sizeof(value.private_key);
}


std::uint8_t * olm::pickle(
    std::uint8_t * pos,
    const olm::Curve25519KeyPair & value
) {
    pos = olm::pickle_bytes(
        pos, value.public_key, sizeof(value.public_key)
    );
    pos = olm::pickle_bytes(
        pos, value.private_key, sizeof(value.private_key)
    );
    return pos;
}


std::uint8_t const * olm::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::Curve25519KeyPair & value
) {
    pos = olm::unpickle_bytes(
        pos, end, value.public_key, sizeof(value.public_key)
    );
    pos = olm::unpickle_bytes(
        pos, end, value.private_key, sizeof(value.private_key)
    );
    return pos;
}
