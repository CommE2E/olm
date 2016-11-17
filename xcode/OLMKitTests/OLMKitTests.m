/*
Copyright 2016 Chris Ballinger
Copyright 2016 OpenMarket Ltd
Copyright 2016 Vector Creations Ltd

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#import <XCTest/XCTest.h>
#import <OLMKit/OLMKit.h>

@interface OLMKitTests : XCTestCase

@end

@implementation OLMKitTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testAliceAndBob {
    NSError *error;

    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:5];
    NSDictionary *bobIdKeys = bob.identityKeys;
    NSString *bobIdKey = bobIdKeys[@"curve25519"];
    NSDictionary *bobOneTimeKeys = bob.oneTimeKeys;
    NSParameterAssert(bobIdKey != nil);
    NSParameterAssert(bobOneTimeKeys != nil);
    __block NSString *bobOneTimeKey = nil;
    NSDictionary *bobOtkCurve25519 = bobOneTimeKeys[@"curve25519"];
    [bobOtkCurve25519 enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        bobOneTimeKey = obj;
    }];
    XCTAssert([bobOneTimeKey isKindOfClass:[NSString class]]);
    
    OLMSession *aliceSession = [[OLMSession alloc] initOutboundSessionWithAccount:alice theirIdentityKey:bobIdKey theirOneTimeKey:bobOneTimeKey error:nil];
    NSString *message = @"Hello!";
    OLMMessage *aliceToBobMsg = [aliceSession encryptMessage:message error:&error];
    XCTAssertNil(error);
    
    OLMSession *bobSession = [[OLMSession alloc] initInboundSessionWithAccount:bob oneTimeKeyMessage:aliceToBobMsg.ciphertext error:nil];
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg error:&error];
    XCTAssertEqualObjects(message, plaintext);
    XCTAssertNil(error);

    XCTAssert([bobSession matchesInboundSession:aliceToBobMsg.ciphertext]);
    XCTAssertFalse([aliceSession matchesInboundSession:@"ARandomOtkMessage"]);

    NSString *aliceIdKey = alice.identityKeys[@"curve25519"];
    XCTAssert([bobSession matchesInboundSessionFrom:aliceIdKey oneTimeKeyMessage:aliceToBobMsg.ciphertext]);
    XCTAssertFalse([bobSession matchesInboundSessionFrom:@"ARandomIdKey" oneTimeKeyMessage:aliceToBobMsg.ciphertext]);
    XCTAssertFalse([bobSession matchesInboundSessionFrom:aliceIdKey oneTimeKeyMessage:@"ARandomOtkMessage"]);

    BOOL success = [bob removeOneTimeKeysForSession:bobSession];
    XCTAssertTrue(success);
}

- (void) testBackAndForth {
    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:1];
    NSDictionary *bobIdKeys = bob.identityKeys;
    NSString *bobIdKey = bobIdKeys[@"curve25519"];
    NSDictionary *bobOneTimeKeys = bob.oneTimeKeys;
    NSParameterAssert(bobIdKey != nil);
    NSParameterAssert(bobOneTimeKeys != nil);
    __block NSString *bobOneTimeKey = nil;
    NSDictionary *bobOtkCurve25519 = bobOneTimeKeys[@"curve25519"];
    [bobOtkCurve25519 enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        bobOneTimeKey = obj;
    }];
    XCTAssert([bobOneTimeKey isKindOfClass:[NSString class]]);
    
    OLMSession *aliceSession = [[OLMSession alloc] initOutboundSessionWithAccount:alice theirIdentityKey:bobIdKey theirOneTimeKey:bobOneTimeKey error:nil];
    NSString *message = @"Hello I'm Alice!";
    OLMMessage *aliceToBobMsg = [aliceSession encryptMessage:message error:nil];
    
    OLMSession *bobSession = [[OLMSession alloc] initInboundSessionWithAccount:bob oneTimeKeyMessage:aliceToBobMsg.ciphertext error:nil];
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg error:nil];
    XCTAssertEqualObjects(message, plaintext);
    BOOL success = [bob removeOneTimeKeysForSession:bobSession];
    XCTAssertTrue(success);
    
    NSString *msg1 = @"Hello I'm Bob!";
    NSString *msg2 = @"Isn't life grand?";
    NSString *msg3 = @"Let's go to the opera.";
    
    OLMMessage *eMsg1 = [bobSession encryptMessage:msg1 error:nil];
    OLMMessage *eMsg2 = [bobSession encryptMessage:msg2 error:nil];
    OLMMessage *eMsg3 = [bobSession encryptMessage:msg3 error:nil];
    
    NSString *dMsg1 = [aliceSession decryptMessage:eMsg1 error:nil];
    NSString *dMsg2 = [aliceSession decryptMessage:eMsg2 error:nil];
    NSString *dMsg3 = [aliceSession decryptMessage:eMsg3 error:nil];
    XCTAssertEqualObjects(msg1, dMsg1);
    XCTAssertEqualObjects(msg2, dMsg2);
    XCTAssertEqualObjects(msg3, dMsg3);
}

- (void) testAccountSerialization {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:5];
    NSDictionary *bobIdKeys = bob.identityKeys;
    NSDictionary *bobOneTimeKeys = bob.oneTimeKeys;
    
    NSData *bobData = [NSKeyedArchiver archivedDataWithRootObject:bob];
    
    OLMAccount *bob2 = [NSKeyedUnarchiver unarchiveObjectWithData:bobData];
    NSDictionary *bobIdKeys2 = bob2.identityKeys;
    NSDictionary *bobOneTimeKeys2 = bob2.oneTimeKeys;
    
    XCTAssertEqualObjects(bobIdKeys, bobIdKeys2);
    XCTAssertEqualObjects(bobOneTimeKeys, bobOneTimeKeys2);
}

- (void) testSessionSerialization {
    NSError *error;

    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:1];
    NSDictionary *bobIdKeys = bob.identityKeys;
    NSString *bobIdKey = bobIdKeys[@"curve25519"];
    NSDictionary *bobOneTimeKeys = bob.oneTimeKeys;
    NSParameterAssert(bobIdKey != nil);
    NSParameterAssert(bobOneTimeKeys != nil);
    __block NSString *bobOneTimeKey = nil;
    NSDictionary *bobOtkCurve25519 = bobOneTimeKeys[@"curve25519"];
    [bobOtkCurve25519 enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        bobOneTimeKey = obj;
    }];
    XCTAssert([bobOneTimeKey isKindOfClass:[NSString class]]);
    
    OLMSession *aliceSession = [[OLMSession alloc] initOutboundSessionWithAccount:alice theirIdentityKey:bobIdKey theirOneTimeKey:bobOneTimeKey error:nil];
    NSString *message = @"Hello I'm Alice!";
    OLMMessage *aliceToBobMsg = [aliceSession encryptMessage:message error:&error];
    XCTAssertNil(error);

    
    OLMSession *bobSession = [[OLMSession alloc] initInboundSessionWithAccount:bob oneTimeKeyMessage:aliceToBobMsg.ciphertext error:nil];
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg error:nil];
    XCTAssertEqualObjects(message, plaintext);
    BOOL success = [bob removeOneTimeKeysForSession:bobSession];
    XCTAssertTrue(success);
    
    NSString *msg1 = @"Hello I'm Bob!";
    NSString *msg2 = @"Isn't life grand?";
    NSString *msg3 = @"Let's go to the opera.";
    
    OLMMessage *eMsg1 = [bobSession encryptMessage:msg1 error:nil];
    OLMMessage *eMsg2 = [bobSession encryptMessage:msg2 error:nil];
    OLMMessage *eMsg3 = [bobSession encryptMessage:msg3 error:nil];
    
    NSData *aliceData = [NSKeyedArchiver archivedDataWithRootObject:aliceSession];
    OLMSession *alice2 = [NSKeyedUnarchiver unarchiveObjectWithData:aliceData];
    
    NSString *dMsg1 = [alice2 decryptMessage:eMsg1 error:nil];
    NSString *dMsg2 = [alice2 decryptMessage:eMsg2 error:nil];
    NSString *dMsg3 = [alice2 decryptMessage:eMsg3 error:nil];
    XCTAssertEqualObjects(msg1, dMsg1);
    XCTAssertEqualObjects(msg2, dMsg2);
    XCTAssertEqualObjects(msg3, dMsg3);
}

- (void)testEd25519Signing {

    OLMUtility *olmUtility = [[OLMUtility alloc] init];
    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];

    NSDictionary *aJSON = @{
                            @"key1": @"value1",
                            @"key2": @"value2"
                            };
    NSData *message = [NSKeyedArchiver archivedDataWithRootObject:aJSON];
    NSString *signature = [alice signMessage:message];


    NSString *aliceEd25519Key = alice.identityKeys[@"ed25519"];

    NSError *error;
    BOOL result = [olmUtility verifyEd25519Signature:signature key:aliceEd25519Key message:message error:&error];
    XCTAssert(result);
    XCTAssertNil(error);
}

@end
