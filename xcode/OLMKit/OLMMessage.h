//
//  OLMMessage.h
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import <Foundation/Foundation.h>

/*
 from olm.hh
 static const size_t OLM_MESSAGE_TYPE_PRE_KEY = 0;
 static const size_t OLM_MESSAGE_TYPE_MESSAGE = 1;
 */
typedef NS_ENUM(NSInteger, OLMMessageType) {
    OLMMessageTypePreKey = 0,
    OLMMessageTypeMessage = 1
};

@interface OLMMessage : NSObject

@property (nonatomic, copy, readonly, nonnull) NSString *ciphertext;
@property (readonly) OLMMessageType type;

- (nullable instancetype) initWithCiphertext:(nonnull NSString*)ciphertext type:(OLMMessageType)type;

@end
