/**************************************************************************

   bufr3_tables.c - Functions for reading BUFR3 B and D tables

   Version: 1.1

   Author: Dr. Rainer Johanni, Apr. 2003

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
extern int errno;
#include "bufr3_tables.h"                  /* martin, org: <bufr3_tables.h> */ 

/* for stat */

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#define strncasecmp strnicmp             /* martin org: #define strncasecmp _strnicmp */
#define stat _stat
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif
#define MAXLINELEN 4096 /* Max length of a line in input files */

#define LIBDWD_BUFRTABLE_B             "C:\\Program Files\\BC5\\PROJECTS\\BUFR_write_TurboWin\\tables\\test2_TabB.csv"
#define LIBDWD_BUFRTABLE_D             "C:\\Program Files\\BC5\\PROJECTS\\BUFR_write_TurboWin\\tables\\TabD.csv"
/*
   User Interface
   ==============



   #include <bufr3_tables.h>

   extern int bufr3_tables_master_version;
   extern int bufr3_tables_local_version;
   extern char bufr3_tables_local_path[4096];

   int bufr3_read_tables(unsigned char *sect1)

   char *bufr3_tables_get_error_string(void)

   tab_b_entry *bufr3_get_tab_b_entry(int X, int Y)

   tab_d_entry *bufr3_get_tab_d_entry(int X, int Y)



   bufr3_read_tables reads new BUFR3 tables B and D.
   Which tables exactly are read is determined by BUFR section 1.
   If the program detects that the tables needed are already
   in memory, nothing is done.

   bufr3_read_tables returns 0 on success and -1 on error.

   In the case of an error bufr3_tables_get_error_string()
   delivers a pointer to the exact error message.
   This pointer points to a static memory area, therefore
   it must not be free'ed by the user.

   bufr3_get_tab_b_entry and bufr3_get_tab_d_entry
   return a pointer to a tab_b_entry or tab_d_entry
   or a NULL pointer if the specific entry doesn't exist.
   This pointer points to a static memory area, therefore
   it must not be free'ed by the user.

   The externally visible variables bufr3_tables_master_version,
   bufr3_tables_local_version and bufr3_tables_local_path
   allow the user to check if the tables really have changed
   during a call to bufr3_read_tables.
   If these three variables stay the same, no new tables have been read.

*/

static tab_b_entry *(bufr3_tab_b[64][256]); /* Table B - array of pointers */
static tab_d_entry *(bufr3_tab_d[64][256]); /* Table D - array of pointers */

static int init_tables=1;

static int num_tab_b_entries;
static unsigned char X_tab_b[64*256], Y_tab_b[64*256];

static int num_tab_d_entries;
static unsigned char X_tab_d[64*256], Y_tab_d[64*256];

static char error_string[4096];

/* External variables */

int bufr3_tables_master_version = -1;
int bufr3_tables_local_version = -1;

char bufr3_tables_local_path[4096];





/**************************************************************************
 *
 * Auxiliary internal routines
 *
 **************************************************************************/

static void strip_whitespace(char *s)
{
   int i;

   /* replace trailing whitespce by 0 bytes */

   for(i=strlen(s)-1;i>=0;i--)
   {
      if(!isspace(s[i])) break;
      s[i] = 0;
   }
}

/* clear_tab_b: Deallocate all table B entries */

static void clear_tab_b()
{
   int i;

   for(i=0;i<num_tab_b_entries;i++)
   {
      int X = X_tab_b[i];
      int Y = Y_tab_b[i];
      free(bufr3_tab_b[X][Y]);
      bufr3_tab_b[X][Y] = 0;
   }
   num_tab_b_entries = 0;
}

/* clear_tab_d: Deallocate all table D entries */

static void clear_tab_d()
{
   int i;

   for(i=0;i<num_tab_d_entries;i++)
   {
      int X = X_tab_d[i];
      int Y = Y_tab_d[i];
      if(bufr3_tab_d[X][Y]->desc) free(bufr3_tab_d[X][Y]->desc);
      free(bufr3_tab_d[X][Y]);
      bufr3_tab_d[X][Y] = 0;
   }
   num_tab_d_entries = 0;
}

static int read_mnemonics(char *filename)
{
   FILE *fd;
   unsigned char line[MAXLINELEN], mnem[8];
   int linelen, F, X, Y, n;

   fd = fopen(filename,"r");

   if(!fd) {
      sprintf(error_string,"Error opening %s, reason: %s",filename,strerror(errno));
      return -1;
   }

   while(fgets(line,MAXLINELEN,fd))
   {
      /* Check if the line was longer than MAXLINELEN (most probably because
         we got a wrong file). This is indicated by a missing newline at the end */

      linelen = strlen(line);
      if(line[linelen-1] != '\n') {
         sprintf(error_string,"Line in %s too long",filename);
         goto ERROR;
      }

      /* Check for empty/comment line */

      for(n=0;n<linelen;n++) if(!isspace(line[n])) break; /* Search first non-space character */
      if(n==linelen || line[n]=='#') continue;

      n = sscanf(line,"%1d%2d%3d %6s",&F,&X,&Y,mnem);

      if(n!=4 || F!=0 || X<0 || X>63 || Y<0 || Y>255) {
         sprintf(error_string,"Line in %s invalid: %s",filename,line);
         goto ERROR;
      }

      /* Store Mnemonic if table B entry is present */

      if(bufr3_tab_b[X][Y]) {
         memset(bufr3_tab_b[X][Y]->mnem,0,BUFR3_MNEM_LEN+1);
         strcpy(bufr3_tab_b[X][Y]->mnem,mnem);
      }
   }

   fclose(fd);
   return 0;

ERROR:
   /* Clean up */

   fclose(fd);
   return -1;
}

static int read_tab_b(char *filename)
{
   FILE *fd;
   unsigned char line[MAXLINELEN], unit[MAXLINELEN], name[MAXLINELEN],
                 mnem[40], text[40];
   int nl=0, linelen, F, X, Y, scale, refval, bits, num, n;

   fd = fopen(filename,"r");

   if(!fd) {
      sprintf(error_string,"Error opening %s, reason: %s",filename,strerror(errno));
      return -1;
   }

   while(fgets(line,MAXLINELEN,fd))
   {
      nl++; /* Line counter */

      /* Check if the line was longer than MAXLINELEN (most probably because
         we got a wrong file). This is indicated by a missing newline at the end */

      linelen = strlen(line);
      if(line[linelen-1] != '\n') {
         sprintf(error_string,"Line in %s too long",filename);
         goto ERROR;
      }

      /* Check for empty/comment line */

      for(n=0;n<linelen;n++) if(!isspace(line[n])) break; /* Search first non-space character */
      if(n==linelen || line[n]=='#') continue;

      /* Check the type of the line (DWD/OPERA/invalid) */

      /* Try to read first number */

      F = -1;
      num = sscanf(line,"%d ",&F);

      if(num==1 && F==0)
      {
         /* Table B line in OPERA-Style */

         num = sscanf(line,"%d %d %d %s %d %d %d %[^\n]",&F,&X,&Y,
                      unit,&scale,&refval,&bits,name);

         if(num != 7 && num != 8) {
            sprintf(error_string,"ERROR in OPERA-Style Table B line: %s",line);
            goto ERROR;
         }

         strip_whitespace(name);
         sprintf(mnem,"0%2.2d%3.3d",X,Y);
      }
      else if(line[0] == '0' && isdigit(line[1]) && isdigit(line[2]) &&
              isdigit(line[3]) && isdigit(line[4]) && isdigit(line[5]) )
      {
         /* Table B line in DWD-Style */

         if(linelen<102) {
            sprintf(error_string,"Table B line too short: %s",line);
            goto ERROR;
         }

         X = (line[1]-'0')*10 + (line[2]-'0');
         Y = (line[3]-'0')*100 + (line[4]-'0')*10 + (line[5]-'0');

         memcpy(name,line+6,40);
         name[40] = 0;
         strip_whitespace(name);

         memcpy(unit,line+46,24);
         unit[24] = 0;
         strip_whitespace(unit);

         memcpy(mnem,line+95,7);
         mnem[7] = 0;
         strip_whitespace(mnem);

         /* Scale, reference value and bits */

         memcpy(text,line+70,5);
         text[5] = 0;
         scale = atoi(text);
         memcpy(text,line+75,15);
         text[15] = 0;
         refval = atoi(text);
         memcpy(text,line+90,5);
         text[5] = 0;
         bits = atoi(text);
      }
      else if(nl==1 && strncmp(line,"    ",4)==0)
      {
         /* Ignore this line, most propably header line of old DWD format */
         continue;
      }
      else
      {
         /* Error: line is not recocnized */
         sprintf(error_string,"Table B line not recognized: %s",line);
         goto ERROR;
      }

      /* If we come here, line is successfully read */

      /* Check if this entry is already defined */

      if(bufr3_tab_b[X][Y]) {
         sprintf(error_string,"Entry %d %d in table B defined twice",X,Y);
         goto ERROR;
      }

      /* allocate entry */

      bufr3_tab_b[X][Y] = (tab_b_entry *) malloc(sizeof(tab_b_entry));
      if(!bufr3_tab_b[X][Y]) {
         sprintf(error_string,"malloc failed");
         goto ERROR;
      }

      /* remember X and Y values in the list of table B entries */

      X_tab_b[num_tab_b_entries] = X;
      Y_tab_b[num_tab_b_entries] = Y;
      num_tab_b_entries++;

      /* Set name */

      memset(bufr3_tab_b[X][Y]->name,0,BUFR3_NAME_LEN+1);
      strncpy(bufr3_tab_b[X][Y]->name,name,BUFR3_NAME_LEN);

      /* Set unit and determine type */

      memset(bufr3_tab_b[X][Y]->unit,0,BUFR3_UNIT_LEN+1);
      strncpy(bufr3_tab_b[X][Y]->unit,unit,BUFR3_UNIT_LEN);

      if(strncasecmp(unit,"CCITT",5)==0)
         bufr3_tab_b[X][Y]->type = TAB_B_TYPE_CHAR;
      else if (strncasecmp(unit,"CODE",4)==0)
         bufr3_tab_b[X][Y]->type = TAB_B_TYPE_CODE;
      else if (strncasecmp(unit,"FLAG",4)==0)
         bufr3_tab_b[X][Y]->type = TAB_B_TYPE_FLAG;
      else if (strncasecmp(unit,"NUMERIC",7)==0)
         bufr3_tab_b[X][Y]->type = TAB_B_TYPE_NUMERIC;
      else
         bufr3_tab_b[X][Y]->type = TAB_B_TYPE_OTHER;

      /* Set mnemonic */

      memset(bufr3_tab_b[X][Y]->mnem,0,BUFR3_MNEM_LEN+1);
      strncpy(bufr3_tab_b[X][Y]->mnem,mnem,BUFR3_MNEM_LEN);

      /* Scale, reference value and bits */

      bufr3_tab_b[X][Y]->scale = scale;
      bufr3_tab_b[X][Y]->refval = refval;
      bufr3_tab_b[X][Y]->bits = bits;
   }

   /* Read in Mnemonics if $LIBDWD_MNEMONICTABLE is set.
      These will override mnemonics read so far */

   if(getenv("LIBDWD_MNEMONICTABLE")) {
      if(read_mnemonics(getenv("LIBDWD_MNEMONICTABLE")) < 0) goto ERROR;
   }

   fclose(fd);
   return 0;

ERROR:
   /* Clean up */

   fclose(fd);
   clear_tab_b();
   return -1;
}

static int read_tab_d(char *filename)
{
   FILE *fd;
   unsigned char line[MAXLINELEN], comment[MAXLINELEN], desc[16384];
   int ndesc=0, F, X, Y, Fi, Xi, Yi, linelen, num, i;

   fd = fopen(filename,"r");

   if(!fd) {
      sprintf(error_string,"Error opening %s, reason: %s",filename,strerror(errno));
      return -1;
   }

   while(1)
   {
      if(fgets(line,MAXLINELEN,fd))
      {
         /* Check if the line was longer than MAXLINELEN (most probably because
            we got a wrong file). This is indicated by a missing newline at the end */

         linelen = strlen(line);
         if(line[linelen-1] != '\n') {
            sprintf(error_string,"Line in %s too long",filename);
            goto ERROR;
         }
         strip_whitespace(line);
      }
      else
      {
         /* We are at EOF. If there is an open group, simulate an empty line,
            else we are done */

         if(ndesc) {
            line[0] = 0;
         } else {
            break;
         }
      }

      if(strlen(line) == 0)
      {
         /* Empty line - if there is an open group, store it */

         if(ndesc)
         {

            /* allocate entry */

            bufr3_tab_d[X][Y] = (tab_d_entry *) malloc(sizeof(tab_d_entry));
            if(!bufr3_tab_d[X][Y]) {
               sprintf(error_string,"malloc failed");
               goto ERROR;
            }

            /* remember X and Y values in the list of table D entries */

            X_tab_d[num_tab_d_entries] = X;
            Y_tab_d[num_tab_d_entries] = Y;
            num_tab_d_entries++;

            bufr3_tab_d[X][Y]->ndesc = ndesc;
            memset(bufr3_tab_d[X][Y]->text,0,BUFR3_TABD_TEXT_LEN+1);
            strncpy(bufr3_tab_d[X][Y]->text,comment,BUFR3_TABD_TEXT_LEN);

            bufr3_tab_d[X][Y]->desc = (unsigned char *) malloc(2*ndesc*sizeof(unsigned char));
            if(!bufr3_tab_d[X][Y]->desc) {
               sprintf(error_string,"malloc failed");
               goto ERROR;
            }

            memcpy(bufr3_tab_d[X][Y]->desc,desc,2*ndesc);

            ndesc = 0;
            comment[0] = 0;
         }

         continue;
      }

      if(line[0] == '#')
      {
         /* This is a comment line. If we are not building a group, store it */

         if(ndesc==0)
         {
            /* search first nonblank character */
            linelen = strlen(line);
            for(i=1;i<linelen;i++) if(line[i] != ' ' && line[i] != '\t') break;
            if(i<linelen) strcpy(comment,line+i);
         }
         continue;
      }

      /* Non empty, non comment lines must contain descriptors */

      if(ndesc)
      {
         /* there must be 1 descriptor to be added to the group */

         num = sscanf(line,"%1d%2d%3d",&Fi,&Xi,&Yi);

         if(num!=3 || Fi<0 || Fi>3 || Xi<0 || Xi>63 || Yi<0 || Yi>255) {
            sprintf(error_string,"Table D line contains no valid descriptor: %s",line);
            goto ERROR;
         }
         desc[2*ndesc  ] = Fi*64 + Xi;
         desc[2*ndesc+1] = Yi;
         ndesc++;
      }
      else
      {
         /* There must be the descriptor for the group and the first descriptor of the group */

         num = sscanf(line,"%1d%2d%3d%1d%2d%3d",&F,&X,&Y,&Fi,&Xi,&Yi);
         if(num!=6 || F != 3 || X<0 || X>63 || Y<0 || Y>255 ||
            Fi<0 || Fi>3 || Xi<0 || Xi>63 || Yi<0 || Yi>255) {
            sprintf(error_string,"Table D line contains no valid pair of descriptors: %s",line);
            goto ERROR;
         }

         /* Check if this entry is already defined */

         if(bufr3_tab_d[X][Y]) {
            sprintf(error_string,"Entry %d %d in table D defined twice",X,Y);
            goto ERROR;
         }

         desc[0] = Fi*64 + Xi;
         desc[1] = Yi;
         ndesc = 1;
      }
   }

   fclose(fd);
   return 0;

ERROR:
   /* Clean up */

   fclose(fd);
   clear_tab_d();
   return -1;
}


static int read_tab_d_old(char *filename)
{
   FILE *fd;
   unsigned char line[MAXLINELEN], *s;
   int nl=0, l, i, X, Y, ndesc;

   fd = fopen(filename,"r");

   if(!fd) {
      sprintf(error_string,"Error opening %s, reason: %s",filename,strerror(errno));
      return -1;
   }

   while(fgets(line,MAXLINELEN,fd))
   {
      nl++; /* Line counter */

      l = strlen(line);
      if(line[l-1] != '\n') {
         sprintf(error_string,"Line in %s too long",filename);
         goto ERROR;
      }

      if(line[0] != '3') continue; /* skip first line and possible empty lines */

      X = (line[1]-'0')*10 + (line[2]-'0');
      Y = (line[3]-'0')*100 + (line[4]-'0')*10 + (line[5]-'0');

      /* Check if this entry is already defined */

      if(bufr3_tab_d[X][Y]) {
         sprintf(error_string,"Entry %d %d in table D defined twice",X,Y);
         goto ERROR;
      }

      /* Count the number of descriptors in this line.
         NB: We don't have a limit on this number here, the line may
         be arbitrary long (limited only by MAXLINELEN) */

      for(i=72;i<l;i+=6)
         if(line[i]!='0' && line[i]!='1'  && line[i]!='2' && line[i]!='3') break;
      ndesc = (i-72)/6;
      if(ndesc==0) {
         sprintf(error_string,"Line %d in %s doesnt contain entries",nl,filename);
         goto ERROR;
      }

      /* allocate entry */

      bufr3_tab_d[X][Y] = (tab_d_entry *) malloc(sizeof(tab_d_entry));
      if(!bufr3_tab_d[X][Y]) {
         sprintf(error_string,"malloc failed");
         goto ERROR;
      }

      /* remember X and Y values in the list of table D entries */

      X_tab_d[num_tab_d_entries] = X;
      Y_tab_d[num_tab_d_entries] = Y;
      num_tab_d_entries++;

      memset(bufr3_tab_d[X][Y]->text,0,BUFR3_TABD_TEXT_LEN);
      memcpy(bufr3_tab_d[X][Y]->text,line+6,BUFR3_TABD_TEXT_LEN<66?BUFR3_TABD_TEXT_LEN:66);
      strip_whitespace(bufr3_tab_d[X][Y]->text);

      bufr3_tab_d[X][Y]->ndesc = ndesc;
      bufr3_tab_d[X][Y]->desc = (unsigned char *) malloc(2*ndesc*sizeof(unsigned char));
      if(!bufr3_tab_d[X][Y]->desc) {
         sprintf(error_string,"malloc failed");
         goto ERROR;
      }

      for(i=0;i<ndesc;i++) {
         s = line+72+6*i;
         bufr3_tab_d[X][Y]->desc[2*i  ] = (s[0]-'0')*64  + (s[1]-'0')*10 + (s[2]-'0');
         bufr3_tab_d[X][Y]->desc[2*i+1] = (s[3]-'0')*100 + (s[4]-'0')*10 + (s[5]-'0');
      }
   }

   fclose(fd);
   return 0;

ERROR:
   /* Clean up */

   fclose(fd);
   clear_tab_d();
   return -1;
}

/*
   get_version_by_date: get the version number by reading ltn_datum
                        in the directory "path"
*/

static int get_version_by_date(char *path, int year, int month, int day)
{
   int date, numtab, datetab;
   char filename[MAXPATHLEN], line[MAXLINELEN];
   FILE *fd;

   /* Get full year */

   if(year<70) {
      /* assume 21. century */
      year += 2000;
   } else {
      /* assume 20. century */
      year += 1900;
   }

   date = year*10000 + month*100 + day;

   /* Get filename */

   sprintf(filename,"%s/ltn_datum",path);

   fd = fopen(filename,"r");
   if(!fd) {
      sprintf(error_string,"Error opening %s, reason: %s",filename,strerror(errno));
      return -1;
   }

   numtab = 0;
   datetab = 0;
   while(fgets(line,MAXLINELEN,fd))
   {
      sscanf(line," %d %d",&numtab,&datetab);
      if(date<datetab) {
         fclose(fd);
         return numtab;
      }
   }

   /* if we are here, date has not been found */

   fclose(fd);
   sprintf(error_string,"Date: %d not found in %s",date,filename);
   return -1;
}


/**************************************************************************
 *
 * User interface routines
 *
 **************************************************************************/

/*
   bufr3_read_tables: Read tables B and D

   Parameter:

   sect1  Section 1 of the BUFR for which the tables should be read.
          This is an input parameter except for the case that
          bytes 11 or 12 (version number master/local table) are
          set to 0. In this case those bytes are replaced by the
          version numbers used.

   Return value: 0 on success, -1 on error
*/

int bufr3_read_tables(unsigned char *sect1)
{
   char *file_b, *file_d, *path, *lpath, local_path[MAXPATHLEN], filename[MAXPATHLEN];
   int X, Y, ret, master_version, local_version;

   if(init_tables) {

      /* This is the first call, set all pointers in the tables B and D to NULL.
         Normally this should not be necessary if static variables are
         initialized to 0, but you never know ... */

      for(X=0;X<64;X++)
         for(Y=0;Y<256;Y++)
         {
            bufr3_tab_b[X][Y] = NULL;
            bufr3_tab_d[X][Y] = NULL;
         }
      num_tab_b_entries = 0;
      num_tab_d_entries = 0;

      bufr3_tables_master_version = -1;
      bufr3_tables_local_version  = -1;
      bufr3_tables_local_path[0]  =  0;

      init_tables = 0;
   }

   /* Check if LIBDWD_BUFRTABLE_B and LIBDWD_BUFRTABLE_D are set */

   file_b = LIBDWD_BUFRTABLE_B;             /* martin org: file_b = getenv("LIBDWD_BUFRTABLE_B"); */
   file_d = LIBDWD_BUFRTABLE_D;             /* martin org: file_d = getenv("LIBDWD_BUFRTABLE_D"); */

   if(file_b && !file_d) {
      sprintf(error_string,"$LIBDWD_BUFRTABLE_B is set, but not $LIBDWD_BUFRTABLE_D");
      return -1;
   }
   if(!file_b && file_d) {
      sprintf(error_string,"$LIBDWD_BUFRTABLE_D is set, but not $LIBDWD_BUFRTABLE_B");
      return -1;
   }

   if(file_b && file_d)
   {
      /* Since these tables are fixed, we need to read them only the first time,
         i.e. when  then number of entries is 0. */

      if(!num_tab_b_entries) {
         ret = read_tab_b(file_b);
         if(ret<0) return -1;
      }
      if(!num_tab_d_entries) {
         ret = read_tab_d(file_d);
         if(ret<0) return -1;
      }
      return 0;
   }

   /* Check if LIBDWD_BUFRTABLE_PATH_V2 is set, read Version 2 tables in this case */
#if 0 /* martin */
   path = getenv("LIBDWD_BUFRTABLE_PATH_V2");

   if(path)
   {
      if(strlen(path)>MAXPATHLEN-40) {
         sprintf(error_string,"$LIBDWD_BUFRTABLE_PATH_V2 too long");
         return -1;
      }

      /* Get master version */

      /* According to a DWD request, the number from sect1[10] should
         not be used since it is wrong for most BUFRs.
         In order to use sect1[10] again, shwitch "#if 0" to "#if 1" below */

#if 0
      master_version = sect1[10];
      if(master_version == 0) {
         master_version = get_version_by_date(path,sect1[12],sect1[13],sect1[14]);
         if(master_version<0) return -1;
         sect1[10] = master_version;
      }
#else
      master_version = get_version_by_date(path,sect1[12],sect1[13],sect1[14]);
      if(master_version<0) return -1;
      if(sect1[10] == 0) sect1[10] = master_version;
#endif

      /* Get path to local version */

      lpath = getenv("LIBDWD_LOCAL_BUFRTABLE_PATH");

      if(lpath) {
         if(strlen(lpath)>MAXPATHLEN-40) {
            sprintf(error_string,"$LIBDWD_LOCAL_BUFRTABLE_PATH too long");
            return -1;
         }
         strcpy(local_path,lpath);
      } else {
         struct stat stat_buf;

         sprintf(local_path,"%s/local_%3.3d_%3.3d",path,sect1[5],sect1[4]);

         /* Check if local_path exists. If this isn't the case we just skip
            reading the local tables (by setting local_path to the empty string) */

         if(stat(local_path,&stat_buf)) local_path[0] = 0;
      }

      /* Get local version */

      local_version = sect1[11];
      if(local_version == 0 && strlen(local_path)>0) {
         local_version = get_version_by_date(local_path,sect1[12],sect1[13],sect1[14]);
         if(local_version<0) return -1;
         sect1[11] = local_version;
      }
      if(strlen(local_path)==0) local_version = -1;

      /* Check if there is anything to do */

      if( master_version == bufr3_tables_master_version &&
          strcmp(local_path,bufr3_tables_local_path) == 0 &&
          local_version == bufr3_tables_local_version ) return 0; /* Tables already loaded */

      /* Load Tables */

      clear_tab_b();
      clear_tab_d();
      bufr3_tables_master_version = -1;
      bufr3_tables_local_version  = -1;
      bufr3_tables_local_path[0]  =  0;

      /* Master table */

      sprintf(filename,"%s/table_b_%3.3d",path,master_version);
      ret = read_tab_b(filename);
      if(ret<0) return -1;

      sprintf(filename,"%s/table_d_%3.3d",path,master_version);
      ret = read_tab_d(filename);
      if(ret<0) return -1;

      bufr3_tables_master_version = master_version;

      /* Local table */

      if(strlen(local_path)>0)
      {
         sprintf(filename,"%s/table_b_%3.3d",local_path,local_version);
         ret = read_tab_b(filename);
         if(ret<0) return -1;

         sprintf(filename,"%s/table_d_%3.3d",local_path,local_version);
         ret = read_tab_d(filename);
         if(ret<0) return -1;

         bufr3_tables_local_version = local_version;
         strcpy(bufr3_tables_local_path,local_path);
      }

      return 0;
   }


   /* Check if LIBDWD_BUFRTABLE_PATH[_V1] is set, read Version 1 tables in this case */

   path = getenv("LIBDWD_BUFRTABLE_PATH_V1");
   if(!path) path = getenv("LIBDWD_BUFRTABLE_PATH");

   if(path)
   {
      if(strlen(path)>MAXPATHLEN-40) {
         sprintf(error_string,"$LIBDWD_BUFRTABLE_PATH[_V1] too long");
         return -1;
      }

      /* Get version */

      if(sect1[4]==0 && sect1[5]==78 && sect1[11]>0)
      {
         local_version = sect1[11];
      }
      else
      {
         local_version = get_version_by_date(path,sect1[12],sect1[13],sect1[14]);
         if(local_version<0) return -1;
         sect1[11] = local_version;
      }

      if(sect1[10] == 0) sect1[10] = 7; /* this is hardcoded in writeBUFR3.F */

      if(local_version == bufr3_tables_local_version) return 0; /* Tables already loaded */

      /* Load Tables */

      clear_tab_b();
      clear_tab_d();
      bufr3_tables_local_version = -1;

      sprintf(filename,"%s/tab_b%3.3d",path,local_version);
      ret = read_tab_b(filename);
      if(ret<0) return -1;

      sprintf(filename,"%s/tab_d%3.3d",path,local_version);
      ret = read_tab_d_old(filename);
      if(ret<0) return -1;

      bufr3_tables_local_version = local_version;

      return 0;
   }

   /* If we come here, $LIBDWD_BUFRTABLE_PATH_V[12] wasn't set */

   sprintf(error_string,"$LIBDWD_BUFRTABLE_PATH_V[12] is not set");
   return -1;
#endif /* martin */
}

char *bufr3_tables_get_error_string(void)
{
   return error_string;
}

/*
   bufr3_get_tab_b_entry, bufr3_get_tab_d_entry:

   Returns a pointer to the table B/D entry X,Y or a NULL pointer if this
   entry is not defined.

   The return value is undefined if a previous bufr3_read_tables call
   didn't succeed or if no bufr3_read_tables call has been made before.
*/
                          

tab_b_entry *bufr3_get_tab_b_entry(int X, int Y)
{
   return bufr3_tab_b[X][Y];
}

tab_d_entry *bufr3_get_tab_d_entry(int X, int Y)
{
   return bufr3_tab_d[X][Y];
}
