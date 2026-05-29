This folder contains the libcRSID library developed for the cRSID SID-player application.

You can copy this directory into your own application's folder and use it directly as source.
To do so, have #include "libcRSID/libcRSID.h" in your application's sources that use the libCRSID functions.
And also add libcRSID/libcRSID.c to your sources to GCC/TCC compilers.
(See cRSID Makefile as an example.)

Or you can link your application against the generated libcRSID.so dynamic or licRSID.a static library.


The Makefile presented here is able to build libcRSID by itself, or by being called from the main Makefile.
The pre-calculated filter/resampling curves can be recalculated by 'make curvegen'.
There's a minimal example application in 'tests' folder to test building against libCRSID by 'make test'.
(There's an example there too now, that demonstrates usage of the ALSA audio-backend directly instead of SDL's audio-backend.)

For more detailed info see the complete cRSID source-code folder's README.txt.


License is still WTF, you can use the code in any way you like, but I'd like to be mentioned as the original author.


2026 Hermit (Mihaly Horvath)

