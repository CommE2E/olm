//
//  OLMSession.m
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import "OLMSession.h"
#import "OLMUtility.h"
#import "OLMAccount_Private.h"
@import olm;

@interface OLMSession()
@property (nonatomic) OlmSession *session;
@end

@implementation OLMSession

- (void) dealloc {
    olm_clear_session(_session);
    free(_session);
}

- (BOOL) initializeSessionMemory {
    size_t size = olm_session_size();
    _session = malloc(size);
    NSParameterAssert(_session != nil);
    if (!_session) {
        return NO;
    }
    _session = olm_session(_session);
    NSParameterAssert(_session != nil);
    if (!_session) {
        return NO;
    }
    return YES;
}

- (instancetype) initWithAccount:(OLMAccount*)account {
    self = [super init];
    if (!self) {
        return nil;
    }
    BOOL success = [self initializeSessionMemory];
    if (!success) {
        return nil;
    }
    _account = account;
    return self;
}

- (instancetype) initOutboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSString*)theirIdentityKey theirOneTimeKey:(NSString*)theirOneTimeKey {
    self = [self initWithAccount:account];
    if (!self) {
        return nil;
    }
    NSMutableData *random = [OLMUtility randomBytesOfLength:olm_create_outbound_session_random_length(_session)];
    NSData *idKey = [theirIdentityKey dataUsingEncoding:NSUTF8StringEncoding];
    NSData *otKey = [theirOneTimeKey dataUsingEncoding:NSUTF8StringEncoding];
    size_t result = olm_create_outbound_session(_session, account.account, idKey.bytes, idKey.length, otKey.bytes, otKey.length, random.mutableBytes, random.length);
    if (result == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_create_outbound_session error: %s", error);
        return nil;
    }
    return self;
}

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account oneTimeKeyMessage:(NSString*)oneTimeKeyMessage {
    self = [self initWithAccount:account];
    if (!self) {
        return nil;
    }
    BOOL success = [self initializeSessionMemory];
    if (!success) {
        return nil;
    }
    NSMutableData *otk = [NSMutableData dataWithData:[oneTimeKeyMessage dataUsingEncoding:NSUTF8StringEncoding]];
    size_t result = olm_create_inbound_session(_session, account.account, otk.mutableBytes, oneTimeKeyMessage.length);
    if (result == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_create_inbound_session error: %s", error);
        return nil;
    }
    return self;
}

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSString*)theirIdentityKey oneTimeKeyMessage:(NSString*)oneTimeKeyMessage {
    self = [self initWithAccount:account];
    if (!self) {
        return nil;
    }
    BOOL success = [self initializeSessionMemory];
    if (!success) {
        return nil;
    }
    NSData *idKey = [theirIdentityKey dataUsingEncoding:NSUTF8StringEncoding];
    NSMutableData *otk = [NSMutableData dataWithData:[oneTimeKeyMessage dataUsingEncoding:NSUTF8StringEncoding]];
    size_t result = olm_create_inbound_session_from(_session, account.account, idKey.bytes, idKey.length, otk.mutableBytes, otk.length);
    if (result == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_create_inbound_session_from error: %s", error);
        return nil;
    }
    return self;
}

- (NSString*) sessionIdentifier {
    size_t length = olm_session_id_length(_session);
    NSMutableData *idData = [NSMutableData dataWithLength:length];
    if (!idData) {
        return nil;
    }
    size_t result = olm_session_id(_session, idData.mutableBytes, idData.length);
    if (result == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_session_id error: %s", error);
        return nil;
    }
    NSString *idString = [[NSString alloc] initWithData:idData encoding:NSUTF8StringEncoding];
    return idString;
}

- (OLMMessage*) encryptMessage:(NSString*)message {
    size_t messageType = olm_encrypt_message_type(_session);
    size_t randomLength = olm_encrypt_random_length(_session);
    NSMutableData *random = [OLMUtility randomBytesOfLength:randomLength];
    NSData *plaintextData = [message dataUsingEncoding:NSUTF8StringEncoding];
    size_t ciphertextLength = olm_encrypt_message_length(_session, plaintextData.length);
    NSMutableData *ciphertext = [NSMutableData dataWithLength:ciphertextLength];
    if (!ciphertext) {
        return nil;
    }
    size_t result = olm_encrypt(_session, plaintextData.bytes, plaintextData.length, random.mutableBytes, random.length, ciphertext.mutableBytes, ciphertext.length);
    if (result == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_encrypt error: %s", error);
        return nil;
    }
    NSString *ciphertextString = [[NSString alloc] initWithData:ciphertext encoding:NSUTF8StringEncoding];
    OLMMessage *encryptedMessage = [[OLMMessage alloc] initWithCiphertext:ciphertextString type:messageType];
    return encryptedMessage;
}

- (BOOL) removeOneTimeKeys {
    size_t result = olm_remove_one_time_keys(_account.account, _session);
    if (result == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_remove_one_time_keys error: %s", error);
        return NO;
    }
    return YES;
}

- (NSString*) decryptMessage:(OLMMessage*)message {
    NSParameterAssert(message != nil);
    NSData *messageData = [message.ciphertext dataUsingEncoding:NSUTF8StringEncoding];
    if (!messageData) {
        return nil;
    }
    NSMutableData *mutMessage = messageData.mutableCopy;
    size_t maxPlaintextLength = olm_decrypt_max_plaintext_length(_session, message.type, mutMessage.mutableBytes, mutMessage.length);
    if (maxPlaintextLength == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_decrypt_max_plaintext_length error: %s", error);
        return nil;
    }
    // message buffer is destroyed by olm_decrypt_max_plaintext_length
    mutMessage = messageData.mutableCopy;
    NSMutableData *plaintextData = [NSMutableData dataWithLength:maxPlaintextLength];
    size_t plaintextLength = olm_decrypt(_session, message.type, mutMessage.mutableBytes, mutMessage.length, plaintextData.mutableBytes, plaintextData.length);
    if (plaintextLength == olm_error()) {
        const char *error = olm_session_last_error(_session);
        NSAssert(NO, @"olm_decrypt error: %s", error);
        return nil;
    }
    plaintextData.length = plaintextLength;
    NSString *plaintext = [[NSString alloc] initWithData:plaintextData encoding:NSUTF8StringEncoding];
    return plaintext;
}

@end
