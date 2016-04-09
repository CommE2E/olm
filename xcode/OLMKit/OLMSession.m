//
//  OLMSession.m
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import "OLMSession.h"
@import olm;

@interface OLMSession()
@property (nonatomic) OlmSession *session;
@end

@implementation OLMSession

- (instancetype) initOutboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSData*)theirIdentityKey theirOneTimeKey:(NSData*)theirOneTimeKey {
    
}

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account oneTimeKeyMessage:(NSData*)oneTimeKeyMessage {
    
}

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSData*)theirIdentityKey oneTimeKeyMessage:(NSData*)oneTimeKeyMessage {
    
}

@end
