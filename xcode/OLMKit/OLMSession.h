//
//  OLMSession.h
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import <Foundation/Foundation.h>
#import "OLMSerializable.h"
#import "OLMAccount.h"

@interface OLMSession : NSObject <OLMSerializable>

- (instancetype) initOutboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSData*)theirIdentityKey theirOneTimeKey:(NSData*)theirOneTimeKey;

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account oneTimeKeyMessage:(NSData*)oneTimeKeyMessage;

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSData*)theirIdentityKey oneTimeKeyMessage:(NSData*)oneTimeKeyMessage;

- (NSData*) sessionIdentifier;

- (BOOL) matchesInboundSession:(NSData*)oneTimeKeyMessage;

- (BOOL) matchesInboundSessionFrom:(NSData*)theirIdentityKey oneTimeKeyMessage:(NSData *)oneTimeKeyMessage;

- (void) removeOneTimeKeys;

@end
