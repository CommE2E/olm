//
//  OLMUtility.m
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import "OLMUtility.h"

@implementation OLMUtility

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
