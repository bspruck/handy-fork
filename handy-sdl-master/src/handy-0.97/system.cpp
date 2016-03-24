//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// System object class                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides the glue to bind of of the emulation objects         //
// together via peek/poke handlers and pass thru interfaces to lower        //
// objects, all control of the emulator is done via this class. Update()    //
// does most of the work and each call emulates one CPU instruction and     //
// updates all of the relevant hardware if required. It must be remembered  //
// that if that instruction involves setting SPRGO then, it will cause a    //
// sprite painting operation and then a corresponding update of all of the  //
// hardware which will usually involve recursive calls to Update, see       //
// Mikey SPRGO code for more details.                                       //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define SYSTEM_CPP

//#include <crtdbg.h>
//#define	TRACE_SYSTEM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "error.h"
#include <zlib.h>
#include "compress/unzip.h"

// The following is needed since GCC doesn't seem to understand the _splitpath function.
// Because of this "problem", an additional header file is created which resides the
// _splitpath function als well as other dependencies.
//
// Source : Usenetposting done by Martin Schmidt
#ifdef SDL_PATCH
#include "stdlib_gcc_extra.h"
#endif

extern void lynx_decrypt(unsigned char * result, const unsigned char * encrypted, const int length);

// Posted by Karri
unsigned char micro_loader_stage1[52]= {
   0xff, 0x81, 0xca, 0x33, 0xbe, 0x80, 0xa2, 0xc4, 0x6d, 0x98, 0xfe, 0x8d, 0xbc, 0x66, 0xc0, 0x7a,
   0x09, 0x50, 0x23, 0x28, 0x18, 0xc8, 0x06, 0x70, 0x58, 0x4f, 0x1b, 0xe1, 0xc7, 0x90, 0x08, 0xcd,
   0x1a, 0x6e, 0x5a, 0x45, 0x32, 0xd7, 0x6d, 0xc6, 0x8a, 0xe5, 0xd8, 0x5c, 0xa0, 0xe8, 0x4f, 0x7a,
   0x5f, 0x73, 0x8d, 0x22
};


// and the secondary loader that follows the encrypted loader

unsigned char micro_loader_stage2[128+12+11]= {
   0xa2, 0x00, 0xa0, 0x08, 0xad, 0xb2, 0xfc, 0x95, 0x26, 0xe8, 0x88, 0xd0,
   0xf7, 0xa5, 0x26, 0x85, 0x2e, 0x20, 0xca, 0xfb, 0xa5, 0x28, 0x49, 0xff, 0xa8, 0xa5, 0x27, 0x49,
   0xff, 0xaa, 0x20, 0xa1, 0xfb, 0xa5, 0x2a, 0xa6, 0x2b, 0x85, 0x31, 0x86, 0x32, 0xa5, 0x2d, 0x49,
   0xff, 0xa8, 0xa5, 0x2c, 0x49, 0xff, 0xaa, 0x20, 0xac, 0xfb, 0x6c, 0x2a, 0x00, 0xe8, 0xd0, 0x03,
   0xc8, 0xf0, 0x57, 0x20, 0xbf, 0xfb, 0x80, 0xf5, 0xe8, 0xd0, 0x03, 0xc8, 0xf0, 0x4c, 0x20, 0xbf,
   0xfb, 0x92, 0x31, 0xe6, 0x31, 0xd0, 0xf1, 0xe6, 0x32, 0x80, 0xed, 0xad, 0xb2, 0xfc, 0xe6, 0x2f,
   0xd0, 0x38, 0xe6, 0x30, 0xd0, 0x34, 0x48, 0xda, 0x5a, 0xa5, 0x1a, 0x29, 0xfc, 0xa8, 0x09, 0x02,
   0xaa, 0xa5, 0x2e, 0xe6, 0x2e, 0x38, 0x80, 0x0b, 0x90, 0x04, 0x8e, 0x8b, 0xfd, 0x18, 0xe8, 0x8e,
   0x87, 0xfd, 0xca, 0x8e, 0x87, 0xfd, 0x2a, 0x8c, 0x8b, 0xfd, 0xd0, 0xec, 0xa5, 0x1a, 0x8d, 0x8b,
   0xfd, 0x64, 0x2f, 0xa9, 0xfc, 0x85, 0x30, 0x7a, 0xfa, 0x68, 0x60
};

int lss_read(void* dest,int varsize, int varcount,LSS_FILE *fp)
{
   ULONG copysize;
   copysize=varsize*varcount;
   if((fp->index + copysize) > fp->index_limit) copysize=fp->index_limit - fp->index;
   memcpy(dest,fp->memptr+fp->index,copysize);
   fp->index+=copysize;
   return copysize;
}

CSystem::CSystem(char* gamefile,char* romfile)
   :mCart(NULL),
    mRom(NULL),
    mMemMap(NULL),
    mRam(NULL),
    mCpu(NULL),
    mMikie(NULL),
    mSusie(NULL),
    mEEPROM()
{

#ifdef _LYNXDBG
   mpDebugCallback=NULL;
   mDebugCallbackObject=0;
#endif

   // Select the default filetype
   UBYTE *filememory=NULL;
   UBYTE *howardmemory=NULL;
   ULONG filesize=0;
   ULONG howardsize=0;

   mFileType=HANDY_FILETYPE_ILLEGAL;
   if(strcmp(gamefile,"")==0) {
      // No file
      filesize=0;
      filememory=NULL;
   } else if(IsZip(gamefile)) {
      // Try and find a file in the zip
      unzFile *fp;
      unz_file_info info;
      char filename_buf[0x100], *ptr;
      bool gotIt;

      if((fp=(unzFile*)unzOpen(gamefile))!=NULL) {
         if(unzGoToFirstFile(fp)!=UNZ_OK) {
            unzClose(fp);
            CLynxException lynxerr;
            lynxerr.Message() << "Handy Error: ZIP File select problems" ;
            lynxerr.Description()
                  << "The file you selected could not be read." << endl
                  << "(The ZIP file may be corrupted)." << endl ;
            throw(lynxerr);
         }

         gotIt = FALSE;
         for (;;) {
            // Get file descriptor and analyse
            if(unzGetCurrentFileInfo(fp, &info, filename_buf, 0x100, NULL, 0, NULL, 0) != UNZ_OK) {
               break;
            } else {
               ptr = strchr(filename_buf, '.');
               if (ptr != NULL) {
                  char buf[4];

                  ptr++;
                  buf[0] = tolower(*ptr);
                  ptr++;
                  buf[1] = tolower(*ptr);
                  ptr++;
                  buf[2] = tolower(*ptr);
                  buf[3] = 0;
                  if (!strcmp(buf, "lnx") || !strcmp(buf, "com") || !strcmp(buf, "o")) {
                     // Found a likely file so signal
                     gotIt = TRUE;
                     break;
                  }
               }

               // No match so lets try the next file
               if(unzGoToNextFile(fp)!=UNZ_OK)	break;
            }
         }

         // Did we strike gold ?
         if(gotIt) {
            if(unzOpenCurrentFile(fp)==UNZ_OK) {
               // Allocate memory for the rom
               filesize=info.uncompressed_size;
               filememory=(UBYTE*) new UBYTE[filesize];

               // Read it into memory
               if(unzReadCurrentFile(fp,filememory,filesize)!=(int)info.uncompressed_size) {
                  unzCloseCurrentFile(fp);
                  unzClose(fp);
                  delete filememory;
                  // Throw a wobbly
                  CLynxException lynxerr;
                  lynxerr.Message() << "Handy Error: ZIP File load problems" ;
                  lynxerr.Description()
                        << "The zip file you selected could not be loaded." << endl
                        << "(The ZIP file may be corrupted)." << endl ;
                  throw(lynxerr);
               }

               // Got it!
               unzCloseCurrentFile(fp);
               unzClose(fp);
            }

         } else {
            CLynxException lynxerr;
            lynxerr.Message() << "Handy Error: ZIP File load problems" ;
            lynxerr.Description()
                  << "The file you selected could not be loaded." << endl
                  << "Could not find a Lynx file in the ZIP archive." << endl ;
            throw(lynxerr);
         }
      } else {
         CLynxException lynxerr;
         lynxerr.Message() << "Handy Error: ZIP File open problems" ;
         lynxerr.Description()
               << "The file you selected could not be opened." << endl
               << "(The ZIP file may be corrupted)." << endl ;
         throw(lynxerr);
      }
   } else {
      // Open the file and load the file
      FILE	*fp;

      // Open the cartridge file for reading
      if((fp=fopen(gamefile,"rb"))==NULL) {
         CLynxException lynxerr;

         lynxerr.Message() << "Handy Error: File Open Error";
         lynxerr.Description()
               << "The lynx emulator will not run without a cartridge image." << endl
               << "\"" << gamefile << "\" was not found in the place you " << endl
               << "specified. (see the Handy User Guide for more information).";
         throw(lynxerr);
      }

      // How big is the file ??
      fseek(fp,0,SEEK_END);
      filesize=ftell(fp);
      fseek(fp,0,SEEK_SET);
      filememory=(UBYTE*) new UBYTE[filesize];

      if(fread(filememory,sizeof(char),filesize,fp)!=filesize) {
         CLynxException lynxerr;
         delete filememory;

         lynxerr.Message() << "Handy Error: Unspecified Load error (Header)";
         lynxerr.Description()
               << "The lynx emulator will not run without a cartridge image." << endl
               << "It appears that your cartridge image may be corrupted or there is" << endl
               << "some other error.(see the Handy User Guide for more information)";
         throw(lynxerr);
      }

      fclose(fp);
   }

   // Now try and determine the filetype we have opened
   if(filesize) {
      char clip[11];
      memcpy(clip,filememory,11);
      clip[4]=0;
      clip[10]=0;

      if(!strcmp(&clip[6],"BS93")) mFileType=HANDY_FILETYPE_HOMEBREW;
      else if(!strcmp(&clip[0],"LYNX")) mFileType=HANDY_FILETYPE_LNX;
      else if(!strcmp(&clip[0],LSS_VERSION_OLD)) mFileType=HANDY_FILETYPE_SNAPSHOT;
      else if(filesize==128*1024 || filesize==256*1024 || filesize==512*1024) {
         fprintf(stderr, "Invalid Cart (type). but 128/256/512k size -> set to RAW and try to load raw rom image\n");
         mFileType=HANDY_FILETYPE_RAW;
         //delete filememory;// WHY????? -> crash!
      } else {
         CLynxException lynxerr;
         delete filememory;
         mFileType=HANDY_FILETYPE_ILLEGAL;
         lynxerr.Message() << "Handy Error: File format invalid!";
         lynxerr.Description()
               << "The image you selected was not a recognised game cartridge format." << endl
               << "(see the Handy User Guide for more information).";
         throw(lynxerr);
      }
   }

   mCycleCountBreakpoint=0xffffffff;

// Create the system objects that we'll use

   // Attempt to load the cartridge errors caught above here...

   mRom = new CRom(romfile);

   // An exception from this will be caught by the level above
   mEEPROM = new CEEPROM();

   switch(mFileType) {
      case HANDY_FILETYPE_RAW:
      case HANDY_FILETYPE_LNX:
         mCart = new CCart(filememory,filesize);
         if(mCart->CartHeaderLess()) {
            // veryvery strange Howard Check CANNOT work, as there are two different loader-less card types...
            // unclear HOW this should do anything useful...
            FILE	*fp;
            char drive[3],dir[256],cartgo[256];
            mFileType=HANDY_FILETYPE_HOMEBREW;
            _splitpath(romfile,drive,dir,NULL,NULL);
            strcpy(cartgo,drive);
            strcat(cartgo,dir);
            strcat(cartgo,"howard.o");

            // Open the howard file for reading
            if((fp=fopen(cartgo,"rb"))==NULL) {
               CLynxException lynxerr;
               delete filememory;
               lynxerr.Message() << "Handy Error: Howard.o File Open Error";
               lynxerr.Description()
                     << "Headerless cartridges need howard.o bootfile to ." << endl
                     << "be able to run correctly, could not open file. " << endl;
               throw(lynxerr);
            }

            // How big is the file ??
            fseek(fp,0,SEEK_END);
            howardsize=ftell(fp);
            fseek(fp,0,SEEK_SET);
            howardmemory=(UBYTE*) new UBYTE[filesize];

            if(fread(howardmemory,sizeof(char),howardsize,fp)!=howardsize) {
               CLynxException lynxerr;
               delete filememory;
               delete howardmemory;
               lynxerr.Message() << "Handy Error: Howard.o load error (Header)";
               lynxerr.Description()
                     << "Howard.o could not be read????." << endl;
               throw(lynxerr);
            }

            fclose(fp);

            // Pass it to RAM to load
            mRam = new CRam(howardmemory,howardsize);
         } else {
            mRam = new CRam(0,0);
         }
         break;
      case HANDY_FILETYPE_HOMEBREW:
         mCart = new CCart(0,0);
         mRam = new CRam(filememory,filesize);
         break;
      case HANDY_FILETYPE_SNAPSHOT:
      case HANDY_FILETYPE_ILLEGAL:
      default:
         mCart = new CCart(0,0);
         mRam = new CRam(0,0);
         break;
   }

   // These can generate exceptions

   mMikie = new CMikie(*this);
   mSusie = new CSusie(*this);

// Instantiate the memory map handler

   mMemMap = new CMemMap(*this);

// Now the handlers are set we can instantiate the CPU as is will use handlers on reset

   mCpu = new C65C02(*this);

// Now init is complete do a reset, this will cause many things to be reset twice
// but what the hell, who cares, I don't.....

   Reset();

// If this is a snapshot type then restore the context

   if(mFileType==HANDY_FILETYPE_SNAPSHOT) {
      if(!ContextLoad(gamefile)) {
         Reset();
         CLynxException lynxerr;
         lynxerr.Message() << "Handy Error: Snapshot load error" ;
         lynxerr.Description()
               << "The snapshot you selected could not be loaded." << endl
               << "(The file format was not recognised by Handy)." << endl ;
         throw(lynxerr);
      }
   }
   if(filesize) delete filememory;
   if(howardsize) delete howardmemory;
   mEEPROM->SetEEPROMType(mCart->mEEPROMType);
}

CSystem::~CSystem()
{
   // Cleanup all our objects

   if(mEEPROM!=NULL) delete mEEPROM;
   if(mCart!=NULL) delete mCart;
   if(mRom!=NULL) delete mRom;
   if(mRam!=NULL) delete mRam;
   if(mCpu!=NULL) delete mCpu;
   if(mMikie!=NULL) delete mMikie;
   if(mSusie!=NULL) delete mSusie;
   if(mMemMap!=NULL) delete mMemMap;
}

bool CSystem::IsZip(char *filename)
{
   UBYTE buf[2];
   FILE *fp;

   if((fp=fopen(filename,"rb"))!=NULL) {
      fread(buf, 2, 1, fp);
      fclose(fp);
      return(memcmp(buf,"PK",2)==0);
   }
   if(fp)fclose(fp);
   return FALSE;
}

void CSystem::HLE_BIOS_FE00(void)
{
   // Select Block in A
   C6502_REGS regs;
   mCpu->GetRegs(regs);
   mCart->SetShifterValue(regs.A);
   // we just put an RTS behind in fake ROM!
}

void CSystem::HLE_BIOS_FE19(void)
{
   // (not) initial jump from reset vector
   // Clear full 64k memory!
   mRam->Clear();

   // Set Load adresse to $200 ($05,$06)
   mRam->Poke(0x0005,0x00);
   mRam->Poke(0x0006,0x02);
   // Call to $FE00
   mCart->SetShifterValue(0);
   // Fallthrou $FE4A
   HLE_BIOS_FE4A();
}

void CSystem::HLE_BIOS_FE4A(void)
{
   UWORD addr=mRam->Peek(0x0005) | (mRam->Peek(0x0006)<<8);

   // Load from Cart (loader blocks)
   unsigned char buff[256];// maximum 5 blocks
   unsigned char res[256];

   buff[0]=mCart->Peek0();
   int blockcount = 0x100 -  buff[0];

   for (int i = 1; i < 1+51*blockcount; ++i) { // first encrypted loader
      buff[i] = mCart->Peek0();
   }
   printf("\n");

   lynx_decrypt(res, buff, 51);

   for (int i = 0; i < 50*blockcount; ++i) {
      Poke_CPU(addr++, res[i]);
   }

   // Load Block(s), decode to ($05,$06)
   // jmp $200

   C6502_REGS regs;
   mCpu->GetRegs(regs);
   regs.PC=0x0200;
   mCpu->SetRegs(regs);
}

void CSystem::HLE_BIOS_FF80(void)
{
   // initial jump from reset vector ... calls FE19
   HLE_BIOS_FE19();
}


void CSystem::Reset(void)
{
   gSystemCycleCount=0;
   gNextTimerEvent=0;
   gCPUBootAddress=0;
   gBreakpointHit=FALSE;
   gSingleStepMode=FALSE;
   gSingleStepModeSprites=FALSE;
   gSystemIRQ=FALSE;
   gSystemNMI=FALSE;
   gSystemCPUSleep=FALSE;
   gSystemHalt=FALSE;

   gThrottleLastTimerCount=0;
   gThrottleNextCycleCheckpoint=0;

   gTimerCount=0;

   gAudioBufferPointer=0;
   gAudioLastUpdateCycle=0;
//	memset(gAudioBuffer,128,HANDY_AUDIO_BUFFER_SIZE); // only for unsigned 8bit
   memset(gAudioBuffer,0,HANDY_AUDIO_BUFFER_SIZE); // for unsigned 8/16 bit

#ifdef _LYNXDBG
   gSystemHalt=TRUE;
#endif

   mMemMap->Reset();
   mCart->Reset();
   mEEPROM->Reset();
   mRom->Reset();
   mRam->Reset();
   mMikie->Reset();
   mSusie->Reset();
   mCpu->Reset();

   // Homebrew hashup

   if(mFileType==HANDY_FILETYPE_HOMEBREW) {
      mMikie->PresetForHomebrew();

      C6502_REGS regs;
      mCpu->GetRegs(regs);
      regs.PC=(UWORD)gCPUBootAddress;
      mCpu->SetRegs(regs);
   } else {
      if(!mRom->mValid) {
         mMikie->PresetForHomebrew();
         mRom->mWriteEnable=true;

         mRom->Poke(0xFE00+0,0x8d);
         mRom->Poke(0xFE00+1,0x97);
         mRom->Poke(0xFE00+2,0xfd);
         mRom->Poke(0xFE00+3,0x60);// RTS

         mRom->Poke(0xFE19+0,0x8d);
         mRom->Poke(0xFE19+1,0x97);
         mRom->Poke(0xFE19+2,0xfd);

         mRom->Poke(0xFE4A+0,0x8d);
         mRom->Poke(0xFE4A+1,0x97);
         mRom->Poke(0xFE4A+2,0xfd);

         mRom->Poke(0xFF80+0,0x8d);
         mRom->Poke(0xFF80+1,0x97);
         mRom->Poke(0xFF80+2,0xfd);

         mRom->mWriteEnable=false;
      }
   }
}

bool CSystem::ContextSave(char *context)
{
   FILE *fp;
   bool status=1;

   if((fp=fopen(context,"wb"))==NULL) return false;

   if(!fprintf(fp,LSS_VERSION)) status=0;

   // Save ROM CRC
   ULONG checksum=mCart->CRC32();
   if(!fwrite(&checksum,sizeof(ULONG),1,fp)) status=0;

   if(!fprintf(fp,"CSystem::ContextSave")) status=0;

   if(!fwrite(&mCycleCountBreakpoint,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gSystemCycleCount,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gNextTimerEvent,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gCPUWakeupTime,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gCPUBootAddress,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gIRQEntryCycle,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gBreakpointHit,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gSingleStepMode,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gSystemIRQ,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gSystemNMI,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gSystemCPUSleep,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gSystemCPUSleep_Saved,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gSystemHalt,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gThrottleMaxPercentage,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gThrottleLastTimerCount,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gThrottleNextCycleCheckpoint,sizeof(ULONG),1,fp)) status=0;

   ULONG tmp=gTimerCount;
   if(!fwrite(&tmp,sizeof(ULONG),1,fp)) status=0;

   if(!fwrite(gAudioBuffer,sizeof(UBYTE),HANDY_AUDIO_BUFFER_SIZE,fp)) status=0;
   if(!fwrite(&gAudioBufferPointer,sizeof(ULONG),1,fp)) status=0;
   if(!fwrite(&gAudioLastUpdateCycle,sizeof(ULONG),1,fp)) status=0;

   // Save other device contexts
   if(!mMemMap->ContextSave(fp)) status=0;
   if(!mCart->ContextSave(fp)) status=0;
   if(!mEEPROM->ContextSave(fp)) status=0;
//	if(!mRom->ContextSave(fp)) status=0; We no longer save the system ROM
   if(!mRam->ContextSave(fp)) status=0;
   if(!mMikie->ContextSave(fp)) status=0;
   if(!mSusie->ContextSave(fp)) status=0;
   if(!mCpu->ContextSave(fp)) status=0;

   fclose(fp);
   return status;
}


bool CSystem::ContextLoad(char *context)
{
   LSS_FILE *fp;
   bool status=1;
   UBYTE *filememory=NULL;
   ULONG filesize=0;

   // First check for ZIP file
   if(IsZip(context)) {
      // Find the file and read into memory
      // Try and find a file in the zip
      unzFile *fp;
      unz_file_info info;
      char filename_buf[0x100], *ptr;
      bool gotIt;

      if((fp=(unzFile*)unzOpen(context))!=NULL) {
         if(unzGoToFirstFile(fp)!=UNZ_OK) {
            unzClose(fp);
            gError->Warning("ContextLoad(): ZIP File select problems, could not read zip file");
            return 1;
         }

         gotIt = FALSE;
         for (;;) {
            // Get file descriptor and analyse
            if(unzGetCurrentFileInfo(fp, &info, filename_buf, 0x100, NULL, 0, NULL, 0) != UNZ_OK) {
               break;
            } else {
               ptr = strchr(filename_buf, '.');
               if (ptr != NULL) {
                  char buf[4];

                  ptr++;
                  buf[0] = tolower(*ptr);
                  ptr++;
                  buf[1] = tolower(*ptr);
                  ptr++;
                  buf[2] = tolower(*ptr);
                  buf[3] = 0;
                  if (!strcmp(buf, "lss")) {
                     // Found a likely file so signal
                     gotIt = TRUE;
                     break;
                  }
               }

               // No match so lets try the next file
               if(unzGoToNextFile(fp)!=UNZ_OK)	break;
            }
         }

         // Did we strike gold ?
         if(gotIt) {
            if(unzOpenCurrentFile(fp)==UNZ_OK) {
               // Allocate memory for the rom
               filesize=info.uncompressed_size;
               filememory=(UBYTE*) new UBYTE[filesize];

               // Read it into memory
               if(unzReadCurrentFile(fp,filememory,filesize)!=(int)info.uncompressed_size) {
                  unzCloseCurrentFile(fp);
                  unzClose(fp);
                  delete filememory;
                  // Throw a wobbly
                  gError->Warning("ContextLoad(): ZIP File load problems, could not read data from the zip file");
                  return 1;
               }

               // Got it!
               unzCloseCurrentFile(fp);
               unzClose(fp);
            }

         } else {
            gError->Warning("ContextLoad(): ZIP File load problems, could not find an LSS file in the zip archive");
            return 1;
         }
      } else {
         gError->Warning("ContextLoad(): ZIP File load problems, could not open the zip archive");
         return 1;
      }

   } else {
      FILE *fp;
      // Just open an read into memory
      if((fp=fopen(context,"rb"))==NULL) status=0;

      fseek(fp,0,SEEK_END);
      filesize=ftell(fp);
      fseek(fp,0,SEEK_SET);
      filememory=(UBYTE*) new UBYTE[filesize];

      if(fread(filememory,sizeof(char),filesize,fp)!=filesize) {
         fclose(fp);
         return 1;
      }
      fclose(fp);
   }

   // Setup our read structure
   fp = new LSS_FILE;
   fp->memptr=filememory;
   fp->index=0;
   fp->index_limit=filesize;

   char teststr[100];
   // Check identifier
   if(!lss_read(teststr,sizeof(char),4,fp)) status=0;
   teststr[4]=0;

   if(strcmp(teststr,LSS_VERSION)==0 || strcmp(teststr,LSS_VERSION_OLD)==0) {
      bool legacy=FALSE;
      if(strcmp(teststr,LSS_VERSION_OLD)==0) {
         legacy=TRUE;
      } else {
         ULONG checksum;
         // Read CRC32 and check against the CART for a match
         lss_read(&checksum,sizeof(ULONG),1,fp);
         if(mCart->CRC32()!=checksum) {
            delete fp;
            delete filememory;
            gError->Warning("LSS Snapshot CRC does not match the loaded cartridge image, aborting load");
            return 0;
         }
      }

      // Check our block header
      if(!lss_read(teststr,sizeof(char),20,fp)) status=0;
      teststr[20]=0;
      if(strcmp(teststr,"CSystem::ContextSave")!=0) status=0;

      if(!lss_read(&mCycleCountBreakpoint,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemCycleCount,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gNextTimerEvent,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gCPUWakeupTime,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gCPUBootAddress,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gIRQEntryCycle,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gBreakpointHit,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSingleStepMode,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemIRQ,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemNMI,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemCPUSleep,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemCPUSleep_Saved,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemHalt,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gThrottleMaxPercentage,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gThrottleLastTimerCount,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gThrottleNextCycleCheckpoint,sizeof(ULONG),1,fp)) status=0;

      ULONG tmp;
      if(!lss_read(&tmp,sizeof(ULONG),1,fp)) status=0;
      gTimerCount=tmp;

      if(!lss_read(gAudioBuffer,sizeof(UBYTE),HANDY_AUDIO_BUFFER_SIZE,fp)) status=0;
      if(!lss_read(&gAudioBufferPointer,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gAudioLastUpdateCycle,sizeof(ULONG),1,fp)) status=0;

      if(!mMemMap->ContextLoad(fp)) status=0;
      // Legacy support
      if(legacy) {
         if(!mCart->ContextLoadLegacy(fp)) status=0;
         if(!mRom->ContextLoad(fp)) status=0;
      } else {
         if(!mCart->ContextLoad(fp)) status=0;
      }
      if(!mRam->ContextLoad(fp)) status=0;
      if(!mMikie->ContextLoad(fp)) status=0;
      if(!mSusie->ContextLoad(fp)) status=0;
      if(!mCpu->ContextLoad(fp)) status=0;
   } else {
      gError->Warning("Not a recognised LSS file");
   }

   delete fp;
   delete filememory;

   return status;
}

#ifdef _LYNXDBG

void CSystem::DebugTrace(int address)
{
   char message[1024+1];
   int count=0;

   sprintf(message,"%08x - DebugTrace(): ",gSystemCycleCount);
   count=strlen(message);

   if(address) {
      if(address==0xffff) {
         C6502_REGS regs;
         char linetext[1024];
         // Register dump
         GetRegs(regs);
         sprintf(linetext,"PC=$%04x SP=$%02x PS=0x%02x A=0x%02x X=0x%02x Y=0x%02x",regs.PC,regs.SP, regs.PS,regs.A,regs.X,regs.Y);
         strcat(message,linetext);
         count=strlen(message);
      } else {
         // The RAM address contents should be dumped to an open debug file in this function
         do {
            message[count++]=Peek_RAM(address);
         } while(count<1024 && Peek_RAM(address++)!=0);
      }
   } else {
      strcat(message,"CPU Breakpoint");
      count=strlen(message);
   }
   message[count]=0;

   // Callback to dump the message
   if(mpDebugCallback) {
      (*mpDebugCallback)(mDebugCallbackObject,message);
   }
}

void CSystem::DebugSetCallback(void (*function)(ULONG objref,char *message),ULONG objref)
{
   mDebugCallbackObject=objref;
   mpDebugCallback=function;
}


#endif
