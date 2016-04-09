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
    NSDictionary *identityKeys = bob.identityKeys;
    NSDictionary *oneTimeKeys = bob.oneTimeKeys;
    NSParameterAssert(identityKeys != nil);
    NSParameterAssert(oneTimeKeys != nil);
}


@end
