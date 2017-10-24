/**************************************************************************

   bufr3_ftn.c - Fortran Interface for BUFR3 library for DWD

   Version: 1.0

   Author: Dr. Rainer Johanni, Mar. 2003

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
extern int errno;

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#include "bufr3.h"                                                        /* martin org: <bufr3.h> */
#include "bufr3_table_a.h"            /* for bufr3_get_header_text() */   /* martin  org: <bufr3_table_a.h> */


#define MAXHANDLES 10    /* for bufr handles */
#define MAXUNITS 10      /* for file units */

/* hflag, uflag: flag arrays for bufr handles and file units:

   hflag[ih] == 0:  Handle is free
   hflag[ih] == 1:  Handle is connected with BUFR data which has been read
   hflag[ih] == 2:  Handle is connected with BUFR data which has been encoded

   uflag[ih] == 0:  Unit is free
   uflag[ih] == 1:  Unit is connected with input file
   uflag[ih] == 2:  Unit is connected with output file in raw mode;
   uflag[ih] == 3:  Unit is connected with output file in "DWD-Mode"
*/


static int hflag[MAXHANDLES] = { 0,0,0,0,0,0,0,0,0,0 };
static int uflag[MAXUNITS]   = { 0,0,0,0,0,0,0,0,0,0 };
static char fname[MAXUNITS][MAXPATHLEN+1];

static bufr3data *(bufdes_arr[MAXHANDLES]);
static FILE *(fd_arr[MAXUNITS]);

static char error_string[4096];

/*
   strip_blanks: Help routine for converting FORTRAN characters to C chars:

   converts all trailing blanks of a (not zero terminated) string s to 0s

   s must have at least space for len+1 characters!
*/

static void strip_blanks(char *s, int len)
{
   int i;

   s[len] = 0; /* Terminate string if it should be fully populated */
   for(i=len-1;i>=0;i--)
   {
      if(s[i] != ' ') break;
      s[i] = 0;
   }
}

/*
   malloc_check:
   Performs a malloc, checks if it fails.
   In the failure case, sets also error_string and the ier parameter.
*/
static void *malloc_check(int len, FTN_INT *ier)
{
   void *data;

   /* if ier is already set, don't try to do anything */

   if(*ier) return 0;

   data = (unsigned char *) malloc(len);
   if(!data) {
      sprintf(error_string,"malloc failed");
      *ier=BUFR3_ERR_MALLOC;
   }
   return data;
}

/*
   get_handle: searches for free BUFR handle, returns handle number.
   In case of error: sets error_string and *ier, returns -1.
*/

static int get_handle(FTN_INT *ier)
{
   int ih;

   for(ih=0;ih<MAXHANDLES;ih++) if(hflag[ih] == 0) break;
   if(ih>=MAXHANDLES)
   {
      sprintf(error_string,"Too many used bufr handles (Forgot to call BUFR3_DESTROY?)");
      *ier = BUFR3_ERR_HANDLE_UNIT;
      return -1;
   }

   return ih;
}

/*
   check_handle: checks if ih is a valid BUFR handle.

   flag == 0 : Check only if handle is valid
   flag == 1 : Check if this is a BUFR for decoding
   flag == 2 : Check if this is a BUFR for decoding and if it is decoded

   In case of error: sets error_string and *ier (if present), returns -1.
   Returns 0 if handle is ok.
*/

static int check_handle(int ih, int flag, FTN_INT *ier)
{
   if(ih<0 || ih>=MAXHANDLES || !hflag[ih] || (flag>0 && hflag[ih]!=1) )
   {
      sprintf(error_string,"Illegal BUFR handle");
      if(ier) *ier = BUFR3_ERR_HANDLE_UNIT;
      return -1;
   }

   if(flag==2 && !bufdes_arr[ih]->results)
   {
      sprintf(error_string,"Bufr is not yet decoded");
      if(ier) *ier = BUFR3_ERR_CALLSEQ;
      return -1;
   }

   return 0;
}

/*
   resort_results set index to the permutation of the BUFR elements
   when results are resorted. If specified, loop info is left
   away during this process.
*/

/* the following 2 variables must be set to 0 before resort_results is called */

static int nent; /* index of entries in bufdes->results */
static int nind; /* index of entries in "index" */

static void resort_results(bufr3data *bufdes, int nsubset, int *index)
{
   int nstep, start=nent; /* starting entry */

   /* Go through the data in 3 steps:
      step 0: gather all data not in repetition lists
      step 1: gather all data in repetition lists with constant count
      step 2: gather all data in repetition lists with variable count
   */

   for(nstep=0;nstep<3;nstep++)
   {
      nent = start;
      while(nent<bufdes->num_results[nsubset])
      {
         if(BUFR3_DESC(bufdes,nsubset,nent).type == ED_LOOP_COUNTER ||
            BUFR3_DESC(bufdes,nsubset,nent).type == ED_LOOP_END)
         {
            /* we are done with searching for this step in this recursion level */

            break;
         }

         if(BUFR3_DESC(bufdes,nsubset,nent).type == ED_LOOP_START)
         {
            /* Start of a loop:
               Process the content of the loop if we are in step 1
               and it has a fixed rep. count or in step 2 and it has
               a variable rep. count.
               Skip it otherways.
               The data part of the ED_LOOP_START entry has the number
               of items in the loop (not counting ED_LOOP_END) */

            if((nstep==1 && BUFR3_DESC(bufdes,nsubset,nent).Y != 0) ||
               (nstep==2 && BUFR3_DESC(bufdes,nsubset,nent).Y == 0) )
            {
               /* call resort_results recursivly until end of loop is reached
                  i.e. the last entry desc encounterd is not ED_LOOP_COUNTER.
                  It should be ED_LOOP_END. */

               if( !(bufdes->ftn_options&FTN_REMOVE_LOOP_INFO) ) index[nind++] = nent;
               nent++; /* skip over ED_LOOP_START */
               do {
                  resort_results(bufdes,nsubset,index);

                  /* The following is for safety only and could only happen
                     if something is corrupted - we do it here in order not to
                     be caught in an endless loop */
                  if(nent>=bufdes->num_results[nsubset]) break;

                  /* The entry at nent must always be ED_LOOP_END or ED_LOOP_COUNTER */
                  if(!(bufdes->ftn_options&FTN_REMOVE_LOOP_INFO)) index[nind++] = nent;
                  nent++;
               } while (BUFR3_DESC(bufdes,nsubset,nent-1).type == ED_LOOP_COUNTER);
            }
            else
            {
               /* Skip */

               nent += BUFR3_DATA(bufdes,nsubset,nent)+2;
            }
         }
         else
         {
            /* Item not in repetition list - store it in index if nstep==0 */
            if(nstep==0) index[nind++] = nent;
            nent++;
         }
      }
   }
}
         



/**************************************************************************
 *
 * File handling:
 *
 * CALL bufr3_open_file(filename,output,raw,iunit,ier)
 * CALL bufr3_close_file(iunit,status,ier)
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   bufr3_open_file: opens BUFR3 file for reading or writing

   --- Input Parameters:

   filename    filename of file to open.
               A filename consisting of a single "-" reads from stdin
               or writes to stdout

   output      Flag for opening input/output file.
               If set to 0, opens file for input else opens file for output.

   raw         Flag for raw output mode (no meaning for input files).
               If set to 0, output is in DWD format, else output is raw.
               DWD format:
               4 leading 0-bytes, every buffer has 4 bytes containg its length
               in big-endian format before and after it's data, 4 trailing 0-bytes
               RAW format:
               BUFRs are written without any gap to the output file.

   --- Output Parameters:

   iunit       returned unit number, a number from 0 .. MAXUNITS-1 on success
   ier         error indicator

   --- Added by FORTRAN:

   l_filename: FORTRAN length of filename
*/

void bufr3_open_file_
   (char *filename, FTN_INT *output, FTN_INT *raw, FTN_INT *iunit, FTN_INT *ier,
    unsigned long l_filename)
{
   int iu;
   char cfilename[MAXPATHLEN+1];

   *ier = 0;
   *iunit = -1;

   /* Search free file unit */

   for(iu=0;iu<MAXUNITS;iu++) if(uflag[iu] == 0) break;
   if(iu>=MAXUNITS)
   {
      sprintf(error_string,"Too many open file units");
      *ier = BUFR3_ERR_HANDLE_UNIT;
      return;
   }

   if(l_filename>MAXPATHLEN) l_filename = MAXPATHLEN; /* Safety first */
   memcpy(cfilename,filename,l_filename);
   strip_blanks(cfilename,l_filename);

   if(strcmp(cfilename,"-")==0)
   {
      /* Read from stdin/write to stdout */

      if(*output) {
         fd_arr[iu] = stdout;
      } else {
         fd_arr[iu] = stdin;
      }
   }
   else
   {
      if(*output) {
         fd_arr[iu] = fopen(cfilename,"wb");
      } else {
         fd_arr[iu] = fopen(cfilename,"rb");
      }

      if(!fd_arr[iu])
      {
        sprintf(error_string,"Error opening %s, reason: %s",cfilename,strerror(errno));
        *ier = BUFR3_ERR_OPEN;
        return;
      }
   }

   *iunit = iu;

   if(*output) {
      if(*raw) {
         uflag[iu] = 2;  /* Raw output file */
      } else {
         uflag[iu] = 3;  /* DWD-Mode output file */
      }
   } else {
      uflag[iu] = 1;     /* Input file */
   }

   strcpy(fname[iu],cfilename);
}

/* ------------------------------------------------------------------------
   bufr3_close_file: Closes BUFR file

   iunit   File unit number (input)
   status  Fortran Character variable - if this variables starts with 'del'
           (case insensitive) the file is deleted after close, in all other
           cases the file is left (input)
   ier     error indicator
*/

void bufr3_close_file_
   (FTN_INT *iunit, char *status, FTN_INT *ier, unsigned long l_status)
{
   int iu=*iunit, res;

   *ier = 0;

   if(iu<0 || iu>=MAXUNITS || !uflag[iu] )
   {
      sprintf(error_string,"Illegal File Unit");
      *ier = BUFR3_ERR_HANDLE_UNIT;
      return;
   }

   if(uflag[iu] == 3)
   {
      res = BUFR3_close_dwd(fd_arr[iu]);
      if(res) *ier = -res;
   }
   else
   {
      fclose(fd_arr[iu]);
   }

   uflag[iu] = 0;

   /* delete file if this is wanted */

   if( strcmp(fname[iu],"-")!=0 && l_status>=3 &&
       (status[0]=='D' || status[0]=='d') &&
       (status[1]=='E' || status[1]=='e') &&
       (status[2]=='L' || status[2]=='l') )
   {
      if(remove(fname[iu])<0) {
         sprintf(error_string,"Error deleting %s, reason: %s",fname[iu],strerror(errno));
         *ier = BUFR3_ERR_DELETE;
         return;
      }
   }

   fname[iu][0] = 0; /* For safety only */
}

/**************************************************************************
 *
 * I/O Routines
 *
 * CALL bufr3_read_bufr(iunit,max_chars,ihandle,ieof,ier)
 * CALL bufr3_write_bufr(iunit,ihandle,ier)
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   bufr3_read_bufr: Reads next BUFR, allocates Bufr structure by calling
                    BUFR3_decode_sections

   --- Input Parameters:

   iunit           File unit from where the BUFR has to be read
   max_chars       Maximum number of characters to skip in input file
                   before the characters BUFR are found

   --- Output Parameters

   ihandle         BUFR handle
   ieof            Set to 1 on EOF, to 0 else
   ier             error indicator
*/

void bufr3_read_bufr_
   (FTN_INT *iunit, FTN_INT *max_chars, FTN_INT *ihandle, FTN_INT *ieof, FTN_INT *ier)
{
   int iu=*iunit, ih, res;
   unsigned char *data;

   *ier = 0;
   *ieof = 0;
   *ihandle=-1;

   if(iu<0 || iu>=MAXUNITS || uflag[iu]!=1 )
   {
      sprintf(error_string,"Illegal File Unit");
      *ier = BUFR3_ERR_HANDLE_UNIT;
      return;
   }

   /* search free BUFR handle */

   if((ih=get_handle(ier)) < 0) return;

   /* Read BUFR */

   res = BUFR3_read_record(fd_arr[iu], *max_chars, &data);
   if(res) { *ier = -res; return; }

   if(!data) { *ieof = 1; return; }

   /* call BUFR3_decode_sections for allocating a bufr3data object */

   res = BUFR3_decode_sections(data,&bufdes_arr[ih]);
   if(res) { free(data); *ier = -res; return; }

   hflag[ih] = 1; /* BUFR read from file */
   *ihandle = ih;
}


/* ------------------------------------------------------------------------
   bufr3_write_bufr: Write BUFR to output file

   --- Input Parameters:

   iunit          File unit
   ihandle        BUFR handle

   --- Output Parameters

   ier            error indicator
*/


void bufr3_write_bufr_
   (FTN_INT *iunit, FTN_INT *ihandle, FTN_INT *ier)
{
   int iu=*iunit, ih=*ihandle, res;

   *ier = 0;

   if(iu<0 || iu>=MAXUNITS || uflag[iu]<=1 )
   {
      sprintf(error_string,"Illegal File Unit");
      *ier = BUFR3_ERR_HANDLE_UNIT;
      return;
   }

   if(check_handle(ih,0,ier)<0) return;

   if(uflag[iu] == 2)
   {
      res = BUFR3_write_raw(fd_arr[iu],bufdes_arr[ih]);
   }
   else
   {
      res = BUFR3_write_dwd(fd_arr[iu],bufdes_arr[ih]);
   }
   if(res) *ier = -res;
}

/**************************************************************************
 *
 * Routines for retrieving common data of a BUFR
 * May be called for read and encoded BUFRs
 *
 * CALL bufr3_get_num_bytes(ihandle,num)
 * CALL bufr3_get_num_subsets(ihandle,num)
 * CALL bufr3_get_num_descriptors(ihandle,num)
 * CALL bufr3_get_section_length(ihandle,isect,num)
 * CALL bufr3_get_section(ihandle,isect,ibufsec,ier)
 * CALL bufr3_get_sections(ihandle,ibufsec0,ibufsec1,ibufsec2,ibufsec3,ibufsec4,ier)
 * CALL bufr3_get_descriptors(ihandle,idescr,ier)
 * CALL bufr3_get_header_text(ihandle,text,ier)
 * CALL bufr3_get_bufr(ihandle,iswap,ibufr,ier)
 *
 * Reverse of bufr3_get_bufr: Putting externally read data into a BUFR handle
 *
 * CALL bufr3_put_bufr(ibufr,ihandle,ier)
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   bufr3_get_num_bytes:       Get total number of bytes of BUFR data
   bufr3_get_num_subsets:     Get number of subsets in BUFR
   bufr3_get_num_descriptors: Get number of descriptors in section 3
   bufr3_get_section_length:  Get length of section 0-5 (especially for
                              a subsequent call to bufr3_get_section)

   --- Input Parameters:

   ihandle         BUFR handle
   isect           Section number (0-5, bufr3_get_section_length only)

   --- Output Parameters

   num             The value wanted

   Since these are very simple routines, there is no IER parameter.
   num is set to 0 if an illegal BUFR handle is passed, no other
   errors are possible
*/

void bufr3_get_num_bytes_
   (FTN_INT *ihandle, FTN_INT *num)
{
   int ih=*ihandle;

   *num = 0;
   if(check_handle(ih,0,0)<0) return;
   *num = bufdes_arr[ih]->nbytes;
}

void bufr3_get_num_subsets_
   (FTN_INT *ihandle, FTN_INT *num)
{
   int ih=*ihandle;

   *num = 0;
   if(check_handle(ih,0,0)<0) return;
   *num = bufdes_arr[ih]->num_subsets;
}

void bufr3_get_num_descriptors_
   (FTN_INT *ihandle, FTN_INT *num)
{
   int ih=*ihandle;

   *num = 0;
   if(check_handle(ih,0,0)<0) return;
   *num = bufdes_arr[ih]->num_descriptors;
}

void bufr3_get_section_length_
   (FTN_INT *ihandle, FTN_INT *isect, FTN_INT *num)
{
   int ih=*ihandle;

   *num = 0;
   if(check_handle(ih,0,0)<0) return;

   switch(*isect)
   {
      case 0: *num = 8; break;
      case 1: *num = bufdes_arr[ih]->sect1_length; break;
      case 2: *num = bufdes_arr[ih]->sect2_length; break;
      case 3: *num = bufdes_arr[ih]->sect3_length; break;
      case 4: *num = bufdes_arr[ih]->sect4_length; break;
      case 5: *num = 4; break;
      default: *num = 0;
   }
}

/* ------------------------------------------------------------------------
   bufr3_get_section: Get one of the sections 0-5 as is (without processing
                      like it is done in bufr3_get_sections).

   --- Input Parameters:

   ihandle         BUFR handle
   isect           section number (0-5, the routine does nothing if isect
                   is outside this range)

   --- Output Parameters

   ibufsec         Recieves the bytes of the section,
                   1 Byte per Fortran variable
   ier             error indicator
*/

void bufr3_get_section_
   (FTN_INT *ihandle, FTN_INT *isect, FTN_INT *ibufsec, FTN_INT *ier)
{
   int i, ih=*ihandle, num;
   bufr3data *bufdes;
   unsigned char *s;

   *ier = 0;

   if(check_handle(ih,0,ier)<0) return;
   bufdes = bufdes_arr[ih];

   switch(*isect)
   {
      case 0: num = 8; s = bufdes->bufr_data; break;
      case 1: num = bufdes_arr[ih]->sect1_length; s = bufdes->sect1; break;
      case 2: num = bufdes_arr[ih]->sect2_length; s = bufdes->sect2; break;
      case 3: num = bufdes_arr[ih]->sect3_length; s = bufdes->sect3; break;
      case 4: num = bufdes_arr[ih]->sect4_length; s = bufdes->sect4; break;
      case 5: num = 4; s = bufdes->sect5; break;
      default: return;
   }

   for(i=0;i<num;i++) ibufsec[i] = s[i];
}


/* ------------------------------------------------------------------------
   bufr3_get_sections: Get data of BUFR sections 1-4

   --- Input Parameters:

   ihandle         BUFR handle

   --- Output Parameters

   ibufsec0        See description of the FORTRAN BUFR interface
   ibufsec1        See description of the FORTRAN BUFR interface
   ibufsec2        See description of the FORTRAN BUFR interface
   ibufsec3        See description of the FORTRAN BUFR interface
   ibufsec4        See description of the FORTRAN BUFR interface
   ier             error indicator
*/

void bufr3_get_sections_
   (FTN_INT *ihandle, FTN_INT *ibufsec0, FTN_INT *ibufsec1, FTN_INT *ibufsec2,
    FTN_INT *ibufsec3, FTN_INT *ibufsec4, FTN_INT *ier)
{
   int i, ih=*ihandle;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,0,ier)<0) return;
   bufdes = bufdes_arr[ih];

   ibufsec0[0] = bufdes->nbytes;
   ibufsec0[1] = bufdes->iednr;

   ibufsec1[ 0] = bufdes->sect1_length;
   for(i=1;i<15;i++) ibufsec1[i] = bufdes->sect1[i+2];

   for(i=0;i<12;i++) ibufsec2[i] = 0;
   ibufsec2[ 0] = bufdes->sect2_length;
   if(bufdes->sect2)
   {
      ibufsec2[ 1] = 0;
      ibufsec2[ 2] = bufdes->sect2[4];
      ibufsec2[ 3] = bufdes->sect2[5];
      ibufsec2[ 4] = bufdes->sect2[6]*256*256 +  bufdes->sect2[7]*256 +  bufdes->sect2[8];
      ibufsec2[ 5] = bufdes->sect2[9];
      ibufsec2[ 6] = bufdes->sect2[10];
      ibufsec2[ 7] = bufdes->sect2[11];
      ibufsec2[ 8] = bufdes->sect2[12];
      ibufsec2[ 9] = bufdes->sect2[13];
      ibufsec2[10] = bufdes->sect2[14];
      if(bufdes->sect2_length>=18)
         ibufsec2[11] = bufdes->sect2[15]*256 + bufdes->sect2[16];
      else
         ibufsec2[11] = BUFR3_UNDEF_VALUE;
   }

   ibufsec3[ 0] = bufdes->sect3_length;
   ibufsec3[ 1] = 0;
   ibufsec3[ 2] = bufdes->num_subsets;
   ibufsec3[ 3] = bufdes->sect3[6];
   ibufsec3[ 4] = bufdes->num_descriptors;

   ibufsec4[ 0] = bufdes->sect4_length;
}

/* ------------------------------------------------------------------------
   bufr3_get_descriptors: Get the descriptors of section 3

   --- Input Parameters:

   ihandle         BUFR handle

   --- Output Parameters

   idescr          Descriptor array. Must be allocated big enough to hold
                   all descriptors.
                   Hint: the number of descriptors is in IBUFSEC3(5)
                   (FORTRAN counting) after a call to bufr3_get_sections
                   or it can be retrieved with bufr3_get_num_descriptors.
   ier             error indicator
      
*/

void bufr3_get_descriptors_
   (FTN_INT *ihandle, FTN_INT *idescr, FTN_INT *ier)
{
   int i, ih=*ihandle;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,0,ier)<0) return;
   bufdes = bufdes_arr[ih];

   for(i=0;i<bufdes->num_descriptors;i++)
   {
      int F, X, Y;
      F = ((bufdes->sect3[2*i+7])>>6) & 0x3;
      X = (bufdes->sect3[2*i+7])&0x3f;
      Y = bufdes->sect3[2*i+8];
      idescr[i] = F*100000 + X*1000 + Y;
   }
}

/* ------------------------------------------------------------------------
   bufr3_get_header_text: Get a text header describing the BUFR.

   If the BUFR has exactly 1 descriptor with F==3, this is
      "3XXYYY" + the Table D description of 3XXYYY
   else it is
      "363255EXTERNAL DATA FORMAT: " + the Table A description (from bufr_table_a.h)

   --- Input Parameters:

   ihandle         BUFR handle

   --- Output Parameters:

   text            Character value
   ier             error indicator

   --- Added by FORTRAN:

   l_text          FORTRAN character length of text
*/


void bufr3_get_header_text_
   (FTN_INT *ihandle, char *text, FTN_INT *ier, unsigned long l_text)
{
   int ih=*ihandle;
   bufr3data *bufdes;
   char header[4096]; /* should be big enough in all cases */

   *ier = 0;

   if(check_handle(ih,0,ier)<0) return;
   bufdes = bufdes_arr[ih];

   if(bufdes->num_descriptors == 1 && (bufdes->descriptors[0] & 0xc0) == 0xc0)
   {
      int X = bufdes->descriptors[0]&0x3f;
      int Y = bufdes->descriptors[1];
      tab_d_entry *tab_d_ptr;
      bufr3_read_tables(bufdes->sect1); /* be sure that the actual tables are set */
      tab_d_ptr = bufr3_get_tab_d_entry(X,Y);
      sprintf(header,"3%2.2d%3.3d%s",X,Y,tab_d_ptr ? tab_d_ptr->text : "???");
   }
   else
   {
      int i, ntaba = bufdes->sect1[8];

      for(i=0;i<256;i++) if(ntaba == tab_a_number[i]) break;

      if(i<256)
      {
         /* Table A entry found */
         sprintf(header,"363255EXTERNAL DATA FORMAT: %s",tab_a_text[i]);
      }
      else
      {
         /* Not found */
         sprintf(header,"363255EXTERNAL DATA FORMAT: RESERVED");
      }
   }

   my_strcpy(text,header,l_text,1);
}

/* ------------------------------------------------------------------------
   bufr3_get_bufr: get raw BUFR data

   --- Input Parameters:

   ihandle        Buffer handle (as returned from bufr3_encode)
   iswap          if set to 0, no byteswapping of the data is performed
                  else the data is swapped on little endian machines
                  in order to stay compatible to other DWD-lib routines

   --- Output Parameters:

   ibufr          BUFR data. Must be allocated big enough to receive the data
   ier            error indicator

*/


void bufr3_get_bufr_
   (FTN_INT *ihandle, FTN_INT *iswap, unsigned char *ibufr, FTN_INT *ier)
{
   int ih=*ihandle, itest, le, i;
   unsigned char *ctest;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,0,ier)<0) return;
   bufdes = bufdes_arr[ih];

   /* Check if we are running on a little endian machine */

   le = 0;
   itest = 0xaa; /* Magic number */
   ctest = (unsigned char *) &itest;
   if(ctest[0] == 0xaa) le = 1;

   /* Copy data */

   if(*iswap && le) {
      int num = bufdes->nbytes;
      if(sizeof(FTN_INT) == 4) {
         for(i=0;i<num-4;i+=4)
         {
            ibufr[i+3] = bufdes->bufr_data[i  ];
            ibufr[i+2] = bufdes->bufr_data[i+1];
            ibufr[i+1] = bufdes->bufr_data[i+2];
            ibufr[i  ] = bufdes->bufr_data[i+3];
         }
         ibufr[i+3] = (i  <num) ? bufdes->bufr_data[i  ] : 0;
         ibufr[i+2] = (i+1<num) ? bufdes->bufr_data[i+1] : 0;
         ibufr[i+1] = (i+2<num) ? bufdes->bufr_data[i+2] : 0;
         ibufr[i  ] = (i+3<num) ? bufdes->bufr_data[i+3] : 0;
      } else if (sizeof(FTN_INT) == 8) {
         for(i=0;i<num-8;i+=8)
         {
            ibufr[i+7] = bufdes->bufr_data[i  ];
            ibufr[i+6] = bufdes->bufr_data[i+1];
            ibufr[i+5] = bufdes->bufr_data[i+2];
            ibufr[i+4] = bufdes->bufr_data[i+3];
            ibufr[i+3] = bufdes->bufr_data[i+4];
            ibufr[i+2] = bufdes->bufr_data[i+5];
            ibufr[i+1] = bufdes->bufr_data[i+6];
            ibufr[i  ] = bufdes->bufr_data[i+7];
         }
         ibufr[i+7] = (i  <num) ? bufdes->bufr_data[i  ] : 0;
         ibufr[i+6] = (i+1<num) ? bufdes->bufr_data[i+1] : 0;
         ibufr[i+5] = (i+2<num) ? bufdes->bufr_data[i+2] : 0;
         ibufr[i+4] = (i+3<num) ? bufdes->bufr_data[i+3] : 0;
         ibufr[i+3] = (i+4<num) ? bufdes->bufr_data[i+4] : 0;
         ibufr[i+2] = (i+5<num) ? bufdes->bufr_data[i+5] : 0;
         ibufr[i+1] = (i+6<num) ? bufdes->bufr_data[i+6] : 0;
         ibufr[i  ] = (i+7<num) ? bufdes->bufr_data[i+7] : 0;
      } else {
         sprintf(error_string,"Can not deal with FORTRAN integer length %d",sizeof(FTN_INT));
         *ier = BUFR3_ERR_INTERNAL;
         return;
      }
   } else {
      memcpy(ibufr,bufdes->bufr_data,bufdes->nbytes);
   }
}

/* ------------------------------------------------------------------------
   bufr3_put_bufr: put externally read raw data into a BUFR handle

   --- Input Parameters:

   ibufr          BUFR data. Should be stored as Fortran INTEGER.
                  Byte swapping is done automatically.

   --- Output Parameters:

   ihandle        Buffer handle
   ier            error indicator

*/


void bufr3_put_bufr_
   (unsigned char *ibufr, FTN_INT *ihandle, FTN_INT *ier)
{
   int ih, len, swap, i, res;
   unsigned char *data;

   *ier = 0;

   /* search free BUFR handle */

   if((ih=get_handle(ier)) < 0) return;

   /* Check for the characters BUFR - this way we also determine if
      the BUFR is stored in Big or Little Endian format */

   len = 0;
   swap = 0;

   if(ibufr[0]=='B' && ibufr[1]=='U' && ibufr[2]=='F' && ibufr[3]=='R')
   {
      /* Big Endian */

      swap = 0;
      len  = ibufr[4]*256*256 + ibufr[5]*256 + ibufr[6];
   }
   else
   {
      /* Not Big Endian, check if Little Endian or exit with error */

      if(sizeof(FTN_INT) == 4)
      {
         /* 4 Byte Integers */

         if(ibufr[3]=='B' && ibufr[2]=='U' && ibufr[1]=='F' && ibufr[0]=='R')
         {
            /* Little Endian */

            swap = 4;
            len  = ibufr[7]*256*256 + ibufr[6]*256 + ibufr[5];
         }
         else
         {
            sprintf(error_string,"Data does not contain BUFR at front");
            *ier = BUFR3_ERR_NO_BUFR;
            return;
         }
      }
      else if(sizeof(FTN_INT) == 8)
      {
         /* 8 Byte Integers */

         if(ibufr[7]=='B' && ibufr[6]=='U' && ibufr[5]=='F' && ibufr[4]=='R')
         {
            /* Little Endian */

            swap = 8;
            len  = ibufr[3]*256*256 + ibufr[2]*256 + ibufr[1];
         }
         else
         {
            sprintf(error_string,"Data does not contain BUFR at front");
            *ier = BUFR3_ERR_NO_BUFR;
            return;
         }
      }
      else
      {
         sprintf(error_string,"Can not deal with FORTRAN integer length %d",sizeof(FTN_INT));
         *ier = BUFR3_ERR_INTERNAL;
         return;
      }
   }

   /* allocate data array */

   data = (unsigned char *) malloc_check(len+8,ier);
   if(!data) return;

   /* Copy data */

   if(swap == 4)
   {
      for(i=0;i<len;i+=4)
      {
         data[i  ] = ibufr[i+3];
         data[i+1] = ibufr[i+2];
         data[i+2] = ibufr[i+1];
         data[i+3] = ibufr[i  ];
      }
   }
   else if (swap == 8)
   {
      for(i=0;i<len;i+=8)
      {
         data[i  ] = ibufr[i+7];
         data[i+1] = ibufr[i+6];
         data[i+2] = ibufr[i+5];
         data[i+3] = ibufr[i+4];
         data[i+4] = ibufr[i+3];
         data[i+5] = ibufr[i+2];
         data[i+6] = ibufr[i+1];
         data[i+7] = ibufr[i  ];
      }
   }
   else
   {
      memcpy(data,ibufr,len);
   }

   /* call BUFR3_decode_sections for allocating a bufr3data object */

   res = BUFR3_decode_sections(data,&bufdes_arr[ih]);
   if(res) { free(data); *ier = -res; return; }

   hflag[ih] = 1; /* BUFR read from file */
   *ihandle = ih;
}

/**************************************************************************
 *
 * Routine for releasing the memory associated with a BUFR handle
 * Must be called for every BUFR handle when it isn't needed any more
 *
 * CALL bufr3_destroy(ihandle,ier)
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   bufr3_destroy: Destroys a BUFR and all it's data associated with ihandle

   --- Input Parameters:

   ihandle        Buffer handle

   --- Output Parameters:

   ier            error indicator

*/

void bufr3_destroy_
   (FTN_INT *ihandle, FTN_INT *ier)
{
   int ih=*ihandle;

   *ier = 0;

   if(check_handle(ih,0,ier)<0) return;

   free(bufdes_arr[ih]->bufr_data);
   BUFR3_destroy(bufdes_arr[ih]);

   hflag[ih] = 0;
}

/**************************************************************************
 *
 * Routines for decoding BUFR data
 *
 * bufr3_decode
 * bufr3_get_data_old
 * bufr3_get_data
 * bufr3_get_entry_descs
 * bufr3_get_entry_texts
 * bufr3_get_entry_units
 * bufr3_get_character_value
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   bufr3_decode: Decodes a BUFR which must have been read

   --- Input Parameters:

   ihandle         BUFR handle
   mnem_list       List of mnemonics. See remarks below.
   mnem_list_len   Length on mnemonic list.
   idesc_list      List of descriptors. See remarks below.
                   Descriptors are given by 6-digit decimal numbers.
                   The first digit should be 0 unless 205YYY is selected.
                   Associated field data is also added when present.
   idesc_list_len  Length of descripto list.
   ioptions        Options for decoding, set this variable by adding
                   (or ORing) the following values:
                   1:  If set, omit loop information in output
                   2:  If set, omit sequence information in output
                   4:  If set, resort output:
                       - all values outsite a repetition list first
                       - then all values from repetition lists with
                         constant repetition count
                       - then all values from repetition lists with
                         variable repetition count

   Remarks:

   If mnem_list_len == 0 AND idesc_list_len==0 all entities in the buffer
   are output.
   If mnem_list_len>0 OR idesc_list_len>0, only entities matching either
   a mnemonic from mnem_list or a descriptor from idesc_list are output.

   --- Output Parameters

   max_subset_len  Max number of elements of a subset. This is the number
                   of decoded elements (taking mnem_list into account),
                   not the number of all elements!
   num_entry_descs Number of entry descriptors
   ier             error indicator

   --- Added by FORTRAN:

   l_mnem_list     FORTRAN character length of mnem_list
*/

void bufr3_decode_
   (FTN_INT *ihandle, char *mnem_list, FTN_INT *mnem_list_len,
    FTN_INT *idesc_list, FTN_INT *idesc_list_len, FTN_INT *ioptions,
    FTN_INT *max_subset_len, FTN_INT *num_entry_descs, FTN_INT *ier, unsigned long l_mnem_list)
{
   int i, j, ih=*ihandle, res;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,1,ier)<0) return;
   bufdes = bufdes_arr[ih];

   *max_subset_len = 0;
   *num_entry_descs = 0;

   res = BUFR3_parse_descriptors(bufdes);
   if(res) { *ier = -res; return; }

   /* Check if mnemonic list is present, output only the mnemoics
      wanted in this case */

   if(*mnem_list_len || *idesc_list_len)
   {
      char s[BUFR3_MNEM_LEN+1];

      /* By default omit all entries */

      for(j=0;j<bufdes->num_entry_descs;j++)
         bufdes->entry_descs[j].omit = 1;

      /* Include all entries in mnem_list */

      for(i=0;i<(*mnem_list_len);i++)
      {
         int l = l_mnem_list>BUFR3_MNEM_LEN ? BUFR3_MNEM_LEN : l_mnem_list;
         memcpy(s,mnem_list+i*l_mnem_list,l);
         strip_blanks(s,l);

         for(j=0;j<bufdes->num_entry_descs;j++)
         {
            if(strcmp(s,bufdes->entry_descs[j].mnem)==0)
               bufdes->entry_descs[j].omit = 0;
         }
      }

      /* Include all entries in idesc_list */

      for(i=0;i<(*idesc_list_len);i++)
      {
         for(j=0;j<bufdes->num_entry_descs;j++)
         {
            if( idesc_list[i] ==
                (bufdes->entry_descs[j].F*100000 +
                 bufdes->entry_descs[j].X*1000 +
                 bufdes->entry_descs[j].Y) )
               bufdes->entry_descs[j].omit = 0;
         }
      }
   }

   bufdes->ftn_options = *ioptions;

   /* If FTN_OMIT_LOOP_INFO is set in ioptions -> omit loop data */

   if((*ioptions)&FTN_OMIT_LOOP_INFO)
   {
      for(i=0;i<bufdes->num_entry_descs;i++)
      {
         if(bufdes->entry_descs[i].type == ED_LOOP_START ||
            bufdes->entry_descs[i].type == ED_LOOP_END ||
            bufdes->entry_descs[i].type == ED_LOOP_COUNTER )
            bufdes->entry_descs[i].omit = 1;
      }
   }

   /* If FTN_OMIT_SEQ_INFO is set in ioptions -> omit sequence data */

   if((*ioptions)&FTN_OMIT_SEQ_INFO)
   {
      for(i=0;i<bufdes->num_entry_descs;i++)
      {
         if(bufdes->entry_descs[i].type == ED_SEQ_START ||
            bufdes->entry_descs[i].type == ED_SEQ_END )
            bufdes->entry_descs[i].omit = 1;
      }
   }

   /* If FTN_RESORT_RESULTS in ioptions is set and loop info
      should be omitted, we have to retrieve it never the less.
      It is removed later in the resorting process */

   if( ( ((*ioptions)&FTN_OMIT_LOOP_INFO) || *mnem_list_len || *idesc_list_len ) &&
       ((*ioptions)&FTN_RESORT_RESULTS) )
   {
      for(i=0;i<bufdes->num_entry_descs;i++)
      {
         if(bufdes->entry_descs[i].type == ED_LOOP_START ||
            bufdes->entry_descs[i].type == ED_LOOP_END ||
            bufdes->entry_descs[i].type == ED_LOOP_COUNTER )
            bufdes->entry_descs[i].omit = 0;
      }
      bufdes->ftn_options |= FTN_REMOVE_LOOP_INFO;
   }

   res = BUFR3_decode_data(bufdes);
   if(res) { *ier = -res; return; }

   /* maximum number of elements in a subset */

   if(bufdes->ftn_options&FTN_REMOVE_LOOP_INFO) {
      *max_subset_len = bufdes->max_nli_results;
   } else {
      *max_subset_len = bufdes->max_num_results;
   }

   *num_entry_descs = bufdes->num_entry_descs;
}

/* ------------------------------------------------------------------------
   bufr3_get_data_old: Get BUFR content (compatible to previous BUFR routines)

   --- Input Parameters:

   ihandle         BUFR handle
   dim1            1. dimension of ytex1, ibufdat, fbufdat, ybufdat, nbufdat
                   dim1 should be as big as the number of subsets in the BUFR
                   If it is smaller, only the first dim1 subsets are returned
   dim2            2. dimension of ytex1, ibufdat, fbufdat, ybufdat
                   dim2 should be as big as the maximum number of elements
                   in a subset. If it is smaller, not all data is returned.

   --- Output Parameters
       (See also description of the FORTRAN BUFR interface)

   ytex1(dim1,dim2)   Character array with data describing BUFR content
   ibufdat(dim1,dim2) Integer BUFR data
   fbufdat(dim1,dim2) Floating point BUFR data
   ybufdat(dim1,dim2) Character BUFR data
   nbufdat(dim1)      Array with lengths of the BUFR subsets
   ier                error indicator

   --- Added by FORTRAN:

   l_ytex1            FORTRAN character length of ytex1
   l_ybufdat          FORTRAN character length of ybufdat
*/

void bufr3_get_data_old_
   (FTN_INT *ihandle, char *ytex1, FTN_INT *ibufdat, FTN_REAL *fbufdat,
    char *ybufdat, FTN_INT *nbufdat, FTN_INT *dim1, FTN_INT *dim2, FTN_INT *ier,
    unsigned long l_ytex1, unsigned long l_ybufdat)
{
   int i, j, n, idx, idx1, idx2, ih=*ihandle, res, *index=0;
   char (*texts)[40], (*units)[24], (*scales)[10];
   bufr3data *bufdes;

   texts  = 0;
   units  = 0;
   scales = 0;

   *ier = 0;

   if(check_handle(ih,2,ier)<0) return;
   bufdes = bufdes_arr[ih];

   if(l_ytex1 < 80)
   {
      sprintf(error_string,"Character length of ytex1 too small");
      *ier = BUFR3_ERR_DIMENSION;
      return;
   }

   memset(ybufdat,' ',(*dim1)*(*dim2)*l_ybufdat);

   /* Get the texts and units array, set scales with string ' 10**XX ' */

   texts  = (char (*)[40]) malloc_check(40*bufdes->num_entry_descs,ier);
   units  = (char (*)[24]) malloc_check(24*bufdes->num_entry_descs,ier);
   scales = (char (*)[10]) malloc_check(10*bufdes->num_entry_descs,ier);
   if(!texts || !units || !scales) goto FREE_ALL;

   res = BUFR3_get_entry_texts(bufdes,(char*)texts,40,1);
   if(res) { *ier = -res; goto FREE_ALL; }
   res = BUFR3_get_entry_units(bufdes,(char*)units,24,1);
   if(res) { *ier = -res; goto FREE_ALL; }

   for(i=0;i<bufdes->num_entry_descs;i++)
   {
      if(bufdes->entry_descs[i].scale) {
         sprintf(scales[i]," 10**%2d ",-bufdes->entry_descs[i].scale);
      } else {
         memset(scales[i],' ',8);
      }
   }

   /* Allocate index array if results have to be resorted */

   if(bufdes->ftn_options&FTN_RESORT_RESULTS) {
      index = (int *) malloc_check(bufdes->max_num_results*sizeof(int),ier);
      if(!index) goto FREE_ALL;
   }

   for(i=0;i<bufdes->num_subsets && i<*dim1;i++)
   {
      if(bufdes->ftn_options&FTN_REMOVE_LOOP_INFO) {
         nbufdat[i] = bufdes->nli_results[i];
      } else {
         nbufdat[i] = bufdes->num_results[i];
      }

      if(bufdes->ftn_options&FTN_RESORT_RESULTS) {
         nent = nind = 0;
         resort_results(bufdes,i,index);
      }

      for(j=0;j<nbufdat[i] && j<*dim2;j++)
      {
         idx  = (*dim1)*j + i;
         idx1 = idx*l_ytex1;
         idx2 = idx*l_ybufdat;

         n = (bufdes->ftn_options&FTN_RESORT_RESULTS) ? index[j] : j;

         my_strcpy(ytex1+idx1,BUFR3_DESC(bufdes,i,n).mnem,8,1);
         memcpy(ytex1+idx1+ 8,texts [bufdes->results[i][n].desc],40);
         memcpy(ytex1+idx1+48,scales[bufdes->results[i][n].desc], 8);
         memcpy(ytex1+idx1+56,units [bufdes->results[i][n].desc],24);
         if(l_ytex1>80) memset(ytex1+idx1+80,' ',l_ytex1-80);

         if(BUFR3_IS_CHAR(bufdes,i,n))
         {
            if(BUFR3_DATA(bufdes,i,n) == BUFR3_UNDEF_VALUE)
            {
               ibufdat[idx] = BUFR3_UNDEF_VALUE;
               fbufdat[idx] = BUFR3_UNDEF_VALUE;
               /* ybufdat is already set to blank */
            }
            else
            {
               ibufdat[idx] = 0;
               fbufdat[idx] = 0;
               my_strcpy(ybufdat+idx2,BUFR3_CHAR_PTR(bufdes,i,n),l_ybufdat,1);
            }
         }
         else
         {
            ibufdat[idx] = BUFR3_DATA(bufdes,i,n);
            fbufdat[idx] = BUFR3_DOUBLE_DATA(bufdes,i,n);
         }
      }
   }

FREE_ALL:
   if(index) free(index);
   if(texts) free(texts);
   if(units) free(units);
   if(scales) free(scales);
}

/* ------------------------------------------------------------------------
   bufr3_get_data:    Get BUFR content (new version, integer only)
   bufr3_get_data_ir: Like bufr3_get_data with additional output
                      of floating point values

   --- Input Parameters:

   ihandle         BUFR handle
   dim1            1. dimension of ibufdat, idescidx, nbufdat
                   dim1 should be as big as the number of subsets in the BUFR
                   If it is smaller, only the first dim1 subsets are returned
   dim2            2. dimension of ibufdat, idescidx
                   dim2 should be as big as the maximum number of elements
                   in a subset. If it is smaller, not all data is returned.

   --- Output Parameters
       (See also description of the FORTRAN BUFR interface)

   ibufdat(dim1,dim2)  Integer BUFR data
   fbufdat(dim1,dim2)  REAL BUFR data (bufr3_get_data_ir only)
   idescidx(dim1,dim2) Description pointers for BUFR data
   nbufdat(dim1)       Array with lengths of the BUFR subsets
   ier                 error indicator

*/

static void bufr3_get_data_all  /* Internal function */
   (FTN_INT *ihandle, FTN_INT *ibufdat, FTN_REAL *fbufdat, FTN_INT *idescidx,
    FTN_INT *nbufdat, FTN_INT *dim1, FTN_INT *dim2, FTN_INT *ier)
{
   int i, j, n, idx, ih=*ihandle, *index=0;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,2,ier)<0) return;
   bufdes = bufdes_arr[ih];

   if(bufdes->ftn_options&FTN_RESORT_RESULTS) {
      index = (int *) malloc_check(bufdes->max_num_results*sizeof(int),ier);
      if(!index) return;
   }

   for(i=0;i<bufdes->num_subsets && i<*dim1;i++)
   {
      if(bufdes->ftn_options&FTN_REMOVE_LOOP_INFO) {
         nbufdat[i] = bufdes->nli_results[i];
      } else {
         nbufdat[i] = bufdes->num_results[i];
      }

      if(bufdes->ftn_options&FTN_RESORT_RESULTS) {
         nent = nind = 0;
         resort_results(bufdes,i,index);
      }

      for(j=0;j<nbufdat[i] && j<*dim2;j++)
      {
         idx  = (*dim1)*j + i;

         n = (bufdes->ftn_options&FTN_RESORT_RESULTS) ? index[j] : j;
         ibufdat[idx]  = bufdes->results[i][n].data;
         idescidx[idx] = bufdes->results[i][n].desc + 1; /* Make it 1 based */
         if(fbufdat) fbufdat[idx] = BUFR3_IS_CHAR(bufdes,i,n) ? 0 : BUFR3_DOUBLE_DATA(bufdes,i,n);
      }
   }

   if(bufdes->ftn_options&FTN_RESORT_RESULTS) free(index);
}

void bufr3_get_data_  /* wrapper function */
   (FTN_INT *ihandle, FTN_INT *ibufdat, FTN_INT *idescidx,
    FTN_INT *nbufdat, FTN_INT *dim1, FTN_INT *dim2, FTN_INT *ier)
{
   bufr3_get_data_all(ihandle,ibufdat,NULL,idescidx,nbufdat,dim1,dim2,ier);
}

void bufr3_get_data_ir_  /* wrapper function */
   (FTN_INT *ihandle, FTN_INT *ibufdat, FTN_REAL *fbufdat, FTN_INT *idescidx,
    FTN_INT *nbufdat, FTN_INT *dim1, FTN_INT *dim2, FTN_INT *ier)
{
   bufr3_get_data_all(ihandle,ibufdat,fbufdat,idescidx,nbufdat,dim1,dim2,ier);
}

/* ------------------------------------------------------------------------
   bufr3_get_entry_descs: Get BUFR entry descriptions

   --- Input Parameters:

   ihandle         BUFR handle

   --- Output Parameters
       (See also description of the FORTRAN BUFR interface)

   itype(*)        Type of entry
   ifxy(*)         The FXXYYY value as decimal number
   is_char(*)      Set to 1 if entry is a character entry
   iscale(*)       Integer scale values
   scale(*)        Floting point scale value, scale(i) = 10**(-iscale(i))
   nbits(*)        Number of bits for the entry
   irefval(*)      Reference value
   ymnem(*)        Mnemonics
                   All the above arrays must be dimensioned to have at least
                   num_entry_descs (returned from bufr3_decode) members!
   ier             error indicator

   --- Added by FORTRAN:

   l_ymnem         FORTRAN character length of ymnem
*/

void bufr3_get_entry_descs_
   (FTN_INT *ihandle, FTN_INT *itype, FTN_INT *ifxy, FTN_INT *is_char,
    FTN_INT *iscale, FTN_REAL *scale, FTN_INT *nbits, FTN_INT *irefval,
    char *ymnem, FTN_INT *ier, unsigned long l_ymnem)
{
   int i, ih=*ihandle;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,2,ier)<0) return;
   bufdes = bufdes_arr[ih];

   for(i=0;i<bufdes->num_entry_descs;i++)
   {
      itype[i]  = bufdes->entry_descs[i].type;
      ifxy[i]   = bufdes->entry_descs[i].F*100000
                + bufdes->entry_descs[i].X*1000
                + bufdes->entry_descs[i].Y;
      is_char[i]= bufdes->entry_descs[i].is_char;
      iscale[i] = bufdes->entry_descs[i].scale;
      scale[i]  = bufdes->entry_descs[i].dscale;
      nbits[i]  = bufdes->entry_descs[i].bits;
      irefval[i]= bufdes->entry_descs[i].refval;
      my_strcpy(ymnem+i*l_ymnem,bufdes->entry_descs[i].mnem,l_ymnem,1);
   }
}

/* ------------------------------------------------------------------------
   bufr3_get_entry_texts: Get textual descriptions of the entries in BUFR

   --- Input Parameters:

   ihandle         BUFR handle

   --- Output Parameters
       (See also description of the FORTRAN BUFR interface)

   ytext(*)        Textual description  of entry
                   ytext must be dimensioned to have at least
                   num_entry_descs (returned from bufr3_decode) members!
                   Character length should be 40
   ier             error indicator

   --- Added by FORTRAN:

   l_ytext         FORTRAN character length of ytext
*/

void bufr3_get_entry_texts_
   (FTN_INT *ihandle, char *ytext, FTN_INT *ier, unsigned long l_ytext)
{
   int ih=*ihandle, res;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,2,ier)<0) return;
   bufdes = bufdes_arr[ih];

   res = BUFR3_get_entry_texts(bufdes,ytext,l_ytext,1);
   if(res) *ier = -res;
}

/* ------------------------------------------------------------------------
   bufr3_get_entry_units: Get units of the entries in BUFR

   --- Input Parameters:

   ihandle         BUFR handle

   --- Output Parameters
       (See also description of the FORTRAN BUFR interface)

   yunit(*)        Unit
                   yunit must be dimensioned to have at least
                   num_entry_descs (returned from bufr3_decode) members!
                   Character length should be 24
   ier             error indicator

   --- Added by FORTRAN:

   l_yunit         FORTRAN character length of yunit
*/

void bufr3_get_entry_units_
   (FTN_INT *ihandle, char *yunit, FTN_INT *ier, unsigned long l_yunit)
{
   int ih=*ihandle, res;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,2,ier)<0) return;
   bufdes = bufdes_arr[ih];

   res = BUFR3_get_entry_units(bufdes,yunit,l_yunit,1);
   if(res) *ier = -res;
}

/* ------------------------------------------------------------------------
   bufr3_get_character_value: Get a character value from BUFR data

   --- Input Parameters:

   ihandle         BUFR handle
   ibufdat         Integer-BUFR-value where character string is wanted

   --- Output Parameters:

   s               Character value
   ier             error indicator

   --- Added by FORTRAN:

   l_s             FORTRAN character length of s
*/

void bufr3_get_character_value_
   (FTN_INT *ihandle, FTN_INT *ibufdat, char *s, FTN_INT *ier, unsigned long l_s)
{
   int ih=*ihandle;
   bufr3data *bufdes;

   *ier = 0;

   if(check_handle(ih,2,ier)<0) return;
   bufdes = bufdes_arr[ih];

   if(*ibufdat>=0 && *ibufdat<bufdes->char_data_used) {
      my_strcpy(s,bufdes->char_data+(*ibufdat),l_s,1);
   } else {
      memset(s,' ',l_s);
   }
}

/**************************************************************************
 *
 * Encoding BUFR data
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   bufr3_encode: Encode BUFR

   --- Input/Output Parameters:

   ibufsec0        See description of the FORTRAN BUFR interface
   ibufsec1        See description of the FORTRAN BUFR interface
   ibufsec2        See description of the FORTRAN BUFR interface
   ibufsec3        See description of the FORTRAN BUFR interface
   ibufsec4        See description of the FORTRAN BUFR interface

   --- Input Parameters:

   idescr(.)       Descriptor array
   nbufdat(.)      Number of entries in every subset
   ibufdat(.,.)    Integer values which have to be encoded
   ybufdat(.,.)    Character values which have to be encoded
   setmne          Flag if ybufmne array should be set.

   --- Output Parameters:

   ybufmne         Mnemonic names of the encoded values
   ihandle         Handle to encoded buffer
   ier             error indicator

   --- Added by FORTRAN:

   l_ybufdat       FORTRAN character length of ybufdat
   l_ybufmne       FORTRAN character length of ybufmne
*/


void bufr3_encode_
   (FTN_INT *ibufsec0, FTN_INT *ibufsec1, FTN_INT *ibufsec2, FTN_INT *ibufsec3,
    FTN_INT *ibufsec4, FTN_INT *idescr, FTN_INT *nbufdat, FTN_INT *ibufdat,
    char *ybufdat, FTN_INT *setmne, char *ybufmne, FTN_INT *ihandle, FTN_INT *ier,
    unsigned long l_ybufdat, unsigned long l_ybufmne)
{
   int ih, iednr, ndescr, num_subsets=0, l3, i, j, res;
   unsigned char *sect1=0, *sect2=0, *sect3=0;
   int *num_entries=0;
   int **entries=0;
   unsigned char ***centries=0;
   bufr3data *bufdes=0;

   *ier = 0;


   /* search free BUFR handle */

   if((ih=get_handle(ier)) < 0) return;

   *ihandle = ih;
   hflag[ih] = 2;

   iednr = ibufsec0[1];

   /* section 1 is always 18 bytes in the FORTRAN interface */

   sect1 = (unsigned char *) malloc_check(18*sizeof(char),ier);
   if(!sect1) goto FREE_ALL;

   sect1[0] = 0;
   sect1[1] = 0;
   sect1[2] = 18;

   for(i=1;i<15;i++) sect1[i+2] = ibufsec1[i];
   sect1[17] = 0;

   if(ibufsec1[8] == -1) sect1[10] = 0;
   if(ibufsec1[9] == -1) sect1[11] = 0;

   /* prepare section 2: 0 or 18 bytes */

   if( (sect1[7]&0x80) == 0x80 )
   {
      sect2 = (unsigned char *) malloc_check(18*sizeof(char),ier);
      if(!sect2) goto FREE_ALL;

      sect2[ 0] = 0;
      sect2[ 1] = 0;
      sect2[ 2] = 18;
      sect2[ 3] = 0;
      sect2[ 4] = ibufsec2[ 2];
      sect2[ 5] = ibufsec2[ 3];
      sect2[ 6] = (ibufsec2[4]>>16)&0xff;
      sect2[ 7] = (ibufsec2[4]>> 8)&0xff;
      sect2[ 8] = (ibufsec2[4]    )&0xff;
      sect2[ 9] = ibufsec2[ 5];
      sect2[10] = ibufsec2[ 6];
      sect2[11] = ibufsec2[ 7];
      sect2[12] = ibufsec2[ 8];
      sect2[13] = ibufsec2[ 9];
      sect2[14] = ibufsec2[10];
      sect2[15] = (ibufsec2[11]>> 8)&0xff;
      sect2[16] = (ibufsec2[11]    )&0xff;
      sect2[17] = 0;
   }
   else
   {
      sect2 = 0;
   }

   /* prepare section 3 */

   num_subsets = ibufsec3[2];
   ndescr = ibufsec3[4];
   l3 = 8+2*ndescr;
   sect3 = (unsigned char *) malloc_check(l3*sizeof(char),ier);
   if(!sect3) goto FREE_ALL;

   sect3[0] = (l3>>16)&0xff;
   sect3[1] = (l3>> 8)&0xff;
   sect3[2] = (l3    )&0xff;
   sect3[3] = 0;
   sect3[4] = (ibufsec3[2]>>8)&0xff;
   sect3[5] = (ibufsec3[2]   )&0xff;
   sect3[6] = ibufsec3[3];

   sect3[l3-1] = 0; /* Padding byte */

   for(i=0;i<ndescr;i++)
   {
      int F, X, Y;
      F = idescr[i]/100000;
      X = (idescr[i]-F*100000)/1000;
      Y = idescr[i]%1000;
      sect3[2*i+7] = F*64+X;
      sect3[2*i+8] = Y;
   }

   /* We must copy the user data since this version uses transposed data */

   entries = (int **) malloc_check(num_subsets*sizeof(int *),ier);
   centries = (unsigned char ***) malloc_check(num_subsets*sizeof(char **),ier);
   num_entries = (int *) malloc_check(num_subsets*sizeof(int),ier);
   if(!entries || !centries || !num_entries) goto FREE_ALL;

   for(i=0;i<num_subsets;i++)
   {
      entries[i] = (int *) malloc_check(nbufdat[i]*sizeof(int),ier);
      centries[i] = (unsigned char **) malloc_check(nbufdat[i]*sizeof(char *),ier);
      if(!entries[i] || !centries[i]) goto FREE_ALL;
      num_entries[i] = nbufdat[i];

      for(j=0;j<nbufdat[i];j++)
      {
         entries[i][j] = ibufdat[i+j*num_subsets];
         centries[i][j] = ybufdat + (i+j*num_subsets)*l_ybufdat;
      }
   }


   /* Call BUFR3_encode */

   res = BUFR3_encode(iednr, sect1, sect2, sect3, entries, centries, num_entries, *setmne, &bufdes);
   if(res) { *ier = -res; goto FREE_ALL; }
   bufdes_arr[*ihandle] = bufdes;


   ibufsec0[0] = bufdes->nbytes;

   ibufsec1[0] = bufdes->sect1_length;

   if(ibufsec1[8] == -1) ibufsec1[8] = bufdes->sect1[10];
   if(ibufsec1[9] == -1) ibufsec1[9] = bufdes->sect1[11];

   ibufsec2[0] = bufdes->sect2_length;
   ibufsec3[0] = bufdes->sect3_length;
   ibufsec4[0] = bufdes->sect4_length;

   /* Copy the mnemonics of the encoded entries to ybufmne */

   if(*setmne)
   {
      for(i=0;i<num_subsets;i++)
      {
         for(j=0;j<nbufdat[i];j++)
         {
            int idx = (j*num_subsets+i)*l_ybufmne;
            my_strcpy(ybufmne+idx,bufdes->entry_descs[bufdes->desc_idx[i][j]].mnem,l_ybufmne,1);
         }
      }
   }

FREE_ALL:
   if(sect1) free(sect1);
   if(sect2) free(sect2);
   if(sect3) free(sect3);

   for(i=0;i<num_subsets;i++)
   {
      if(entries  && entries[i] ) free(entries[i]);
      if(centries && centries[i]) free(centries[i]);
   }
   if(entries)  free(entries);
   if(centries) free(centries);
   if(num_entries) free(num_entries);
}

/**************************************************************************
 *
 * Retrieve the last error message
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   bufr3_get_error_message

   --- Output Parameters:

   s               Error message

   --- Added by FORTRAN:

   l_s             FORTRAN character length of s
*/

void bufr3_get_error_message_(char *s, unsigned long l_s)
{
   char *e;

   /* if error_string is set, use this one (since there must be an error
      message from the FORTRAN Interface) else call BUFR3_get_error_string
      to retrieve the error message from the C-part */

   if(error_string[0]) {
      my_strcpy(s,error_string,l_s,1);
      error_string[0] = 0;
   } else {
      e = BUFR3_get_error_string();
      my_strcpy(s,e,l_s,1);
   }
}
