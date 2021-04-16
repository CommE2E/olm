OlmLibSdk
=========

OlmLibSdk exposes an android wrapper to libolm.

Installation
------------
Create a libs directory in your project directory
Copy the olm-sdk.aar into it.

In your build.gradle file, add in the android section::

    repositories {
        flatDir {
            dir 'libs'
        }
    }

Add in the dependencies category::

    compile(name: 'olm-sdk', ext: 'aar')

Development
-----------
import the project from the ``android/`` path.

The project contains some JNI files and some Java wrapper files.

The project contains some tests under AndroidTests package.
