//
//  OLMUtility.m
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import "OLMUtility.h"

@implementation OLMUtility

+ (NSData*) randomBytesOfLength:(NSUInteger)length {
    uint8_t *randomBytes = malloc(length * sizeof(uint8_t));
    NSParameterAssert(randomBytes != NULL);
    if (!randomBytes) {
        return nil;
    }
    int result = SecRandomCopyBytes(kSecRandomDefault, length, randomBytes);
    if (result != 0) {
        free(randomBytes);
        return nil;
    }
    NSData *data = [NSData dataWithBytesNoCopy:randomBytes length:length freeWhenDone:YES];
    return data;
}

@end
