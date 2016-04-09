//
//  OLMMessage.h
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, OLMMessageType) {
    OLMMessageTypeUnknown,
    OLMMessageTypePreKey,
    OLMMessageTypeMessage
};

@interface OLMMessage : NSObject

@property (nonatomic, readonly, nonnull) NSString *message;
@property (readonly) OLMMessageType type;

- (nonnull instancetype) initWithMessage:(nonnull NSString*)message type:(OLMMessageType)type;

@end
