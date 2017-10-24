/**************************************************************************

   bufr3_tables.h - Include file for BUFR3 library for DWD

   Author: Dr. Rainer Johanni, Feb. 2003

 **************************************************************************/

#ifndef BUFR3_TABLES_H
#define BUFR3_TABLES_H

#define BUFR3_NAME_LEN 40 /* Table B */
#define BUFR3_UNIT_LEN 24 /* Table B */
#define BUFR3_MNEM_LEN  8 /* Table B */

#define BUFR3_TABD_TEXT_LEN 66 /* Table D */


/* Table B description */

typedef enum _tab_b_entry_type {
   TAB_B_TYPE_OTHER,         /* Anything else than the data below */
   TAB_B_TYPE_CHAR,          /* "CCITT IA5" */
   TAB_B_TYPE_CODE,          /* "CODE TABLE" */
   TAB_B_TYPE_FLAG,          /* "FLAG TABLE" */
   TAB_B_TYPE_NUMERIC        /* "NUMERIC" */
} tab_b_entry_type;

typedef struct _tab_b_entry
{
   int  scale;
   int  refval;
   int  bits;
   tab_b_entry_type type;
   char name[BUFR3_NAME_LEN+1];
   char unit[BUFR3_UNIT_LEN+1];
   char mnem[BUFR3_MNEM_LEN+1];
} tab_b_entry;

/* Table D description */

typedef struct _tab_d_entry
{
   int ndesc;             /* number of descriptors */
   unsigned char *desc;   /* descriptor array, one descriptor is
                             defined by two successive characters */
   char text[BUFR3_TABD_TEXT_LEN]; /* Description */
} tab_d_entry;

/* prototypes */

int bufr3_read_tables(unsigned char *sect1);
char *bufr3_tables_get_error_string(void);
tab_b_entry *bufr3_get_tab_b_entry(int X, int Y);
tab_d_entry *bufr3_get_tab_d_entry(int X, int Y);

#endif
