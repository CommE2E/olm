/*
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

#import "OLMKit.h"

#include "olm/olm.h"

@implementation OLMKit

+ (NSString*)versionString
{
    uint8_t major, minor, patch;
    
    olm_get_library_version(&major, &minor, &patch);
    
    return [NSString stringWithFormat:@"%tu.%tu.%tu", major, minor, patch];
}

+ (instancetype)sharedInstance
{
    static OLMKit *sharedInstance = nil;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}

@end
