//Main configuration-header of cRSID library sources
//Sane C 'weak' default values if not defined externally by Config.mk and makefiles.
//(This allows to still build the sources as 'blackbox' without the need for external defines/overrides.)
//(And the curve-generator tool respects and uses this config as well, so 'make curvegen' when changing values here.)

#ifndef LIBCRSID_HEADER__CONFIG
#define LIBCRSID_HEADER__CONFIG //used  to prevent double inclusion of this header-file


#ifndef CRSID_OVERSAMPLING_RATIO
 #define CRSID_OVERSAMPLING_RATIO 5  //gets refined to float at init, so that cycles per sample is what is integer
#endif


#ifndef CRSID_FILTERTABLE_RESOLUTION
 #define CRSID_FILTERTABLE_RESOLUTION 12 //bits  (quantization resolution of table-values)
#endif

#ifndef CRSID_OVERSAMPLING_FILTERTABLE_RESOLUTION
 #define CRSID_OVERSAMPLING_FILTERTABLE_RESOLUTION 13 //bits  (quantization resolution of table-values)
#endif


#ifndef CRSID_RESAMPLER_SINCWINDOW_PERIODS
 #define CRSID_RESAMPLER_SINCWINDOW_PERIODS 6 //12 //Number of Sinc-window sine-periods (min. 4) (for Nyquist-filter)
#endif                       //(Should be an even number, the bigger, the more demanding the convolution is on CPU.)

#ifndef CRSID_RESAMPLER_SINCPERIOD_SAMPLES
 #define CRSID_RESAMPLER_SINCPERIOD_SAMPLES 256 //128 //sample-entries in a Sinc-period (max. 256)
#endif

#ifndef CRSID_RESAMPLER_SINCWINDOW_MAGNITUDE
 #define CRSID_RESAMPLER_SINCWINDOW_MAGNITUDE 2048 //value of Sinc-window's data in the middle (max. 16384)
#endif


#endif //LIBCRSID_HEADER__CONFIG