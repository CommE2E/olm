//
//  OLMKitTests.m
//  OLMKitTests
//
//  Created by Chris Ballinger on 4/8/16.
//
//

#import <XCTest/XCTest.h>
@import OLMKit;

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

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
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
    
    OLMSession *aliceSession = [[OLMSession alloc] initOutboundSessionWithAccount:alice theirIdentityKey:bobIdKey theirOneTimeKey:bobOneTimeKey];
    NSString *message = @"Hello!";
    OLMMessage *aliceToBobMsg = [aliceSession encryptMessage:message];
    
    OLMSession *bobSession = [[OLMSession alloc] initInboundSessionWithAccount:bob oneTimeKeyMessage:aliceToBobMsg.ciphertext];
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg];
    XCTAssertEqualObjects(message, plaintext);
    BOOL success = [bobSession removeOneTimeKeys];
    XCTAssertTrue(success);
}


@end
