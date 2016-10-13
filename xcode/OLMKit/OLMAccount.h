//
//  OLMAccount.h
//  olm
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import <Foundation/Foundation.h>
#import "OLMSerializable.h"

@class OLMSession;

@interface OLMAccount : NSObject <OLMSerializable, NSSecureCoding>

/** Creates new account */
- (instancetype) initNewAccount;

/** public identity keys. base64 encoded in "curve25519" and "ed25519" keys */
- (NSDictionary*) identityKeys;

/** signs message with ed25519 key for account */
- (NSString*) signMessage:(NSData*)messageData;

/** Public parts of the unpublished one time keys for the account */
- (NSDictionary*) oneTimeKeys;

- (BOOL) removeOneTimeKeysForSession:(OLMSession*)session;

/** Marks the current set of one time keys as being published. */
- (void) markOneTimeKeysAsPublished;

/** The largest number of one time keys this account can store. */
- (NSUInteger) maxOneTimeKeys;

/** Generates a number of new one time keys. If the total number of keys stored
 * by this account exceeds -maxOneTimeKeys then the old keys are
 * discarded. */
- (void) generateOneTimeKeys:(NSUInteger)numberOfKeys;

@end
