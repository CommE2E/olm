#ifndef ERROR_HH_
#define ERROR_HH_

namespace axolotl {

enum struct ErrorCode {
    SUCCESS = 0, /*!< There wasn't an error */
    NOT_ENOUGH_RANDOM = 1,  /*!< Not enough entropy was supplied */
    OUTPUT_BUFFER_TOO_SMALL = 2, /*!< Supplied output buffer is too small */
    BAD_MESSAGE_VERSION = 3,  /*!< The message version is unsupported */
    BAD_MESSAGE_FORMAT = 4, /*!< The message couldn't be decoded */
    BAD_MESSAGE_MAC = 5, /*!< The message couldn't be decrypted */
};

} // namespace axolotl

#endif /* ERROR_HH_ */
