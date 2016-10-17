//
//  OLMUtility.m
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import "OLMUtility.h"

#include "olm/olm.h"

@interface OLMUtility()

@property (nonatomic) OlmUtility *utility;

@end

@implementation OLMUtility

- (void) dealloc {
    olm_clear_utility(_utility);
    free(_utility);
}

- (BOOL) initializeUtilityMemory {
    size_t utilitySize = olm_utility_size();
    _utility = malloc(utilitySize);
    NSParameterAssert(_utility != nil);
    if (!_utility) {
        return NO;
    }
    _utility = olm_utility(_utility);
    NSParameterAssert(_utility != nil);
    if (!_utility) {
        return NO;
    }
    return YES;
}

- (instancetype) init {
    self = [super init];
    if (!self) {
        return nil;
    }
    BOOL success = [self initializeUtilityMemory];
    if (!success) {
        return nil;
    }
    return self;
}

- (NSString *)sha256:(NSData *)message {
    size_t length = olm_sha256_length(_utility);

    NSMutableData *shaData = [NSMutableData dataWithLength:length];
    if (!shaData) {
        return nil;
    }

    size_t result = olm_sha256(_utility, message.bytes, message.length, shaData.mutableBytes, shaData.length);
    if (result == olm_error()) {
        const char *error = olm_utility_last_error(_utility);
        NSAssert(NO, @"olm_sha256 error: %s", error);
        return nil;
    }
    
    NSString *sha = [[NSString alloc] initWithData:shaData encoding:NSUTF8StringEncoding];
    return sha;
}

- (BOOL)verifyEd25519Signature:(NSString*)signature key:(NSString*)key message:(NSData*)message error:(NSError**)error {

    NSData *keyData = [key dataUsingEncoding:NSUTF8StringEncoding];
    NSData *signatureData = [signature dataUsingEncoding:NSUTF8StringEncoding];

    size_t result = olm_ed25519_verify(_utility,
                                       keyData.bytes, keyData.length,
                                       message.bytes, message.length,
                                       signatureData.bytes, signatureData.length
                                       );

    if (result == olm_error()) {
        if (error) {
            NSDictionary *userInfo = @{NSLocalizedFailureReasonErrorKey: [NSString stringWithUTF8String:olm_utility_last_error(_utility)]};

            // @TODO
            *error = [[NSError alloc] initWithDomain:@"OLMKitErrorDomain" code:0 userInfo:userInfo];
        }
        return NO;
    }
    else {
        return YES;
    }
}

+ (NSMutableData*) randomBytesOfLength:(NSUInteger)length {
    NSMutableData *randomData = [NSMutableData dataWithLength:length];
    if (!randomData) {
        return nil;
    }
    int result = SecRandomCopyBytes(kSecRandomDefault, randomData.length, randomData.mutableBytes);
    if (result != 0) {
        return nil;
    }
    return randomData;
}

@end
