//
//  OLMMessage.m
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import "OLMMessage.h"

@implementation OLMMessage

- (nullable instancetype) initWithCiphertext:(nonnull NSString*)ciphertext type:(OLMMessageType)type {
    NSParameterAssert(ciphertext != nil);
    self = [super init];
    if (!self) {
        return nil;
    }
    _ciphertext = [ciphertext copy];
    _type = type;
    return self;
}

@end
