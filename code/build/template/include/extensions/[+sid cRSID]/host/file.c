
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#ifdef CRSID_PLATFORM_PC
 #include "libgen.h"
#endif

#include "md5.c"



static const char* findSongLengthRow (const char* text, const char* prefix) { //, char** delimiter) {
   static const char* line_end;
   const char* line_start = text;
   size_t prefix_len = strlen(prefix);
   while (*line_start != '\0') { // Find next newline
       line_end = strchr(line_start, '\n');
       if (line_end == NULL) {
           line_end = line_start + strlen(line_start);
       }
       const char* actual_end = line_end;
       if (line_end > line_start && *(line_end - 1) == '\r') { // Adjust for \r\n if present
           actual_end--;
       }
       if (strncmp(line_start, prefix, prefix_len) == 0) { //printf("Found line: %.*s\n", (int)(actual_end - line_start), line_start);
           //*delimiter = (char*) actual_end;
           return line_start;
       }
       line_start = (*line_end == '\0') ? line_end : line_end + 1;
   }
   return NULL;
}


void cRSID_getPlaylengths (unsigned char* filedata, int filesize) {
 enum { MD5_BIT_AMOUNT = 128, MD5_BYTE_AMOUNT = MD5_BIT_AMOUNT/8, MD5_NYBBLE_AMOUNT = MD5_BYTE_AMOUNT*2, MD5_DELIMITER = '='}; //, PLAYTIME_DELIMITER = ' ' };
 static const char* PLAYTIME_DELIMITER = " \n\r";
 static int i,j, LineLength, PlayTimeLength, SubtuneCount=0, ScannedDataCount, ScannedMinutes=0, ScannedSeconds=0, ScannedMicroSeconds=0; //static char Data;
 static const char *LineStart, *LineEnd, *PlayTimeStart, *PlayTimeEnd; //static char LineEndTemp;
 static uint8_t MD5sum [MD5_BYTE_AMOUNT];
 static char MD5string [MD5_NYBBLE_AMOUNT+1], PlayTimeString [CRSID_PLAYTIMESTRING_LENGTH_MAX];

 for (i=0; i<=CRSID_SUBTUNE_AMOUNT_MAX; ++i) cRSID.SubtuneDurations[i] = 0; //Will need to be populated from playlist or songlength-database file

 if (cRSID.SongLengths == NULL) return;

 cRSID_MD5_calculateSum( filedata, filesize, MD5sum );
 for(i=0; i < MD5_BYTE_AMOUNT; ++i) sprintf( MD5string + i * 2, "%02x", MD5sum[i] );
 //printf("MD5-sum: %s\n",MD5string);
 LineStart = findSongLengthRow( cRSID.SongLengths, MD5string ); //, &LineEnd ); //LineEndTemp = *LineEnd; *LineEnd = '\0'; printf("--%s--\n", LineStart); *LineEnd = LineEndTemp;
 if (LineStart == NULL) return;
 LineStart = strchr( LineStart, MD5_DELIMITER ); if (LineStart == NULL) return; //now we have subtune-songlnengths separated by spaces in 'M:SS[.sss]' format where 'sss' is optional milliseconds field
 LineLength = strcspn( LineStart, "\r\n" ); LineEnd = LineStart + LineLength; //printf( "%.*s\n", LineLength, LineStart+1 );
 PlayTimeStart = LineStart + 1; //skip '='
 for (i = SubtuneCount = 0; i <= CRSID_SUBTUNE_AMOUNT_MAX && PlayTimeStart < LineEnd; ++i) {
    PlayTimeLength = strcspn( PlayTimeStart, PLAYTIME_DELIMITER ); PlayTimeEnd = PlayTimeStart + PlayTimeLength;
    strncpy( PlayTimeString, PlayTimeStart, PlayTimeLength ); PlayTimeString[PlayTimeLength]='\0'; //printf( "%s\n", PlayTimeString );
    ScannedDataCount = sscanf( PlayTimeString, "%d:%d.%d", &ScannedMinutes, &ScannedSeconds, &ScannedMicroSeconds ); //printf("Subtune %d Playtime: %d numbers found: Minutes:%d, Seconds:%d, MicroSeconds:%d\n",SubtuneCount, ScannedDataCount, ScannedMinutes, ScannedSeconds, ScannedMicroSeconds );
    if (ScannedDataCount >= 2) {
        cRSID.SubtuneDurations[ SubtuneCount + 1 ] = ScannedMinutes * CRSID_SECONDS_PER_MINUTE + ScannedSeconds + (ScannedMicroSeconds > 0); // >= 500);
    }
    ++SubtuneCount;
    PlayTimeStart = PlayTimeEnd + strspn( PlayTimeEnd, PLAYTIME_DELIMITER ); //go after the just processed playtime and look for end of whitespace (next entry) or end
 }
}



#ifdef CRSID_PLATFORM_PC



char* cRSID_fileNameOnly (char* path) {
 int LastPos; char* name;
 LastPos=strlen(path)-1;
 #ifndef WINDOWS
  if (path[LastPos] == '/') path[LastPos]='\0'; //remove possible trailing slash if foldername
  return basename(path);
 #else
  if (path[LastPos] == '\\') path[LastPos]='\0'; //remove possible trailing slash if foldername
  name=strrchr(path,'\\'); if(name==NULL) name=strrchr(path,'/');  return (name!=NULL)? name+1:path;
 #endif
}

char* cRSID_folderNameOnly (char* path) {
  return dirname(path);
}


char cRSID_compareFileExtension (char *filename, char *extension) {
 short i,j;  //get pointer of file-extension from filename string  //if no '.' found, point to end of the string
 for (i=strlen(filename)-1,j=strlen(extension)-1; i>=0 && j>=0; --i,--j) {
  if(tolower(filename[i])!=tolower(extension[j])) return 0;
 }
 return 1;
}

char cRSID_playableExtension (char *name) {
 if (name==NULL) return 0;
 cRSID_removeNewline(name);
 return cRSID_compareFileExtension(name,CRSID_SID_FILE_EXTENSION) || cRSID_compareFileExtension(name,CRSID_SID_FILE_EXTENSION_ALTERNATIVE); //|| cRSID_compareFileExtension(name,".sid.prg") || cRSID_compareFileExtension(name,".prg");
}


int cRSID_loadSIDfile (unsigned char* SIDfileData, char* filename, int maxlen) {
 static signed short Data;
 static signed int SizeCnt;
 static FILE *SIDfile;
 /*#ifndef CRSID_LIBRARY
  //static
         #include "../../resources/builtin-music.h"
 #endif*/

 SizeCnt=0;
 if (cRSID.BuiltInMusic==0) {
  if ( (SIDfile=fopen(filename,"rb")) == NULL || !cRSID_playableExtension(filename) ) return CRSID_ERROR_LOAD;
  while ( (Data=fgetc(SIDfile)) != EOF ) {
   if (SizeCnt >= maxlen) return CRSID_ERROR_LOAD;
   SIDfileData[SizeCnt] = Data; SizeCnt++;
  }
  fclose(SIDfile);
 }
 //#ifndef CRSID_LIBRARY
 else {
  while (cRSID.BuiltInMusicData != NULL && SizeCnt < cRSID.BuiltInMusicSize)
  { SIDfileData[SizeCnt] = cRSID.BuiltInMusicData[SizeCnt]; ++SizeCnt; } //cRSID.BuiltInMusic=0; }  //else while(SizeCnt<(int)sizeof(builtin_music)) { SIDfileData[SizeCnt] = builtin_music[SizeCnt]; ++SizeCnt; }
 }
 //#endif

 cRSID_getPlaylengths( SIDfileData, SizeCnt ); //getting the info early, but later on too in processSIDfileData()

 return SizeCnt;
}


#endif //CRSID_PLATFORM_PC


cRSID_SIDheader* cRSID_processSIDfileData (unsigned char* filedata, int filesize) {
 static int i;
 static unsigned short SIDdataOffset;
 static cRSID_SIDheader *SIDheader;
 static const char MagicStringPSID[]="PSID";
 //static const char MagicStringRSID[]="RSID";

 cRSID.SIDheader = SIDheader = (cRSID_SIDheader*) filedata;

 //copy KERNAL & BASIC ROM contents into the RAM under them? (So PSIDs that don't select bank correctly will work better.)
 for (i=0xA000; i<0x10000; ++i) cRSID_C64.RAMbank[i]=cRSID_C64.ROMbanks[i];
 for (i=0x0000; i < 0xA000; ++i) cRSID_C64.RAMbank[i]=0; //fresh start (maybe some bugged SIDs want 0 at certain RAM-locations)
 for (i=0xC000; i < 0xD000; ++i) cRSID_C64.RAMbank[i]=0;

 if ( SIDheader->MagicString[0] != 'P' && SIDheader->MagicString[0] != 'R' ) return NULL;
 for (i=1; i < (int)(sizeof(MagicStringPSID)-1); ++i) { if (SIDheader->MagicString[i] != MagicStringPSID[i]) return NULL; }
 cRSID.RealSIDmode = ( SIDheader->MagicString[0] == 'R' );
 cRSID_C64.RealSIDmode = cRSID.RealSIDmode; //update shadowed copy used by the audio thread (sound-thread shouldn't run at this moment)

 if (SIDheader->LoadAddressH==0 && SIDheader->LoadAddressL==0) { //load-address taken from first 2 bytes of the C64 PRG
  cRSID.LoadAddress = (filedata[SIDheader->HeaderSize+1]<<8) + (filedata[SIDheader->HeaderSize+0]);
  SIDdataOffset = SIDheader->HeaderSize+2;
 }
 else { //load-adress taken from SID-header
  cRSID.LoadAddress = (SIDheader->LoadAddressH<<8) + (SIDheader->LoadAddressL);
  SIDdataOffset = SIDheader->HeaderSize;
 }

 //Used by subtune-playback too after first init, so tunes that modify memory can start with a freshly loaded data:
 for (i=SIDdataOffset; i<filesize; ++i) cRSID_C64.RAMbank [ cRSID.LoadAddress + (i-SIDdataOffset) ] = filedata[i];

 i = cRSID.LoadAddress + (filesize-SIDdataOffset);
 cRSID.EndAddress = (i<0x10000) ? i : 0xFFFF;

 cRSID.PSIDdigiMode = ( !cRSID.RealSIDmode && (SIDheader->ModelFormatStandard & 2) );

 cRSID_getPlaylengths( filedata, filesize );
 if (cRSID.FallbackPlayTime > 0) {
    for (i=0; i<=CRSID_SUBTUNE_AMOUNT_MAX; ++i) { if (cRSID.SubtuneDurations[i] <= 0) cRSID.SubtuneDurations[i] = cRSID.FallbackPlayTime; }
 }

 #ifdef CRSID_PLATFORM_PC
 if (cRSID.PlayTimeMinutes[cRSID.PlayListPlayPosition] || cRSID.PlayTimeSeconds[cRSID.PlayListPlayPosition]) {//if playtime is given in playlist, override for 1st subtune
  cRSID.SubtuneDurations[ 1 ] = cRSID.PlayTimeMinutes[cRSID.PlayListPlayPosition] * CRSID_SECONDS_PER_MINUTE + cRSID.PlayTimeSeconds[cRSID.PlayListPlayPosition];
 }
 #endif

 return cRSID.SIDheader;
}

