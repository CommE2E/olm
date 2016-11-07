/*
 Copyright 2016 OpenMarket Ltd

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

#include "olm/olm.h"

@interface OLMKitGroupTests : XCTestCase

@end

@implementation OLMKitGroupTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testAliceAndBob {

    OLMOutboundGroupSession *aliceSession = [[OLMOutboundGroupSession alloc] initOutboundGroupSession];
    XCTAssertGreaterThan(aliceSession.sessionIdentifier.length, 0);
    XCTAssertGreaterThan(aliceSession.sessionKey.length, 0);
    XCTAssertEqual(aliceSession.messageIndex, 0);

    // Store the session key before starting encrypting
    NSString *sessionKey = aliceSession.sessionKey;

    NSString *message = @"Hello!";
    NSString *aliceToBobMsg = [aliceSession encryptMessage:message];

    XCTAssertEqual(aliceSession.messageIndex, 1);
    XCTAssertGreaterThanOrEqual(aliceToBobMsg.length, 0);

    OLMInboundGroupSession *bobSession = [[OLMInboundGroupSession alloc] initInboundGroupSessionWithSessionKey:sessionKey];
    XCTAssertEqualObjects(aliceSession.sessionIdentifier, bobSession.sessionIdentifier);

    NSUInteger messageIndex;
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg messageIndex:&messageIndex];
    XCTAssertEqualObjects(message, plaintext);
    XCTAssertEqual(messageIndex, 0);
}

- (void)testOutboundGroupSessionSerialization {

    OLMOutboundGroupSession *aliceSession = [[OLMOutboundGroupSession alloc] initOutboundGroupSession];

    NSData *aliceData = [NSKeyedArchiver archivedDataWithRootObject:aliceSession];
    OLMOutboundGroupSession *aliceSession2 = [NSKeyedUnarchiver unarchiveObjectWithData:aliceData];

    XCTAssertEqualObjects(aliceSession2.sessionKey, aliceSession.sessionKey);
    XCTAssertEqualObjects(aliceSession2.sessionIdentifier, aliceSession.sessionIdentifier);
}

- (void)testInboundGroupSessionSerialization {

    OLMOutboundGroupSession *aliceSession = [[OLMOutboundGroupSession alloc] initOutboundGroupSession];

    OLMInboundGroupSession *bobSession = [[OLMInboundGroupSession alloc] initInboundGroupSessionWithSessionKey:aliceSession.sessionKey];

    NSData *bobData = [NSKeyedArchiver archivedDataWithRootObject:bobSession];
    OLMInboundGroupSession *bobSession2 = [NSKeyedUnarchiver unarchiveObjectWithData:bobData];

    XCTAssertEqualObjects(bobSession2.sessionIdentifier, aliceSession.sessionIdentifier);
}

@end
