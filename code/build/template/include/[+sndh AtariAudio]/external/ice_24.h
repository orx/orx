/*
** Ice 2_40 depacker, universal C version
** function is reentrand and thread safe
** placed in public domain 2007 by Hans Wessels
**
** the function:
** ice_24_depack(unsigned char *src, unsigned char *dst);
** depacks Ice 2.4 packed data located at src to dst, it
** returns the size of the depacked data or -1 if no valid
** Ice 2.4 header was found. The memory at dst should be large
** enough to hold the depacked data.
**
** the function:
** int ice_24_header(unsigned char *src)
** returns 0 if no valid Ice 2.4 header was found at src
**
** the function:
** unsigned long ice_24_packedsize(unsigned char *src)
** returns the size of the ice 2.4 packed data located at src,
** the function does not check for a valid ice 2.4 header
**
** the function:
** unsigned long ice_24_origsize(unsigned char *src)
** returns the unpacked (original) size of the ice 2.4 packed 
** data located at src, the function does not check for a valid
** ice 2.4 header
**
** Ice 2.4 was a popoular data packer for the Atari ST series
** Ice 2.4 packed data can be recognized by the characters "ICE!"
** at the first 4 positions in a file
*/

#ifndef __ICE_24_C_H__
#define __ICE_24_C_H__

extern "C"
{

int ice_24_header(unsigned char *src); /* returns 0 if no ice 24 header was found */

long ice_24_packedsize(unsigned char *src); /* returns packed size of ice packed data */

long ice_24_origsize(unsigned char *src); /* returns origiginal size of ice packed data */

long ice_24_depack(unsigned char *src, unsigned char *dst); /* Ice! V 2.4 depacker, returns size of depacked data */

}

#endif