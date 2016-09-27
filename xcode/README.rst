OLMKit
======

OLMKit exposes an Objective-C wrapper to libolm.

The original work by Chris Ballinger can be found at https://github.com/chrisballinger/OLMKit.

Installation
------------
You can embed OLMKit to your application project with CocoaPods. The pod for
the latest OLMKit release is::

    pod 'OLMKit'

Development
-----------
Run `pod install` and open `OLMKit.xcworkspace`.

The project contains only tests files. The libolm and the Objective-C wrapper source files are loaded via the OLMKit CocoaPods pod.

To add a new source file, add it to the file system and run `pod update` to make CocoaPods insert into OLMKit.xcworkspace. 

