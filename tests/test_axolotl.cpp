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
#include "axolotl/axolotl.hh"
#include "unittest.hh"


int main() {

{ /* Loopback test case */
TestCase test_case("Axolotl Loopback 1");

std::uint8_t root_info[] = "Axolotl";
std::uint8_t ratchet_info[] = "AxolotlRatchet";
std::uint8_t message_info[] = "AxolotlMessageKeys";

axolotl::KdfInfo kdf_info = {
    root_info, sizeof(root_info) - 1,
    ratchet_info, sizeof(ratchet_info - 1),
    message_info, sizeof(ratchet_info - 1)
};

axolotl::Session alice(kdf_info);
axolotl::Session bob(kdf_info);

std::uint8_t random_bytes[] = "0123456789ABDEF0123456789ABCDEF";
axolotl::Curve25519KeyPair bob_key;
axolotl::generate_key(random_bytes, bob_key);

std::uint8_t shared_secret[] = "A secret";

alice.initialise_as_bob(shared_secret, sizeof(shared_secret) - 1, bob_key);
bob.initialise_as_alice(shared_secret, sizeof(shared_secret) - 1, bob_key);

std::uint8_t plaintext[] = "Message";
std::size_t plaintext_length = sizeof(plaintext) - 1;

std::size_t message_length, random_length, actual_length;
std::size_t encrypt_length, decrypt_length;

message_length = bob.encrypt_max_output_length(plaintext_length);
random_length = bob.encrypt_random_length();
assert_equals(std::size_t(0), random_length);
actual_length = alice.decrypt_max_plaintext_length(message_length);
{
    std::uint8_t message[message_length];
    std::uint8_t actual[actual_length];

    encrypt_length = bob.encrypt(
        plaintext, plaintext_length,
        NULL, 0,
        message, message_length
    );
    assert_equals(message_length, encrypt_length);

    decrypt_length = alice.decrypt(
        message, message_length,
        actual, actual_length
    );
    assert_equals(plaintext_length, decrypt_length);
    assert_equals(plaintext, actual, decrypt_length);
}

} /* Loopback test case */

}
