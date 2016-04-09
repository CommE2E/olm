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
#import "OLMMessage.h"

@interface OLMSession : NSObject <OLMSerializable>

@property (nonatomic, strong) OLMAccount *account;

- (instancetype) initOutboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSString*)theirIdentityKey theirOneTimeKey:(NSString*)theirOneTimeKey;

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account oneTimeKeyMessage:(NSString*)oneTimeKeyMessage;

- (instancetype) initInboundSessionWithAccount:(OLMAccount*)account theirIdentityKey:(NSString*)theirIdentityKey oneTimeKeyMessage:(NSString*)oneTimeKeyMessage;

- (NSString*) sessionIdentifier;

- (BOOL) matchesInboundSession:(NSString*)oneTimeKeyMessage;

- (BOOL) matchesInboundSessionFrom:(NSString*)theirIdentityKey oneTimeKeyMessage:(NSString *)oneTimeKeyMessage;

- (BOOL) removeOneTimeKeys;

/** UTF-8 plaintext -> base64 ciphertext */
- (OLMMessage*) encryptMessage:(NSString*)message;

/** base64 ciphertext -> UTF-8 plaintext */
- (NSString*) decryptMessage:(OLMMessage*)message;

@end
