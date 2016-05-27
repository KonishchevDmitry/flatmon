// This file should be included in each source file to provide the basic and properly configured functionality for your
// project.

#ifndef Util_Core_hpp
#define Util_Core_hpp

#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <cstddef>
    #include <cstdlib>
#endif

// Arduino IDE compiles project and libraries with the same command line options. Enabling all warnings in IDE settings
// produces warnings from libraries on each compilation. This workaround is used to enable the warnings only for files
// controlled by the user.
#pragma GCC diagnostic warning "-Wall"

#endif

