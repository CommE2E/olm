//
//  OLMAccount.m
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import "OLMAccount.h"
#import "OLMAccount_Private.h"
#import "OLMUtility.h"

@import Security;

@implementation OLMAccount

- (void) dealloc {
    olm_clear_account(_account);
    free(_account);
}

- (BOOL) initializeAccountMemory {
    size_t accountSize = olm_account_size();
    _account = malloc(accountSize);
    NSParameterAssert(_account != nil);
    if (!_account) {
        return NO;
    }
    _account = olm_account(_account);
    NSParameterAssert(_account != nil);
    if (!_account) {
        return NO;
    }
    return YES;
}

- (instancetype) initNewAccount {
    self = [super init];
    if (!self) {
        return nil;
    }
    BOOL success = [self initializeAccountMemory];
    if (!success) {
        return nil;
    }
    size_t randomLength = olm_create_account_random_length(_account);
    NSMutableData *random = [OLMUtility randomBytesOfLength:randomLength];
    size_t accountResult = olm_create_account(_account, random.mutableBytes, random.length);
    if (accountResult == olm_error()) {
        const char *error = olm_account_last_error(_account);
        NSLog(@"error creating account: %s", error);
        return nil;
    }
    return self;
}

- (size_t) maxOneTimeKeys {
    return olm_account_max_number_of_one_time_keys(_account);
}


/** public identity keys */
- (NSDictionary*) identityKeys {
    size_t identityKeysLength = olm_account_identity_keys_length(_account);
    uint8_t *identityKeysBytes = malloc(identityKeysLength);
    if (!identityKeysBytes) {
        return nil;
    }
    size_t result = olm_account_identity_keys(_account, identityKeysBytes, identityKeysLength);
    if (result == olm_error()) {
        const char *error = olm_account_last_error(_account);
        NSLog(@"error getting id keys: %s", error);
        free(identityKeysBytes);
        return nil;
    }
    NSData *idKeyData = [NSData dataWithBytesNoCopy:identityKeysBytes length:identityKeysLength freeWhenDone:YES];
    NSError *error = nil;
    NSDictionary *keysDictionary = [NSJSONSerialization JSONObjectWithData:idKeyData options:0 error:&error];
    if (error) {
        NSLog(@"Could not decode JSON: %@", error.localizedDescription);
    }
    return keysDictionary;
}

- (NSDictionary*) oneTimeKeys {
    size_t otkLength = olm_account_one_time_keys_length(_account);
    uint8_t *otkBytes = malloc(otkLength);
    if (!otkBytes) {
        return nil;
    }
    size_t result = olm_account_one_time_keys(_account, otkBytes, otkLength);
    if (result == olm_error()) {
        const char *error = olm_account_last_error(_account);
        NSLog(@"error getting id keys: %s", error);
        free(otkBytes);
    }
    NSData *otk = [NSData dataWithBytesNoCopy:otkBytes length:otkLength freeWhenDone:YES];
    NSError *error = nil;
    NSDictionary *keysDictionary = [NSJSONSerialization JSONObjectWithData:otk options:0 error:&error];
    if (error) {
        NSLog(@"Could not decode JSON: %@", error.localizedDescription);
    }
    return keysDictionary;
}


- (void) generateOneTimeKeys:(NSUInteger)numberOfKeys {
    size_t randomLength = olm_account_generate_one_time_keys_random_length(_account, numberOfKeys);
    NSMutableData *random = [OLMUtility randomBytesOfLength:randomLength];
    size_t result = olm_account_generate_one_time_keys(_account, numberOfKeys, random.mutableBytes, random.length);
    if (result == olm_error()) {
        const char *error = olm_account_last_error(_account);
        NSLog(@"error generating keys: %s", error);
    }
}


@end
