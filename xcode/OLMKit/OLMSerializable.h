//
//  OLMSerializable.h
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import <Foundation/Foundation.h>

@protocol OLMSerializable <NSObject>

/** Initializes from encrypted serialized data. Will throw error if invalid key or invalid base64. */
- (instancetype) initWithSerializedData:(NSString*)serializedData key:(NSData*)key error:(NSError**)error;

/** Serializes and encrypts object data, outputs base64 blob */
- (NSString*) serializeDataWithKey:(NSData*)key error:(NSError**)error;

@end
