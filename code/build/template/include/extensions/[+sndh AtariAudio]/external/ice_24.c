/*
** Ice 2_40 depacker, universal C version
** function is reentrand and thread safe
** placed in public domain 2007 by Hans Wessels
**
** the function:
** ice_24_depack(unsigned char *src, unsigned char *dst);
** depacks Ice 2.1 packed data located at src to dst, it
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
** the function does not check for a valid ice 2.1 header
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

unsigned long get24_long(unsigned char *p)
{
  unsigned long res;
  res=*p++;
  res<<=8;
  res+=*p++;
  res<<=8;
  res+=*p++;
  res<<=8;
  res+=*p;
  return res;
}

unsigned long ice24_get_long(unsigned char **p)
{
  unsigned char *q=*p;
  q-=4;
  *p=q;
  return get24_long(q);
}

unsigned int ice24_getbits(int len, int *cmdp, int *maskp, unsigned char **p)
{
  int tmp=0;
  int cmd=*cmdp;
  int mask=*maskp;
  while(len>0)
  {
    tmp+=tmp;
    mask>>=1;
    if(mask==0)
    {
      *p-=1;
      cmd=**p;
      mask=0x80;
    }
    if(cmd&mask)
    {
      tmp+=1;
    }
    len--;
  }
  *cmdp=cmd;
  *maskp=mask;
  return tmp;
}

int ice_24_header(unsigned char *src)
{ /* returns 0 if no ice 24 header was found */
  return (get24_long(src)==0x49434521UL);
}

long ice_24_packedsize(unsigned char *src)
{ /* returns packed size of ice packed data */
  return (long) get24_long(src+4);
}

long ice_24_origsize(unsigned char *src)
{ /* returns origiginal size of ice packed data */
  return (long) get24_long(src+8);
}

long ice_24_depack(unsigned char *src, unsigned char *dst)
{ /* Ice! V 2.4 depacker */
  unsigned char *p;
  int cmd;
  int mask=0;
  long orig_size;
  if(!ice_24_header(src))
  { /* No 'Ice!' header */
    return -1;
  }
  orig_size=ice_24_origsize(src); /* orig size */
  p=dst+orig_size;
  src+=ice_24_packedsize(src); /* packed size */
  ice24_getbits(1, &cmd, &mask, &src); /* init cmd */
  /* Ice has an init problem, the LSB in cmd that is set is _NOT_ valid
   * reaching this bit is a sign to reload the cmd data (4 bytes)
   * as the msb bit is always set there are always 2 bits set in the cmd
   * block
   */
  { /* fix reload */
    mask=0x80;
    while(!(cmd&1))
    { /* dump all 0 bits */
      cmd>>=1;
      mask>>=1;
    }
    /* dump one 1 bit */
    cmd>>=1;
  }
  for(;;)
  {
    if(ice24_getbits(1, &cmd, &mask, &src))
    { /* literal */
      const int lenbits[]={1,2,2,3,8,15};
      const long int maxlen[]={1,3,3,7,255,32768L};
      const int offset[]={1,2,5,8,15,270};
      int tablepos=-1;
      long int len;
      do
      {
        tablepos++;
        len=ice24_getbits(lenbits[tablepos], &cmd, &mask, &src);
      }
      while(len==maxlen[tablepos]);
      len+=offset[tablepos];
      if((p-dst)<len)
      {
        len=(long int)(p-dst);
      }
      while(len>0)
      {
        *--p=*--src;
        len--;
      }
      if(p<=dst)
      {
        return orig_size;
      }
    }
    /* no else here, always a sld after a literal */
    { /* sld */
      unsigned char* q;
      const int extra_bits[]={0,0,1,2,10};
      const int offset[]={0,1,2,4,8};
      int len;
      int pos=0;
      int tablepos=0;
      while(ice24_getbits(1, &cmd, &mask, &src))
      {
        tablepos++;
        if(tablepos==4)
        {
          break;
        }
      }
      len=offset[tablepos]+ice24_getbits(extra_bits[tablepos], &cmd, &mask, &src);
      if(len)
      {
        const int extra_bits[]={8,5,12};
        const int offset[]={32,0,288};
        int tablepos=0;
        while(ice24_getbits(1, &cmd, &mask, &src))
        {
          tablepos++;
          if(tablepos==2)
          {
            break;
          }
        }
        pos=offset[tablepos]+ice24_getbits(extra_bits[tablepos], &cmd, &mask, &src);
        if(pos!=0)
        {
          pos+=len;
        }
      }
      else
      {
        if(ice24_getbits(1, &cmd, &mask, &src))
        {
          pos=64+ice24_getbits(9, &cmd, &mask, &src);
        }
        else
        {
          pos=ice24_getbits(6, &cmd, &mask, &src);
        }
      }
      len+=2;
      q=p+pos+1;
      if((p-dst)<len)
      {
        len=(int)(p-dst);
      }
      while(len>0)
      {
        *--p=*--q;
        len--;
      }
      if(p<=dst)
      {
        return orig_size;
      }
    }
  }
}
