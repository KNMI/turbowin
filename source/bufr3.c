/**************************************************************************

   bufr3.c - BUFR3 library for DWD

   Version: 1.1.1

   Author: Dr. Rainer Johanni, Mar. 2003

   Last Changes:
   2003-05-16, R. Johanni: Corrected bug in put_string

 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "bufr3.h"             /* martin, org: <bufr3.h> */

#define ONES_MASK(nbits)  ( (1<<(nbits))-1 )

/* Uncomment the following line if changed reference values
   (following a 203XXX descriptor) should be treated like
   ordinary data values.
   This is mainly for compatibility with the old libraries.
*/

/* #define CHGREF_COMPAT */

static double factor[81] =
{
  1.e-40,1.e-39,1.e-38,1.e-37,1.e-36,1.e-35,1.e-34,1.e-33,1.e-32,1.e-31,
  1.e-30,1.e-29,1.e-28,1.e-27,1.e-26,1.e-25,1.e-24,1.e-23,1.e-22,1.e-21,
  1.e-20,1.e-19,1.e-18,1.e-17,1.e-16,1.e-15,1.e-14,1.e-13,1.e-12,1.e-11,
  1.e-10, 1.e-9, 1.e-8, 1.e-7, 1.e-6, 1.e-5, 1.e-4, 1.e-3, 1.e-2, 1.e-1,
    1.,
    1.e1,  1.e2,  1.e3,  1.e4,  1.e5,  1.e6,  1.e7,  1.e8,  1.e9, 1.e10,
   1.e11, 1.e12, 1.e13, 1.e14, 1.e15, 1.e16, 1.e17, 1.e18, 1.e19, 1.e20,
   1.e21, 1.e22, 1.e23, 1.e24, 1.e25, 1.e26, 1.e27, 1.e28, 1.e29, 1.e30,
   1.e31, 1.e32, 1.e33, 1.e34, 1.e35, 1.e36, 1.e37, 1.e38, 1.e39, 1.e40
};

/* Internal variables for inter-routine communication */

/* for descriptor decoding (in decode_descriptors) */

static int add_width;         /* Additional width */
static int add_scale;         /* Additional scale */

#define MAX_ASSOC_FIELDS 16 /* should be plenty, I have never seen more than 1 */

static int num_assoc_fields;                   /* Number of associated fields in effect */
static int assoc_field_bits[MAX_ASSOC_FIELDS]; /* stack for bits of assoc fields */

#define MAX_RECURSION_DEPTH 50

static int recursion_depth;

/* for BUFR decoding/encoding (in decode_bufr) */

static int nsubset;                  /* Actual subset being decoded */
static unsigned long bit_position;   /* Bit position in buffer */

static int nentry;              /* Number of entries processed in actual subset */

/* Error message in case of an error */

static char error_string[4096];

/* -----------------------------------------------------------------------
   Caching of last calculated entry_descs.
   Since the tables used must be the same, we also store the version of
   the tables used.
*/

static int num_descs_saved = 0;
static unsigned char *descs_saved = 0;
static int num_ed_saved = 0;
static bufr_entry_desc *ed_saved = 0;
static int has_QAInfo_saved = 0;

extern int bufr3_tables_master_version;
extern int bufr3_tables_local_version;
extern char bufr3_tables_local_path[4096];

static int master_version_saved, local_version_saved;
static char local_path_saved[4096];

/* Quality assessment stuff */

static int QAInfo_mode=0;     /* Flag which QA Info is currently processed - 0,22,23,24,25,32 */
static int num_QA_entries=0;  /* number of QA entries (2-23-255 etc) processed */
static int QA_def_bitmap=0;   /* Flag if defining a bitmap for later use */
static int *QA_edescs=0;      /* Array of entry desc numbers of data used for QA */
static int max_QA_edescs=0;   /* allocated length of QA_edescs */
static int num_QA_edescs=0;   /* number of entries in QA_edescs */
static int *QA_present=0;     /* Array containing the numbers of present entries (defined by DATA PRESENT INDICATOR) */
static int max_QA_present=0;  /* allocated length of QA_present */
static int num_QA_present=0;  /* number of entries in QA_present */
static int num_QA_dptotal=0;  /* total number of DATA PRESENT INDICATORs processed so far */

/**************************************************************************
 *
 * Help routines.
 *
 * ensure_size checks the size of an object and reallocates it if necessary
 *
 * my_strcpy copies strings up to a maximum length either in FORTRAN style
 *           (filled up with blanks, not termination) or in C style
 *           (zero terminated)
 *
 **************************************************************************/

/*
   ensure_size:

   Checks if the size of the memory area where *ptr points to
   (which is currently allocated to (*cur_num)*width) is sufficient
   to keep min_num items of width bytes.
   If this is not the case, it reallocates *ptr (possibly changing
   its location) and adjusts *cur_num.
   The size is increased by delta*width bytes.

   The newly allocated memory area is set to 0.

   If no memory is currently allocated, *ptr must be set to 0 on entry

   Return value:
   0 upon successful completion, -BUFR3_ERR_MALLOC if realloc fails
*/

static int ensure_size(void **ptr, int *cur_num, int min_num, int width, int delta)
{
   void *newptr;

   if(min_num>*cur_num) {
      newptr = realloc(*ptr,(*cur_num+delta)*width);
      if(!newptr) {
         sprintf(error_string,"Allocation of %d additional bytes failed",delta*width);
         return -BUFR3_ERR_MALLOC;
      }
      memset((char*)newptr+(*cur_num)*width,0,delta*width);
      *ptr = newptr;
      *cur_num += delta;
   }

   return 0;
}

/*
   my_strcpy:

   Copy zero terminated string from src to dst.
   dst holds a maximum of num characters.

   If ftn_style == 0, strings are zero terminated
   (this implies that a maximum of num-1 characters are
   filled to dst so that there is space for the zero byte).

   If ftn_style == 1, dst is filled up with blanks
   if there are less than num characters in src.
*/

void my_strcpy(char *dst, char *src, int num, int ftn_style)
{
   int i;

   /* Copy a maximum of num characters to dst */

   for(i=0; i<num && src[i]!=0; i++) dst[i] = src[i];

   if(ftn_style)
   {
      /* fill up with blanks */

      for(;i<num;i++) dst[i] = ' ';
   }
   else
   {
      /* add zero byte */

      if(i==num)
         dst[num-1] = 0;
      else
         dst[i] = 0;
   }
}

/**************************************************************************
 *
 * Parsing of descriptors
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   add_entry_desc: add an entry to the entry_descs array

   expands the array if necessary

   Return value:
   0 upon successful completion
   -BUFR3_ERR_MALLOC if realloc fails
   -BUFR3_ERR_DATAWIDTH if bits > 32 for a non-character entry
*/

static int add_entry_desc(bufr3data *bufdes, entry_desc_type type,
                           int F, int X, int Y, int bits, int scale)
{
   bufr_entry_desc *e;
   tab_b_entry *tab_b_ptr;

   /* expand entry_descs array, if necessary */

   if(ensure_size((void**)&(bufdes->entry_descs),&bufdes->max_entry_descs,bufdes->num_entry_descs+1,
                  sizeof(bufr_entry_desc),1024) < 0) return -BUFR3_ERR_MALLOC;

   e = &(bufdes->entry_descs[bufdes->num_entry_descs++]);

   e->type = type;
   e->F = F;
   e->X = X;
   e->Y = Y;
   e->bits = bits;
   e->scale = scale;
   /* scales of 40 or more should never happen, but play safe ... */
   e->dscale = (scale<-40 || scale>40) ? 0. : factor[40-scale];

   /* Get table B entry pointer for X,Y */

   tab_b_ptr = bufr3_get_tab_b_entry(X,Y);

   /* set is_char for appropriate entries */

   e->is_char = (type==ED_DATA && tab_b_ptr->type==TAB_B_TYPE_CHAR) ||
                type==ED_CHARACTER || 
                ((type==ED_ASSOC_FIELD || type==ED_UNKNOWN) && bits>32);

   if(type==ED_DATA) e->refval = tab_b_ptr->refval;

   if(!e->is_char && bits>32) {
      sprintf(error_string,"Width of BUFR data = %d too big, max is 32 bits",bits);
      return -BUFR3_ERR_DATAWIDTH;
   }

   return 0;
}

/* ------------------------------------------------------------------------
   decode_descriptors: decodes numdes descriptors stored in list

   n:       Number of descriptors
   list:    List with descriptors (stored as character entities)
   bufdes:  BUFR description

   decode_descriptors calls itself recursivly in the case of sequence
   descriptors (3XXYYY) and repetitions (1XXYYY)

   During the decoding process, it builds the entry_descs array
   (by calling add_entry_desc) which contains all necessary information
   for decoding the BUFR data in the next step.

   Return value:
   0 upon successful completion
   -BUFR3_ERR_MALLOC if realloc fails (in add_entry_desc)
   -BUFR3_ERR_DESCRIPTORS if there is an error in the descriptors

*/

static int decode_descriptors(bufr3data *bufdes, int numdes, unsigned char *list)
{
   int F,X,Y,i,n,nbits,scale,res;
   int local_width = 0;
   tab_b_entry *tab_b_ptr;
   tab_d_entry *tab_d_ptr;

   /* Testing the recursion level is for safety only, we want to
      avoid that the program hangs in case of a cyclic recursion
      in the Table D description */

   recursion_depth++;

   if(recursion_depth>=MAX_RECURSION_DEPTH) {
      strcpy(error_string,"Recursion level too deep - cyclic Table D descriptors ???");
      return -BUFR3_ERR_DESCRIPTORS;
   }

   for(i=0;i<numdes;i++)
   {
      F = (list[2*i]>>6)&3;
      X = list[2*i]&63;
      Y = list[2*i+1];

      if( F == 0 )
      {
         /* F == 0: Simple data entries */

         /* Deal with associated fields */
         /* associated fields are not applied to class 31 elements */

         if(num_assoc_fields>0 && X!=31 ) {
            res = add_entry_desc(bufdes,ED_ASSOC_FIELD,F,X,Y,
                           assoc_field_bits[num_assoc_fields-1],0);
            if(res) return res;
         }

         tab_b_ptr = bufr3_get_tab_b_entry(X,Y);

         if(tab_b_ptr)
         {
            nbits = tab_b_ptr->bits;
            scale = tab_b_ptr->scale;
            if(tab_b_ptr->type != TAB_B_TYPE_CHAR &&
               tab_b_ptr->type != TAB_B_TYPE_CODE &&
               tab_b_ptr->type != TAB_B_TYPE_FLAG)
            {
               nbits += add_width;
               scale += add_scale;
            }
            res = add_entry_desc(bufdes,ED_DATA,F,X,Y,nbits,scale);
            if(res) return res;
         }
         else
         {
            /* The Table B has no defined entry for X,Y */

            if(local_width==0)
            {
               /* We can not proceed with an unknown type of unknown width */
               sprintf(error_string,"Unkown type 0-%2.2d-%3.3d in descriptor list",X,Y);
               return -BUFR3_ERR_DESCRIPTORS;
            }
            res = add_entry_desc(bufdes,ED_UNKNOWN,F,X,Y,local_width,0);
            if(res) return res;
         }

         local_width = 0;  /* Reset local_width */
      }
      else if( F == 1 )
      {
         /* F == 1: Data replication/repetition */

         res = add_entry_desc(bufdes,ED_LOOP_START,F,X,Y,0,0);
         if(res) return res;

         /* Get the index of the loop start entry */
         n = bufdes->num_entry_descs - 1;

         if(Y==0)
         {
            /* If Y == 0, this is a delayed repetition which must be immediatly followed
               by a repetition specifier */

            int Fi, Xi, Yi;
            i++;
            if(i>=numdes) {
               strcpy(error_string,"End of descriptors in repetition list");
               return -BUFR3_ERR_DESCRIPTORS;
            }
            Fi = (list[2*i]>>6)&3;
            Xi = list[2*i]&63;
            Yi = list[2*i+1];
            tab_b_ptr = bufr3_get_tab_b_entry(Xi,Yi);
            if(Fi!=0 || Xi!=31 || (Yi!=0 && Yi!=1 && Yi!=2 && Yi!=11 && Yi!=12) || !tab_b_ptr ) {
               sprintf(error_string,"Illegal Repetition specifier %d %2.2d %3.3d",Fi,Xi,Yi);
               return -BUFR3_ERR_DESCRIPTORS;
            }

            res = add_entry_desc(bufdes,ED_DATA,Fi,Xi,Yi,tab_b_ptr->bits,0);
            if(res) return res;
         }

         /* Add entry descriptor for loop counter */

         res = add_entry_desc(bufdes,ED_LOOP_COUNTER,0,0,0,0,0);
         if(res) return res;

         /* Check if there are enough remaining descriptors */

         if( i+X >= numdes ) {
            strcpy(error_string,"End of descriptors in repetition list");
            return -BUFR3_ERR_DESCRIPTORS;
         }

         /* Call decode_descriptors recursivly to parse the loop body */

         res = decode_descriptors(bufdes,X,list+2*(i+1));
         if(res) return res;
         i += X;

         /* Add End-of-Loop marker */

         res = add_entry_desc(bufdes,ED_LOOP_END,0,0,0,0,0);
         if(res) return res;

         /* Save the index of the end of loop marker */
         bufdes->entry_descs[n].loopend = bufdes->num_entry_descs - 1;
      }
      else if( F == 2 )
      {
         /* F == 2: Data description operators */

         switch(X)
         {
            case  1:  /* Change data width */
               res = add_entry_desc(bufdes,ED_DATA_DESC_OP,F,X,Y,0,0);
               if(res) return res;
               if(Y != 0) {
                  add_width += Y-128;
               } else {
                  add_width = 0;
               }
               break;

            case  2:  /* Change scale */
               res = add_entry_desc(bufdes,ED_DATA_DESC_OP,F,X,Y,0,0);
               if(res) return res;
               if(Y != 0) {
                  add_scale += Y-128;
               } else {
                  add_scale = 0;
               }
               break;

            case  3:  /* Change reference values */
               res = add_entry_desc(bufdes,ED_DATA_DESC_OP,F,X,Y,0,0);
               if(res) return res;
               if(Y != 0) {
                  if(Y == 255) {
                     strcpy(error_string,"Misplaced 2-03-255 descriptor");
                     return -BUFR3_ERR_DESCRIPTORS;
                  }

                  /* The following values are changed reference values until
                     a 2-03-255 descriptor is encountered */

                  for(;;) {
                     int Fi,Xi,Yi;
                     i++;
                     if(i>=numdes) {
                        strcpy(error_string,"End of descriptors in change reference values list");
                        return -BUFR3_ERR_DESCRIPTORS;
                     }
                     Fi = (list[2*i]>>6)&3;
                     Xi = list[2*i]&63;
                     Yi = list[2*i+1];
                     if(Fi==2 && Xi==3 && Yi==255) {
                        /* End of list */
                        res = add_entry_desc(bufdes,ED_DATA_DESC_OP,Fi,Xi,Yi,0,0); /* For debug purposes only */
                        if(res) return res;
                        break;
                     }
                     if(Fi!=0) {
                        sprintf(error_string,"Illegal descriptor %d-%2.2d-%3.3d in change reference values list",Fi,Xi,Yi);
                        return -BUFR3_ERR_DESCRIPTORS;
                     }
                     tab_b_ptr = bufr3_get_tab_b_entry(Xi,Yi);
                     if(tab_b_ptr) {
                        res = add_entry_desc(bufdes,ED_CHANGE_REF,Fi,Xi,Yi,Y,tab_b_ptr->scale+add_scale);
                        if(res) return res;
                     } else {
                        res = add_entry_desc(bufdes,ED_CHANGE_REF,Fi,Xi,Yi,Y,0);
                        if(res) return res;
                     }
                  }
               } else {
                  /* Reset reference values to original ones for Y == 0 */
                  /* Nothing to do here */
               }
               break;

            case  4:  /* Add associated field */
               res = add_entry_desc(bufdes,ED_DATA_DESC_OP,F,X,Y,0,0);
               if(res) return res;
               /* The tricky part here is that a value Y > 0 adds to the currently defined
                  associated field, a value Y==0 cancels only the most recently addition.
                  We have to use a stack like mechanism here! */
               if(Y != 0) {
                  if(num_assoc_fields >= MAX_ASSOC_FIELDS-1) {
                     strcpy(error_string,"MAX_ASSOC_FIELDS exceeded");
                     return -BUFR3_ERR_DESCRIPTORS;
                  }
                  if(num_assoc_fields>0) {
                     assoc_field_bits[num_assoc_fields] = 
                        assoc_field_bits[num_assoc_fields-1] + Y;
                  } else {
                     assoc_field_bits[0] = Y;
                  }
                  num_assoc_fields++;
               } else {
                  /* Cancel most recently defined associated field */
                  if(num_assoc_fields>0) num_assoc_fields--;
               }
               break;

            case 5:  /* Character data */
               res = add_entry_desc(bufdes,ED_CHARACTER,F,X,Y,8*Y,0);
               if(res) return res;
               break;

            case 6:  /* Signify data width for the following local descriptor */
               res = add_entry_desc(bufdes,ED_DATA_DESC_OP,F,X,Y,0,0);
               if(res) return res;
               /* Check if next descriptor has F == 0 */
               if(i+1>=numdes) {
                  strcpy(error_string,"End of descriptors after 2-06-YYY descriptor");
                  return -BUFR3_ERR_DESCRIPTORS;
               }
               if((list[2*(i+1)]&0xc0)!=0) {
                  strcpy(error_string,"Illegal descriptor after 2-06-YYY");
                  return -BUFR3_ERR_DESCRIPTORS;
               }

               local_width = Y; /* Will be used in the next loop iteration */
               break;

            case 22: /* Quality assessment information (which has data in BUFR) */
            case 23:
            case 24:
            case 25:
            case 32:
               bufdes->has_QAInfo = 1;
               if(Y==255) {
                  res = add_entry_desc(bufdes,ED_QAINFO,F,X,Y,0,0);
               } else {
                  res = add_entry_desc(bufdes,ED_DATA_DESC_OP,F,X,Y,0,0);
               }
               if(res) return res;
               break;

            default: /* Unknown data description operator */
               res = add_entry_desc(bufdes,ED_DATA_DESC_OP,F,X,Y,0,0);
               if(res) return res;
               break;
         }
      }
      else if( F == 3 )
      {
         /* F == 3 means a sequence of descriptors defined in table D.
            Just call decode_descriptors recursivly with this descriptor sequence */

         tab_d_ptr = bufr3_get_tab_d_entry(X,Y);
         if(tab_d_ptr)
         {
            res = add_entry_desc(bufdes,ED_SEQ_START,F,X,Y,0,0); /* For debugging purposes only */
            if(res) return res;
            res = decode_descriptors(bufdes,tab_d_ptr->ndesc,tab_d_ptr->desc);
            if(res) return res;
         }
         else
         {
            sprintf(error_string,"Undefined table D entry referenced: 3-%2.2d-%3.3d",X,Y);
            return -BUFR3_ERR_DESCRIPTORS;
         }

         /* Add End-of-Sequence marker */

         res = add_entry_desc(bufdes,ED_SEQ_END,0,0,0,0,0);
         if(res) return res;
      }
   }

   recursion_depth--;
   return 0;
}

/**************************************************************************
 *
 * Decoding of BUFR data
 *
 **************************************************************************/

/* ------------------------------------------------------------------------
   add_entry: Adds one entry to the results array of the subset
              given by the "subset" parameter.

   expands the array if necessary

   Return value:
   0 upon successful completion, -BUFR3_ERR_MALLOC if realloc fails
*/

static int add_entry(bufr3data *bufdes, int subset, int data, int desc)
{
   int n;
   entry_desc_type edtype;

   /* check if this entry should be omitted */

   if(bufdes->entry_descs[desc].omit) return 0;

   /* check for space, re-allocate if necessary */

   if(ensure_size((void**)&(bufdes->results[subset]),&(bufdes->max_results[subset]),
      bufdes->num_results[subset]+1,sizeof(bufr_entry),16384) < 0) return -BUFR3_ERR_MALLOC;

   n = bufdes->num_results[subset];
   bufdes->num_results[subset]++;

   edtype = bufdes->entry_descs[desc].type;
   if(edtype != ED_LOOP_START && edtype != ED_LOOP_END && edtype != ED_LOOP_COUNTER)
      bufdes->nli_results[subset]++;

   bufdes->results[subset][n].data = data;
   bufdes->results[subset][n].desc = desc;

   return 0;
}

/* ------------------------------------------------------------------------
   add_entry_to_all: if BUFR is compressed, add an entry to all subsets,
                     else add entry to actual subset

   Return value:
   0 upon successful completion, -BUFR3_ERR_MALLOC if realloc fails in add_entry
*/

static int add_entry_to_all(bufr3data *bufdes, int data, int desc)
{
   int res;

   /* check if this entry should be omitted */

   if(bufdes->entry_descs[desc].omit) return 0;

   if(bufdes->is_compressed)
   {
      int i;
      for(i=0;i<bufdes->num_subsets;i++) {
         res = add_entry(bufdes,i,data,desc);
         if(res) return res;
      }
   }
   else
   {
      res = add_entry(bufdes,nsubset,data,desc);
      if(res) return res;
   }
   return 0;
}

/* ------------------------------------------------------------------------
   get_bits: fetches the next nbits bits from the buffer
             and returns them right justified

   If nbits > 32 this routine increases bit_position by nbits and
   returns 0xffffffff. The caller should ensure that this never happens.

   If the the buffer would be exhausted (ie. bit_position+nbits is bigger
   than the number of bits in the buffer) this routine increases bit_position
   by nbits and returns the undefined value (= ONES_MASK(nbits)).
   Normally this should result in a fatal error, but due to bugs in some
   buffer encoding software this happens sometimes.
   The caller should check for that case and take the appropriate means.
*/

static unsigned long get_bits(bufr3data *bufdes, int nbits)
{
   unsigned char *s;
   int nr, n;
   unsigned long data;

   if(nbits==0) return 0;

   /* check for errors */

   if(nbits > 32)  {
      bit_position += nbits;
      return 0xffffffff;
   }

   if(bit_position+nbits > 8*bufdes->bin_length)
   {
      bit_position += nbits; /* Virtual bit position */
      return ONES_MASK(nbits);
   }

   /* Get current buffer position and the number of bits remaining at that position */

   s  = bufdes->bufr_data + bufdes->bin_offset + (bit_position>>3);
   nr = 8-(bit_position&7);

   if(nbits <= nr)
   {
      /* we do not cross a byte boundary */
      data = (*s) >> (nr-nbits);
   }
   else
   {
      /* we must cross a byte boundary */
      n = nbits-nr;
      data = (*s++) << n;
      for(n-=8;n>0;n-=8) data |= (*s++) << n;
      data |= (*s) >> (-n);
   }

   data &= ONES_MASK(nbits);
   bit_position += nbits;

   return data;
}

/* ------------------------------------------------------------------------
   get_string: fetches a string of nbits bits from the buffer
               and places it into the char_data array.

   If nbits is not a multiple of 8, only the rightmost nbits%8 bits of
   the FIRST character are set.
   The data is terminated by a zero byte in order to ease post processing.

   Return value:
   If the string consists completely of 1 bits, BUFR3_UNDEF_VALUE is returned,
   else the offset in char_data is returned if there was no error.
   -BUFR3_ERR_MALLOC if realloc fails.

   Since BUFR3_UNDEF_VALUE is also negative, the caller should check
   if the return value equals -BUFR3_ERR_MALLOC (and not only if it is <0).
*/

static int get_string(bufr3data *bufdes, int nbits)
{
   int i, n, undef=1;
   unsigned char *s;


   n = (nbits+7)/8;
   if(ensure_size((void**)&(bufdes->char_data),&(bufdes->char_data_len),
      bufdes->char_data_used+n+1,1,16384) < 0) return -BUFR3_ERR_MALLOC;

   s = bufdes->char_data + bufdes->char_data_used;

   if(nbits%8) {
      s[0] = get_bits(bufdes,nbits%8);
      if(s[0] != ONES_MASK(nbits%8)) undef = 0;
      for(i=1;i<n;i++) {
         s[i] = get_bits(bufdes,8);
         if(s[i] != 255) undef = 0;
      }
   } else {
      for(i=0;i<n;i++) {
         s[i] = get_bits(bufdes,8);
         if(s[i] != 255) undef = 0;
      }
   }

   s[n] = 0;

   if(undef) return BUFR3_UNDEF_VALUE; /* do not advance char_data_used! */

   bufdes->char_data_used += n+1;

   return (s-bufdes->char_data);
}

/* ------------------------------------------------------------------------
   put_bits: puts bits into the buffer, reallocates the buffer if necessary

   Return value:
   0 upon sucessful completion
   -BUFR3_ERR_MALLOC if realloc failes
   -BUFR3_ERR_DATAWIDTH if nbits>32
   -BUFR3_ERR_DATARANGE if data value not within range

*/

static int put_bits(bufr3data *bufdes, int nbits, int bits)
{
   unsigned char *s;
   int nr, n;
   unsigned long data;

   if(nbits==0) return 0;

   /* check for errors */

   if(nbits > 32) {
      /* This case should never happen since it is already checked when nbits is calculated */
      sprintf(error_string,"Width of data to be put into BUFR = %d too big, max is 32 bits",nbits);
      return -BUFR3_ERR_DATAWIDTH;
   }

   /* make buffer bigger if necessary */

   n = ((bit_position+nbits+7)>>3) + bufdes->bin_offset + 8; /* Bytes needed */
   if(ensure_size((void**)&(bufdes->bufr_data),&(bufdes->totalbytes),n,1,16384) < 0)
      return -BUFR3_ERR_MALLOC;

   if(bits == BUFR3_UNDEF_VALUE)
   {
      data = ONES_MASK(nbits);
   }
   else
   {
      /* check if data is correct */
      if(bits & ~ONES_MASK(nbits))
      {
         sprintf(error_string,"Value of data to be put into BUFR out of range: %d (range: 0..%d)",
                 bits,ONES_MASK(nbits));
         return -BUFR3_ERR_DATARANGE;
      }
      else
      {
         data = bits;
      }
   }

   /* Get current buffer position and the number of bits remaining at that position */

   s  = bufdes->bufr_data + bufdes->bin_offset + (bit_position>>3);
   nr = 8-(bit_position&7);

   if(nbits <= nr)
   {
      /* we do not cross a byte boundary */
      *s |= data << (nr-nbits);
   }
   else
   {
      /* we must cross a byte boundary */
      n = nbits-nr;
      (*s++) |= (data>>n)&ONES_MASK(nr);
      for(n-=8;n>0;n-=8) (*s++) = data>>n;
      (*s) = data<<(-n);
   }

   bit_position += nbits;

   return 0;
}

/* ------------------------------------------------------------------------
   put_string: put a string of nbits bits into the buffer.
               nsubset, nentry are the position in the centry member in bufdes.
               If nsubset<0 or the respective entry member contains
               BUFR3_UNDEF_VALUE an undefined string is put into the buffer

   If nbits is not a multiple of 8, only the rightmost nbits%8 bits of
   the FIRST character are used.

   Return value:
   0 upon sucessful completion
   -BUFR3_ERR_MALLOC if realloc failes
*/

static int put_string(bufr3data *bufdes, int nbits, int nsubset, int nentry)
{
   int i, n, k, undef, res;

   undef = 0;
   if(nsubset<0 || bufdes->entries[nsubset][nentry] == BUFR3_UNDEF_VALUE) undef=1;

   n = (nbits+7)/8;
   k = nbits%8;
   if (k==0) k=8;

   for(i=0;i<n;i++) {
      if(undef) {
         res = put_bits(bufdes,k,ONES_MASK(k));
         if(res<0) return res;
      } else {
         res = put_bits(bufdes,k,bufdes->centries[nsubset][nentry][i]);
         if(res<0) return res;
      }
      k = 8;
   }

   return 0;
}

/* ------------------------------------------------------------------------
   decode_bufr: decode BUFR data using the entry_descs array which
                has been previously assembled

   decode_bufr calls itself recursivly when it encounters a ED_LOOP_START
   tag during decoding.

   startdes is entry number where to start  in the entry_descs array,
   which is > 0 in the case of a recursive call

   decode_bufr returns at the end of the list or when a ED_LOOP_END
   tag is encountered (which means that this has been a call to
   execute a loop iteration which is now finished).

   decode_bufr fills up the results array by calling add_entry
   or add_entry_to_all

   Return value:
   0 upon successful completion
   an error code in the case of error
*/

static int decode_bufr(bufr3data *bufdes, int startdes)
{
   int F,X,Y,i,j,nbits,data,cpos,is_char,res,refval;
   int nrep, start_bit_pos, loopend, nls, nle;
   entry_desc_type type;

   for(i=startdes;i<bufdes->num_entry_descs;i++)
   {
      /* Check if the buffer is exhausted, i.e. if more bits are
         read than are in the buffer.
         Normally this should result in a fatal error, but due to bugs in some
         buffer encoding software this happens sometimes.
         We allow that up to 256 bits more than there are in the buffer are
         used, this limit of 256 is obviously arbitrary, so it could be adjusted
         if necessary */

      if(bit_position > 8*bufdes->bin_length+256) {
        strcpy(error_string,"BUFR exhausted (more bits are needed than there are in the BUFR)");
        return -BUFR3_ERR_EXHAUSTED;
      }

      type = bufdes->entry_descs[i].type;
      F = bufdes->entry_descs[i].F;
      X = bufdes->entry_descs[i].X;
      Y = bufdes->entry_descs[i].Y;
      nbits = bufdes->entry_descs[i].bits;
      refval = bufdes->entry_descs[i].refval;
      is_char = bufdes->entry_descs[i].is_char;

#ifdef DEBUG
      printf ("%d %2.2d %3.3d %-8s %c %3d %5ld",F,X,Y,bufdes->entry_descs[i].mnem,
              (is_char?'C':' '),nbits,bit_position);
      if(nbits>0 && nbits<=32 && bit_position+nbits <= 8*bufdes->bin_length)
      {
         unsigned long oldpos, bits;
         oldpos = bit_position;
         bits = get_bits(bufdes,nbits);
         bit_position = oldpos;
         printf(" 0x%8.8lx=%d",bits,bits);
      }
      if(bit_position+nbits > 8*bufdes->bin_length) {
         printf(" ??????????");
      }
      printf("\n");
      fflush(stdout);
#endif

      /* If Quality Assessment Info is used and this entry_desc is a plain
         data entry, store it for using it in the bitmap */
      /* RJ: I am not sure about ED_CHARACTER (205YYY) */

      if(bufdes->has_QAInfo && !QAInfo_mode && (type==ED_DATA || type==ED_UNKNOWN || type==ED_CHARACTER) )
      {
         if(ensure_size((void **)(&QA_edescs), &max_QA_edescs, num_QA_edescs+1, sizeof(int),16384) < 0)
            return -BUFR3_ERR_MALLOC;
         QA_edescs[num_QA_edescs++] = i;
      }

      /* Shortcut for omitting data in the most typical cases */

      if(bufdes->entry_descs[i].omit && (type==ED_DATA || type==ED_ASSOC_FIELD || type==ED_CHARACTER) )
      {
         bit_position += nbits;
         if(bufdes->is_compressed && (type==ED_DATA || type==ED_ASSOC_FIELD) )
         {
            nbits = get_bits(bufdes,6);
            if(is_char) nbits *= 8;
            bit_position += nbits*bufdes->num_subsets;
         }
         continue;
      }

      switch(type)
      {
         case ED_QAINFO:

            /* Error checks: We must be in the correct mode, building of bitmap must be finished
               and there may be no more ED_QAINFO type entries than present entries in bitmap */

            if(X != QAInfo_mode) {
               sprintf(error_string,"2-%2.2d-255 without preceeding 2-%2.2d-000",X,X);
               return -BUFR3_ERR_DESCRIPTORS;
            }
            if(num_QA_dptotal < num_QA_edescs) {
               sprintf(error_string,"QAInfo bitmap too small - has %d entries, needs %d",
                       num_QA_dptotal, num_QA_edescs);
               return -BUFR3_ERR_QAINFO;
            }
            if(num_QA_entries >= num_QA_present) {
               sprintf(error_string,"trying to retrieve too many 2-%2.2d-255 entries",X);
               return -BUFR3_ERR_QAINFO;
            }

            nbits = bufdes->entry_descs[QA_present[num_QA_entries]].bits;
            refval = bufdes->entry_descs[QA_present[num_QA_entries]].refval;
            if(X==25) {
               /* For 2 25 255: width n+1, refval -2**n */
               refval = -(1<<nbits);
               nbits++;
            }
            num_QA_entries++;

            /* Fall through - no break here! */

         case ED_DATA:
         case ED_ASSOC_FIELD:
         case ED_UNKNOWN:

            if(is_char)
            {
               /* This is character data or unknown data with more than 32 bits */

               cpos = get_string(bufdes,nbits);
               if(cpos == -BUFR3_ERR_MALLOC) return cpos;

               if(bufdes->is_compressed)
               {
                  /* For compressed character data the number of bytes (not bits!) follows.
                     If this number is 0, the strings are identical and encoded in the
                     base field, else they are in the difference fields and the
                     base field doesn't matter at all.

                     We don't know, what to do for unknown data, but we assume
                     it is also character when nbits > 32

                     We can not deal with associated fields > 32 bits in compressed BUFRs
                  */

                  if(type==ED_ASSOC_FIELD) {
                     sprintf(error_string,"Can not deal with associated fields of width"
                                          " %d in compressed BUFRs",nbits);
                     return -BUFR3_ERR_DATAWIDTH;
                  }

                  nbits = get_bits(bufdes,6)*8;

                  if(nbits) {
                     for(j=0;j<bufdes->num_subsets;j++)
                     {
                        cpos = get_string(bufdes,nbits);
                        if(cpos == -BUFR3_ERR_MALLOC) return cpos;
                        res = add_entry(bufdes,j,cpos,i);
                        if(res) return res;
                     }
                  } else {
                     res = add_entry_to_all(bufdes,cpos,i);
                     if(res) return res;
                  }
               }
               else
               {
                  res = add_entry(bufdes,nsubset,cpos,i);
                  if(res) return res;
               }
            }
            else
            {
               /* This is numeric data */

               data = get_bits(bufdes,nbits);
               if(data == ONES_MASK(nbits))
                  data = BUFR3_UNDEF_VALUE;
               else
                  data += refval;

               if(bufdes->is_compressed)
               {
                  nbits = get_bits(bufdes,6);
#ifdef DEBUG
                  printf(" ... number of difference bits 0x%x=%d\n",nbits,nbits);
                  fflush(stdout);
#endif
                  if(nbits>32) {
                     sprintf(error_string,"Width of compressed data = %d too big, max is 32 bits",nbits);
                     return -BUFR3_ERR_DATAWIDTH;
                  }
                  for(j=0;j<bufdes->num_subsets;j++)
                  {
                     unsigned long diff = get_bits(bufdes,nbits);
#ifdef DEBUG
                     if(nbits) printf("     %3d 0x%x=%d\n",j,diff,diff);
#endif
                     /* The data is undefined if the base value was undefined
                        or if nbits > 0 and diff is undefined */
                     if(data==BUFR3_UNDEF_VALUE || (nbits>0 && diff==ONES_MASK(nbits))) {
                        res = add_entry(bufdes,j,BUFR3_UNDEF_VALUE,i);
                        if(res) return res;
                     } else {
                        res = add_entry(bufdes,j,data+diff,i);
                        if(res) return res;
                     }
                  }
               }
               else
               {
                  res = add_entry(bufdes,nsubset,data,i);
                  if(res) return res;
               }
            }

            /* If QA info is processed, check for DATA PRESENT INDICATOR */

            if(QAInfo_mode && type == ED_DATA && X==31 && Y==31)
            {
               /* DATA PRESENT INDICATOR - build up bitmap */

               if(num_QA_dptotal >= num_QA_edescs) {
                  sprintf(error_string,"QAInfo bitmap too big - must have %d entries\n",num_QA_edescs);
                  return -BUFR3_ERR_QAINFO;
               }

               /* Get value from results - a value of 0 means defined!!!
                  We assume that the entries are identical for compressed BUFRs */

               if(!bufdes->results[nsubset][bufdes->num_results[nsubset]-1].data) {
                 if(ensure_size((void **)(&QA_present), &max_QA_present, num_QA_present+1, sizeof(int),16384) < 0)
                    return -BUFR3_ERR_MALLOC;
                  QA_present[num_QA_present++] = QA_edescs[num_QA_dptotal];
               }
               num_QA_dptotal++;
            }

            break;

         case ED_CHANGE_REF: /* change reference values */

            data = get_bits(bufdes,nbits);
            /* if first bit is set, this is a negative value */
            if( data&(1<<(nbits-1)) ) data = -(data & ONES_MASK(nbits-1));
            res = add_entry_to_all(bufdes,data,i);
            if(res) return res;

            if(bufdes->is_compressed) {
               /* Number of difference bits - must always be 0 */
               nbits = get_bits(bufdes,6);
               if(nbits!=0) {
                  sprintf(error_string,"Number of difference bits for change reference"
                                       " values is %d, must be 0!",nbits);
                  return -BUFR3_ERR_COMPRESSED;
               }
            }

            /* change all reference values of corresponding data entries
               until end of descriptors or a 2 03 000 descriptor */

            for(j=i+1;j<bufdes->num_entry_descs;j++)
            {
                if(bufdes->entry_descs[j].type == ED_DATA &&
                   bufdes->entry_descs[j].X == X &&
                   bufdes->entry_descs[j].Y == Y )
                {
                   bufdes->entry_descs[j].refval = data;
                }

                if(bufdes->entry_descs[j].type == ED_DATA_DESC_OP &&
                   bufdes->entry_descs[j].X == 3 &&
                   bufdes->entry_descs[j].Y == 0 ) break; /* revert to original ref values */
            }

            break;

         case ED_LOOP_START: /* Start of loop */

            loopend = bufdes->entry_descs[i].loopend;

            if(Y==0) {
               /* next entry is descriptor for rep count */
               nrep = get_bits(bufdes,bufdes->entry_descs[i+1].bits);
#ifdef DEBUG
               printf("%d %2.2d %3.3d Delayed Rep. Factor: 0x%8.8lx=%d\n",
                      bufdes->entry_descs[i+1].F,bufdes->entry_descs[i+1].X,
                      bufdes->entry_descs[i+1].Y,nrep,nrep);
               fflush(stdout);
#endif
               if(bufdes->is_compressed) {
                  /* Number of difference bits - must always be 0 */
                  nbits = get_bits(bufdes,6);
#ifdef DEBUG
                  printf(" ... number of difference bits 0x%x=%d\n",nbits,nbits);
                  fflush(stdout);
#endif
                  if(nbits!=0) {
                     sprintf(error_string,"Number of difference bits for delayed rep."
                                          " factor is %d, must be 0!",nbits);
                     return -BUFR3_ERR_COMPRESSED;
                  }
               }
            } else {
               /* repetition count is Y */
               nrep = Y;
#ifdef DEBUG
               printf("         Repetition count: %4d\n",nrep);
               fflush(stdout);
#endif
            }

            /* Add marker for start of loop. The data portion will be set
               at loop end (if the marker isn't omitted).
               For that we need the number of entries at loop start */

            res = add_entry_to_all(bufdes,0,i);
            if(res) return res;

            if(bufdes->entry_descs[i].omit) {
               nls = -1;
            } else {
               nls = bufdes->num_results[nsubset]; /* Number of entries at loop start */
            }

            if(Y==0) {
               res = add_entry_to_all(bufdes,nrep,i+1);
               if(res) return res;
               i++;
            }

            start_bit_pos = bit_position; /* get current bit position for the case
                                                    of delayed repetition */

            for(j=0;j<nrep;j++)
            {
               /* add marker for loop counter, desc is at actual position + 1 */
               res = add_entry_to_all(bufdes,j,i+1);
               if(res) return res;

               /* In the case of delayed repetition we reset the bit position to
                  the position we had at the start of the loop.
                  This way, always the same data is decoded */

               if(Y==0 && (bufdes->entry_descs[i].Y==11 || bufdes->entry_descs[i].Y==12) )
                  bit_position = start_bit_pos;

               /* call decode_bufr recursivly for decoding the loop sequence */
               /* descs for loop start at actual position + 2 */

               res = decode_bufr(bufdes,i+2);
               if(res) return res;
            }

            /* Set the data of the ED_LOOP_START to the number of items in the loop */

            if(nls>0)
            {
               nle = bufdes->num_results[nsubset]; /* Number of entries at loop end */
               if(bufdes->is_compressed)
               {
                  for(j=0;j<bufdes->num_subsets;j++) bufdes->results[j][nls-1].data = nle-nls;
               }
               else
               {
                  bufdes->results[nsubset][nls-1].data = nle-nls;
               }
            }

            /* advance i to end position */

            i = loopend;

            res = add_entry_to_all(bufdes,BUFR3_UNDEF_VALUE,i); /* marker for end of loop */
            if(res) return res;
            break;

         case ED_LOOP_END: /* End of loop indicator */

            return 0;

         case ED_CHARACTER: /* Character data from 205YYY */

            cpos = get_string(bufdes,nbits);
            if(cpos == -BUFR3_ERR_MALLOC) return cpos;
            res = add_entry_to_all(bufdes,cpos,i);
            if(res) return res;
            break;

         case ED_DATA_DESC_OP: /* Data description operators F==2, X!=5 */
            if(X<22) continue; /* Make it faster (hopefully) by omitting the following stuff */
            if( (X==22 || X==23 || X==24 || X==25 || X==32) && Y==0)
            {
               QAInfo_mode = X;
               num_QA_entries = 0;
               if(!QA_def_bitmap) {
                  /* purge bitmap */
                  num_QA_dptotal = 0;
                  num_QA_present = 0;
               }
            }
            if( X==35 && Y==0 )
            {
               /* Cancel backward data reference */
               QAInfo_mode = 0;
               /* purge bitmap */
               QA_def_bitmap = 0;
               num_QA_dptotal = 0;
               num_QA_present = 0;
               /* RJ: Should we also reset num_QA_edescs ???? */
            }
            if( X==36 && Y==0 ) QA_def_bitmap = 1;
            if( X==37 && Y==0 )
            {
               /* Use defined data present bitmap */
               /* Nothing to do here, we always reuse a present bitmap
                  as long as QA_def_bitmap is set */
            }
            if( X==37 && Y==255 )
            {
               /* Cancel use defined data present bitmap */
               /* purge bitmap */
               QA_def_bitmap = 0;
               num_QA_dptotal = 0;
               num_QA_present = 0;
            }
            break;

         case ED_SEQ_START: /* Start of sequence */
         case ED_SEQ_END:   /* End of sequence */
            res = add_entry_to_all(bufdes,BUFR3_UNDEF_VALUE,i);
            if(res) return res;
            break;

         case ED_LOOP_COUNTER: /* Loop counter marker */

            /* Should never happen !! */
            sprintf(error_string,"Internal Error: Loop counter marker encountered in decode_bufr");
            return -BUFR3_ERR_INTERNAL;
            break;
      }
   }

   return 0;
}

/* ------------------------------------------------------------------------
   encode_bufr: encode BUFR data using the entry_descs array which
                has been previously assembled

   Return value:
   0 upon successful completion
   an error code in the case of error
*/

static int encode_bufr(bufr3data *bufdes, int startdes)
{
   int F,X,Y,i,j,nbits,data,is_char,nb,res,refval;
   int nrep, loopend;
   entry_desc_type type;

   for(i=startdes;i<bufdes->num_entry_descs;i++)
   {
      type = bufdes->entry_descs[i].type;
      F = bufdes->entry_descs[i].F;
      X = bufdes->entry_descs[i].X;
      Y = bufdes->entry_descs[i].Y;
      nbits = bufdes->entry_descs[i].bits;
      refval = bufdes->entry_descs[i].refval;
      is_char = bufdes->entry_descs[i].is_char;

#ifdef DEBUG
      printf ("%d %2.2d %3.3d %-8s %c %3d %5ld %5d",F,X,Y,bufdes->entry_descs[i].mnem,
              (is_char?'C':' '),nbits,bit_position,nentry);
      if(nbits>0 && nbits<=32)
      {
         printf(" %12d",bufdes->entries[nsubset][nentry]);
      }
      printf("\n");
      fflush(stdout);
#endif

      /* Check the length of the input arrays */

      if( type==ED_DATA || type==ED_UNKNOWN || type==ED_ASSOC_FIELD ||
          type==ED_CHANGE_REF || type==ED_CHARACTER ||
          (type==ED_LOOP_START && Y==0) )
      {
         if(bufdes->is_compressed)
         {
            for(j=0;j<bufdes->num_subsets;j++)
            {
               if(bufdes->num_entries[j]<=nentry) {
                  sprintf(error_string,"Max number of entries exceeded for subset %d",j);
                  return -BUFR3_ERR_NUM_ENTRIES;
               }
            }
         }
         else
         {
            if(bufdes->num_entries[nsubset]<=nentry) {
               sprintf(error_string,"Max number of entries exceeded for subset %d",nsubset);
               return -BUFR3_ERR_NUM_ENTRIES;
            }
         }
      }

      /* Check if data is identical for compressed buffers where this is necessary */
      /* We skip this check for character data */

      if( bufdes->is_compressed &&
            (type==ED_CHANGE_REF || (type==ED_LOOP_START && Y==0)) )
      {
         data = bufdes->entries[0][nentry];
         for(j=1;j<bufdes->num_subsets;j++)
         {
             if(bufdes->entries[j][nentry] != data) {
                sprintf(error_string,"Compressed Buffer but common data differs");
                return -BUFR3_ERR_COMPRESSED;
             }
         }
      }

      /* Save index into entry_desc, if wanted */

      if(bufdes->desc_idx)
      {
         int flag = 0;
         if(type==ED_DATA || type==ED_UNKNOWN || type==ED_ASSOC_FIELD ||
            type==ED_CHANGE_REF || type==ED_CHARACTER || type==ED_QAINFO)
         {
            bufdes->desc_idx[nsubset][nentry] = i;
            flag = 1;
         }
         if(type==ED_LOOP_START && Y==0)
         {
            bufdes->desc_idx[nsubset][nentry] = i+1;
            flag = 1;
         }
         if(flag && bufdes->is_compressed)
         {
            for(j=1;j<bufdes->num_subsets;j++)
               bufdes->desc_idx[j][nentry] = bufdes->desc_idx[0][nentry];
         }
      }

      /* If Quality Assessment Info is used and this entry_desc is a plain
         data entry, store it for using it in the bitmap */
      /* RJ: I am not sure about ED_CHARACTER (205YYY) */

      if(bufdes->has_QAInfo && !QAInfo_mode && (type==ED_DATA || type==ED_UNKNOWN || type==ED_CHARACTER) )
      {
         if(ensure_size((void **)(&QA_edescs), &max_QA_edescs, num_QA_edescs+1, sizeof(int),16384) < 0)
            return -BUFR3_ERR_MALLOC;
         QA_edescs[num_QA_edescs++] = i;
      }

      switch(type)
      {
         case ED_QAINFO:

            /* Error checks: We must be in the correct mode, building of bitmap must be finished
               and there may be no more ED_QAINFO type entries than present entries in bitmap */

            if(X != QAInfo_mode) {
               sprintf(error_string,"2-%2.2d-255 without preceeding 2-%2.2d-000",X,X);
               return -BUFR3_ERR_DESCRIPTORS;
            }
            if(num_QA_dptotal < num_QA_edescs) {
               sprintf(error_string,"QAInfo bitmap too small - has %d entries, needs %d",
                       num_QA_dptotal, num_QA_edescs);
               return -BUFR3_ERR_QAINFO;
            }
            if(num_QA_entries >= num_QA_present) {
               sprintf(error_string,"trying to retrieve too many 2-%2.2d-255 entries",X);
               return -BUFR3_ERR_QAINFO;
            }

            nbits = bufdes->entry_descs[QA_present[num_QA_entries]].bits;
            refval = bufdes->entry_descs[QA_present[num_QA_entries]].refval;
            if(X==25) {
               /* For 2 25 255: width n+1, refval -2**n */
               refval = -(1<<nbits);
               nbits++;
            }
            num_QA_entries++;

            /* Fall through - no break here! */

         case ED_DATA:
         case ED_ASSOC_FIELD:
         case ED_UNKNOWN:

            if(is_char)
            {
               /* This is character data or unknown data with more than 32 bits */

               if(bufdes->is_compressed)
               {
                  /* For compressed character data the number of bytes (not bits!) follows.
                     We always encode in the difference fields

                     We don't know, what to do for unknown data, but we assume
                     it is also character when nbits > 32

                     We can not deal with associated fields > 32 bits in compressed BUFRs
                  */

                  if(type==ED_ASSOC_FIELD) {
                     sprintf(error_string,"Can not deal with associated fields of width"
                                          " %d in compressed BUFRs",nbits);
                     return -BUFR3_ERR_DATAWIDTH;
                  }

                  res = put_string(bufdes,nbits,-1,0);
                  if(res) return res;
                  nb = (nbits+7)/8;
                  res = put_bits(bufdes,6,nb);
                  if(res) return res;
                  nb *= 8;
                  for(j=0;j<bufdes->num_subsets;j++)
                  {
                     res = put_string(bufdes,nb,j,nentry);
                     if(res) return res;
                  }
               }
               else
               {
                  res = put_string(bufdes,nbits,nsubset,nentry);
                  if(res) return res;
               }
            }
            else
            {
               /* This is numeric data */

               if(bufdes->is_compressed)
               {
                  int numdef, minval=0, maxval=0;

                  /* Get minimum value */

                  numdef = 0; /* number of defined values */
                  for(j=0;j<bufdes->num_subsets;j++)
                  {
                     data = bufdes->entries[j][nentry];
                     if(data == BUFR3_UNDEF_VALUE) continue;
                     if(numdef==0) {
                        minval = data;
                        maxval = data;
                     } else {
                        if(data < minval) minval = data;
                        if(data > maxval) maxval = data;
                     }
                     numdef++;
                  }

                  /* Check if all are undefined */

                  if(numdef == 0)
                  {
                     res = put_bits(bufdes,nbits,BUFR3_UNDEF_VALUE);
                     if(res) return res;
                     res = put_bits(bufdes,6,0);
                     if(res) return res;
                  }
                  else
                  {
                     data = minval;
                     data -= refval;
                     res = put_bits(bufdes,nbits,data);
                     if(res) return res;
                     if(numdef == bufdes->num_subsets && minval == maxval)
                     {
                        /* Special case: all values are equal */
                        res = put_bits(bufdes,6,0);
                        if(res) return res;
                     }
                     else
                     {
                        /* calculate, how may bits we need for encoding the difference.
                           Since a value with all 1's is UNDEFINED, the number of bits
                           must be big enough to hold maxval-minval+1 */

                        for(nb=1;nb<30;nb++) if ( (1<<nb) > (maxval-minval+1) ) break;

                        res = put_bits(bufdes,6,nb);
                        if(res) return res;

                        for(j=0;j<bufdes->num_subsets;j++)
                        {
                           data = bufdes->entries[j][nentry];
                           if(data == BUFR3_UNDEF_VALUE) {
                              res = put_bits(bufdes,nb,BUFR3_UNDEF_VALUE);
                              if(res) return res;
                           } else {
                              res = put_bits(bufdes,nb,data-minval);
                              if(res) return res;
                           }
                        }
                     }
                  }
               }
               else
               {
                  data = bufdes->entries[nsubset][nentry];
                  if(data == BUFR3_UNDEF_VALUE) {
                     res = put_bits(bufdes,nbits,BUFR3_UNDEF_VALUE);
                     if(res) return res;
                  } else {
                     data -= refval;
                     res = put_bits(bufdes,nbits,data);
                     if(res) return res;
                  }
               }
            }

            /* If QA info is processed, check for DATA PRESENT INDICATOR */

            if(QAInfo_mode && type == ED_DATA && X==31 && Y==31)
            {
               /* DATA PRESENT INDICATOR - build up bitmap */

               if(num_QA_dptotal >= num_QA_edescs) {
                  sprintf(error_string,"QAInfo bitmap too big - must have %d entries\n",num_QA_edescs);
                  return -BUFR3_ERR_QAINFO;
               }

               /* Get value from results - a value of 0 means defined!!!
                  We assume that the entries are identical for compressed BUFRs */

               if(!bufdes->entries[nsubset][nentry]) {
                 if(ensure_size((void **)(&QA_present), &max_QA_present, num_QA_present+1, sizeof(int),16384) < 0)
                    return -BUFR3_ERR_MALLOC;
                  QA_present[num_QA_present++] = QA_edescs[num_QA_dptotal];
               }
               num_QA_dptotal++;
            }

            nentry++;
            break;

         case ED_CHANGE_REF: /* change reference values */

            data = bufdes->entries[nsubset][nentry++];

            /* change all reference values of corresponding data entries
               until end of descriptors or a 2 03 000 descriptor */

            for(j=i+1;j<bufdes->num_entry_descs;j++)
            {
                if(bufdes->entry_descs[j].type == ED_DATA &&
                   bufdes->entry_descs[j].X == X &&
                   bufdes->entry_descs[j].Y == Y )
                {
                   bufdes->entry_descs[j].refval = data;
                }

                if(bufdes->entry_descs[j].type == ED_DATA_DESC_OP &&
                   bufdes->entry_descs[j].X == 3 &&
                   bufdes->entry_descs[j].Y == 0 ) break; /* revert to original ref values */
            }

            /* if this is a negative value, take the absolute value and set first bit */
            if (data < 0) data = (-data) | (1<<(nbits-1));
            res = put_bits(bufdes,nbits,data);
            if(res) return res;

            if(bufdes->is_compressed) {
               /* Number of difference bits - we already checked if all values are
                  equal, therefore this is always 0 */
               res = put_bits(bufdes,6,0);
               if(res) return res;
            }

            break;

         case ED_LOOP_START: /* Start of loop */

            loopend = bufdes->entry_descs[i].loopend;

            if(Y==0) {
               /* next entry is descriptor for rep count */
               nrep = bufdes->entries[nsubset][nentry++];
               res = put_bits(bufdes,bufdes->entry_descs[i+1].bits,nrep);
               if(res) return res;
               if(bufdes->is_compressed) {
                  /* Number of difference bits - must always be 0 */
                  res = put_bits(bufdes,6,0);
                  if(res) return res;
               }
               i++; /* advance loop counter */
            } else {
               /* repetition count is Y */
               nrep = Y;
            }
#ifdef DEBUG
            printf("         Repetition count: %4d\n",nrep);
            fflush(stdout);
#endif

            /* In the case of delayed repetition we expect the data only once
               and output it only once */

            if(Y==0 && (bufdes->entry_descs[i].Y==11 || bufdes->entry_descs[i].Y==12) ) nrep=1;

            for(j=0;j<nrep;j++)
            {
               /* call encode_bufr recursivly for decoding the loop sequence */
               /* descs for loop start at actual position + 2 */

               res = encode_bufr(bufdes,i+2);
               if(res) return res;
            }

            /* advance i to end position */

            i = loopend;
            break;

         case ED_LOOP_END: /* End of loop indicator */

            return 0;

         case ED_CHARACTER: /* Character data from 205YYY */

            res = put_string(bufdes,nbits,nsubset,nentry++);
            if(res) return res;
            break;

         case ED_DATA_DESC_OP: /* Data description operators F==2, X!=5 */
            if(X<22) continue; /* Make it faster (hopefully) by omitting the following stuff */
            if( (X==22 || X==23 || X==24 || X==25 || X==32) && Y==0)
            {
               QAInfo_mode = X;
               num_QA_entries = 0;
               if(!QA_def_bitmap) {
                  /* purge bitmap */
                  num_QA_dptotal = 0;
                  num_QA_present = 0;
               }
            }
            if( X==35 && Y==0 )
            {
               /* Cancel backward data reference */
               QAInfo_mode = 0;
               /* purge bitmap */
               QA_def_bitmap = 0;
               num_QA_dptotal = 0;
               num_QA_present = 0;
               /* RJ: Should we also reset num_QA_edescs ???? */
            }
            if( X==36 && Y==0 ) QA_def_bitmap = 1;
            if( X==37 && Y==0 )
            {
               /* Use defined data present bitmap */
               /* Nothing to do here, we always reuse a present bitmap
                  as long as QA_def_bitmap is set */
            }
            if( X==37 && Y==255 )
            {
               /* Cancel use defined data present bitmap */
               /* purge bitmap */
               QA_def_bitmap = 0;
               num_QA_dptotal = 0;
               num_QA_present = 0;
            }
            break;

         case ED_SEQ_START: /* Start of sequence */
         case ED_SEQ_END:   /* End of sequence */

            /* Nothing to do here */
            break;

         case ED_LOOP_COUNTER: /* Loop counter marker */

            /* Should never happen !! */
            sprintf(error_string,"Internal Error: Loop counter marker encountered in encode_bufr");
            return -BUFR3_ERR_INTERNAL;
            break;
      }
   }

   return 0;
}

/* ------------------------------------------------------------------------
   mnem_extend: extend a mnemonic for uniqueness

   If number > 0, add a letter to the mnemonic.
   The letter is taken from the range 0..9, A..P, R..Z
   This is mainly for compatibilty with the old DWD BUFR programs.
*/

static char *extensions = "0123456789ABCDEFGHIJKLMNOPRSTUVWXYZ";

static void mnem_extend(char *mnem, int number)
{
   int k, c;

   if(number==0) return;

   if (number<=strlen(extensions)) {
      c = extensions[number-1];
   } else {
      c = '*';
   }

   for(k=0;k<BUFR3_MNEM_LEN;k++)
      if( mnem[k]==0 ) { mnem[k] = c; mnem[k+1] = 0; break; }
}


/* ------------------------------------------------------------------------

   add_mnemonics:

   Add mnemonic to the entry_descs structure.
   This is done in a separate rountine (and not in add_entry_desc)
   because it is mainly for compatibility with old BUFR programs
   and could also be omitted if these are not needed.
*/

/* the following macro prints the number n into the string s with exactly
   3 digits (ie. with leading zeros).
   The result is undefined if n > 999 or n < 0.
   This is exactly the same (but significantly faster) as:

   sprintf(s,"%3.3d",n)
*/

#define FORMAT_3DIGIT_NUM(s,n) \
*(s)=(n)/100+'0'; *((s)+1)=((n)/10)%10+'0'; *((s)+2)=(n)%10+'0'; *((s)+3)=0

static void add_mnemonics(bufr3data *bufdes)
{
   int i, k, F, X, Y, num, num_char, num_loops, num_chgref;
   unsigned char num_data[64][256]; /* unsigned char instead of int for speed reasons */
   bufr_entry_desc *e;
   tab_b_entry *tab_b_ptr;

   num_char = num_loops = num_chgref = 0;
   memset(num_data,0,sizeof(num_data));

   /* Loop over the entry_descs */

   for(i=0;i<bufdes->num_entry_descs;i++)
   {
      e = bufdes->entry_descs + i;
      
      F = e->F;
      X = e->X;
      Y = e->Y;

      /* Set default mnemonic: FXXYYY */

      e->mnem[0] = F + '0';
      e->mnem[1] = X/10 + '0';
      e->mnem[2] = X%10 + '0';
      FORMAT_3DIGIT_NUM(e->mnem+3,Y);

      tab_b_ptr = bufr3_get_tab_b_entry(X,Y);

      switch(e->type)
      {
         case ED_DATA: /* Simple data entries */
            num = num_data[X][Y]++;
            strcpy(e->mnem,tab_b_ptr->mnem);
            mnem_extend(e->mnem,num);
            break;

         case ED_ASSOC_FIELD: /* Associated field */
            num = num_data[X][Y];
            if (tab_b_ptr) strcpy(e->mnem,tab_b_ptr->mnem);
            mnem_extend(e->mnem,num);
            /* Add the letter 'Q' */
            for(k=0;k<BUFR3_MNEM_LEN;k++)
               if(e->mnem[k]==0) { e->mnem[k] = 'Q'; e->mnem[k+1] = 0; break; }
            break;

         case ED_UNKNOWN: /* Unknown descriptor */
            num = num_data[X][Y]++;
            mnem_extend(e->mnem,num);
            break;

         case ED_CHARACTER: /* Character */
            num = num_char++;
            strcpy (e->mnem,"YSUPL");
            mnem_extend(e->mnem,num);
            break;

         case ED_LOOP_START: /* Data replication/repetition */
            num = num_loops++;
            strcpy(e->mnem,"Loop");
            FORMAT_3DIGIT_NUM(e->mnem+4,num);
            break;

         case ED_LOOP_END:
            strcpy(e->mnem,"EndLoop");
            break;

         case ED_LOOP_COUNTER:
            strcpy(e->mnem,"Lcnt");
            FORMAT_3DIGIT_NUM(e->mnem+4,num_loops-1);
            break;

         case ED_CHANGE_REF: /* Change reference value */
#ifdef CHGREF_COMPAT
            num = num_data[X][Y]++;
            if(tab_b_ptr) strcpy (e->mnem,tab_b_ptr->mnem);
            mnem_extend(e->mnem,num);
#else
            num = num_chgref++;
            strcpy (e->mnem,"CHGREF");
            mnem_extend(e->mnem,num);
#endif
            break;

         case ED_DATA_DESC_OP:
         case ED_QAINFO:
            break;

         case ED_SEQ_START:
            break;

         case ED_SEQ_END:
            strcpy(e->mnem,"End Seq");
            break;
      }
   }
}

/**************************************************************************
 *
 * User Interface routines
 *
 **************************************************************************/

/**************************************************************************
 *
 * BUFR3_read_record: read next record from file fd.
 * Skips a maximum of max_chars characters until the string BUFR is found.
 * The location where pdata points to is set to a pointer to the data object
 * which is allocated in this routine.
 * The user is responsible of deallocating this memory space after use.
 * This pointer will be zero if EOF was reached during read or
 * an error happened.
 *
 * Return values:
 * 0 on success or EOF
 * A negative error number in the case of an error
 *
 **************************************************************************/

int BUFR3_read_record(FILE *fd, int max_chars, unsigned char **pdata)
{
   unsigned char c[4];
   int n=0, a;

   *pdata = 0;

   /* read first 4 characters */

   if(fread(c,1,4,fd) != 4) return 0; /* EOF */

   while(c[0]!='B' || c[1]!='U' || c[2]!='F' || c[3]!='R')
   {
      if(n>=max_chars) {
         strcpy(error_string,"String BUFR not found in input file");
         return -BUFR3_ERR_NO_BUFR;
      }

      a = fgetc(fd);
      if(a==EOF) return 0;

      n++; /* number of characters read after the first 4 */

      /* Shift search string */
      c[0]=c[1]; c[1]=c[2]; c[2]=c[3]; c[3]=a;
   }

   /* When we are here, we got the BUFR header,
      read next 4 characters for getting length */

   if(fread(c,1,4,fd) != 4) {
      sprintf(error_string,"EOF reading BUFR data");
      return -BUFR3_ERR_READ;
   }

   n = c[0]*256*256 + c[1]*256 + c[2];

   *pdata = (unsigned char *) malloc(n);
   if(!*pdata) {
      sprintf(error_string,"malloc of %d bytes failed",n);
      return -BUFR3_ERR_MALLOC;
   }

   /* Fill first 8 bytes with the data we have already read */

   (*pdata)[0] = 'B';  (*pdata)[1] = 'U';  (*pdata)[2] = 'F';  (*pdata)[3] = 'R';
   (*pdata)[4] = c[0]; (*pdata)[5] = c[1]; (*pdata)[6] = c[2]; (*pdata)[7] = c[3];

   /* Read remainder */

   if(fread(*pdata+8,1,n-8,fd) != n-8)
   {
      free(*pdata);
      *pdata = 0;
      sprintf(error_string,"EOF reading BUFR data (%d Bytes)",n);
      return -BUFR3_ERR_READ;
   }

   return 0;
}

/**************************************************************************
 *
 * BUFR3_decode_sections: takes BUFR 3 data and decodes the section headers.
 *
 * Delivers a pointer to a bufr3data object or NULL in case of an error.
 * The pointer is returned in the location where pbufdes points to.
 *
 * Return values:
 * 0 on success or EOF
 * A negative error number in the case of an error
 *
 **************************************************************************/

int BUFR3_decode_sections(unsigned char *data, bufr3data **pbufdes)
{
   bufr3data *bufdes;
   int nrem;

   *pbufdes = 0;

   /* Allocate the buffer structure, fill with zeros */

   bufdes = (bufr3data *) malloc(sizeof(bufr3data));
   if(!bufdes) {
      sprintf(error_string,"malloc of %d bytes failed",sizeof(bufr3data));
      return -BUFR3_ERR_MALLOC;
   }

   memset(bufdes,0,sizeof(bufr3data));

   /* bufdes->bufr_data points to the data. NB: This is not a copy!
      Therefore data must exist as long this buffer is decoded.
      It is also NOT free'd with the other stuff */

   bufdes->bufr_data = data;
   bufdes->nbytes = data[4]*256*256 + data[5]*256 + data[6];

   /* Set and check buffer edition number - should be 3 for this software */

   bufdes->iednr = bufdes->bufr_data[7];

#ifdef PRINT_WARNINGS
   if(bufdes->iednr != 3)
   {
      /* RJ: Some buffers seem to have 2, ????? */
      fprintf(stderr,"WARNING: Buffer edition number is %d (should be 3)\n",bufdes->bufr_data[7]);
   }
#endif

   nrem = bufdes->nbytes - 8; /* Number of bytes remaining */

   /* Split buffer data into sections, check for errors */


   /* Section 1 */

   bufdes->sect1 = bufdes->bufr_data + 8; /* Section 1 starts always after 8 bytes */
   bufdes->sect1_length = bufdes->sect1[0]*256*256 + bufdes->sect1[1]*256 + bufdes->sect1[2];

   /* Safety chechks */

   if(bufdes->sect1_length<18)
   {
      sprintf(error_string,"Section 1 length: %d (must be at least 18)",bufdes->sect1_length);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }
   if(bufdes->sect1_length>nrem)
   {
      sprintf(error_string,"Section 1 length %d > max. length: %d",bufdes->sect1_length,nrem);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }
   nrem -= bufdes->sect1_length;


   /* Section 2, if present */

   if( (bufdes->sect1[7]&0x80) == 0x80 )
   {
      bufdes->sect2 = bufdes->sect1 + bufdes->sect1_length;
      bufdes->sect2_length = bufdes->sect2[0]*256*256 + bufdes->sect2[1]*256 + bufdes->sect2[2];

      /* Safety chechks */

      if(bufdes->sect2_length<4)
      {
         sprintf(error_string,"Section 2 length: %d (must be at least 4)",bufdes->sect2_length);
         free(bufdes);
         return -BUFR3_ERR_SECTIONS;
      }
      if(bufdes->sect2_length>nrem)
      {
         sprintf(error_string,"Section 2 length: %d max. length: %d",bufdes->sect2_length,nrem);
         free(bufdes);
         return -BUFR3_ERR_SECTIONS;
      }
      nrem -= bufdes->sect2_length;

      bufdes->sect3 = bufdes->sect2 + bufdes->sect2_length;
   }
   else
   {
      bufdes->sect2 = 0;
      bufdes->sect2_length = 0;
      bufdes->sect3 = bufdes->sect1 + bufdes->sect1_length;
   }


   /* Section 3 */

   /* bufdes->sect3 is already set */
   bufdes->sect3_length = bufdes->sect3[0]*256*256 + bufdes->sect3[1]*256 + bufdes->sect3[2];

   /* Safety chechks */

   if(bufdes->sect3_length<8)
   {
      sprintf(error_string,"Section 3 length: %d (must be at least 8)",bufdes->sect3_length);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }
   if(bufdes->sect3_length>nrem)
   {
      sprintf(error_string,"Section 3 length %d > max. length: %d",bufdes->sect3_length,nrem);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }
   nrem -= bufdes->sect3_length;

   /* Additional variables */

   bufdes->num_subsets   = bufdes->sect3[4]*256 + bufdes->sect3[5];
   bufdes->is_observed   = (bufdes->sect3[6]&0x80) == 0x80;
   bufdes->is_compressed = (bufdes->sect3[6]&0x40) == 0x40;
   bufdes->descriptors   = bufdes->sect3 + 7;
   bufdes->num_descriptors = (bufdes->sect3_length-7)/2;


   /* Section 4 */

   bufdes->sect4 = bufdes->sect3 + bufdes->sect3_length;
   bufdes->sect4_length = bufdes->sect4[0]*256*256 + bufdes->sect4[1]*256 + bufdes->sect4[2];

   /* Safety chechks */

   if(bufdes->sect4_length<4)
   {
      sprintf(error_string,"Section 4 length: %d (must be at least 4)",bufdes->sect4_length);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }
   if(bufdes->sect4_length>nrem)
   {
      sprintf(error_string,"Section 4 length %d > max. length: %d",bufdes->sect4_length,nrem);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }
   nrem -= bufdes->sect4_length;

   /* Additional variables */

   bufdes->bin_offset = 8 + bufdes->sect1_length + bufdes->sect2_length + bufdes->sect3_length + 4;
   bufdes->bin_length = bufdes->sect4_length - 4;


   /* Section 5 (End section '7777') */

   bufdes->sect5 = bufdes->sect4 + bufdes->sect4_length;

   /* Safety checks */

   if(nrem<4)
   {
      sprintf(error_string,"Section 5 length: %d (must be at least 4)",nrem);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }

#ifdef PRINT_WARNINGS
   if(bufdes->sect5[0] != '7' || bufdes->sect5[1] != '7' ||
      bufdes->sect5[2] != '7' || bufdes->sect5[3] != '7')
   {
      /* This should be an error, but some buffers don't have 7777 */
      fprintf(stderr,"WARNING: Section 5 is NOT '7777': 0x%2.2x%2.2x%2.2x%2.2x\n",
              bufdes->sect5[0],bufdes->sect5[1],bufdes->sect5[2],bufdes->sect5[3]);
   }
#endif

   *pbufdes = bufdes;
   return 0;
}


/**************************************************************************
 *
 * BUFR3_parse_descriptors: Parses the descriptors in the BUFR
 *
 * Input:
 * bufdes  pointer to a bufr3data object returned by BUFR3_decode_sections.
 *
 * Output: Only via the bufr3data object.
 *
 * Return value:
 * 0 upon successful completion
 * A negative error number in the case of an error
 *
 * NB: The data given to BUFR3_decode_sections must still be alive
 *     when BUFR3_parse_descriptors is called since only a pointer to this
 *     data is kept and no copy is made for performance reasons.
 *
 **************************************************************************/

int BUFR3_parse_descriptors(bufr3data *bufdes)
{
   int res;

   if(bufdes->num_entry_descs != 0) {
      strcpy(error_string,"BUFR3_parse_descriptors called more than once");
      return -BUFR3_ERR_CALLSEQ;
   }

   /* Read the actual tables B and D */

   if(bufr3_read_tables(bufdes->sect1)<0) {
      sprintf(error_string,"ERROR reading BUFR tables: %s",bufr3_tables_get_error_string());
      return -BUFR3_ERR_TABLES;
   }

   if(num_descs_saved == bufdes->num_descriptors &&
      memcmp(descs_saved,bufdes->descriptors,2*bufdes->num_descriptors) == 0 &&
      master_version_saved == bufr3_tables_master_version &&
      local_version_saved  == bufr3_tables_local_version  &&
      strcmp(local_path_saved,bufr3_tables_local_path) == 0 )
   {
      /* we can reuse the previously calculated entry_descs */

      bufdes->entry_descs = (bufr_entry_desc *)malloc(num_ed_saved*sizeof(bufr_entry_desc));
      if(!bufdes->entry_descs) {
         strcpy(error_string,"malloc failed");
         return -BUFR3_ERR_MALLOC;
      }
      memcpy(bufdes->entry_descs,ed_saved,num_ed_saved*sizeof(bufr_entry_desc));
      bufdes->num_entry_descs = bufdes->max_entry_descs = num_ed_saved;
      bufdes->has_QAInfo = has_QAInfo_saved;
   }
   else
   {
      /* entry_descs must be calculated */

      add_width = 0;
      add_scale = 0;
      num_assoc_fields = 0;
      recursion_depth = 0;

      res = decode_descriptors(bufdes, bufdes->num_descriptors, bufdes->descriptors);
      if(res) return res;

      add_mnemonics(bufdes);

      /* Store the entry_descs for later reuse */

      if(descs_saved) free(descs_saved);
      if(ed_saved) free(ed_saved);
      descs_saved = (unsigned char *) malloc(2*bufdes->num_descriptors);
      ed_saved = (bufr_entry_desc *) malloc(bufdes->num_entry_descs*sizeof(bufr_entry_desc));
      if(!descs_saved || !ed_saved) {
         strcpy(error_string,"malloc failed");
         if(descs_saved) free(descs_saved);
         if(ed_saved) free(ed_saved);
         descs_saved=0;
         ed_saved=0;
         return -BUFR3_ERR_MALLOC;
      }
      memcpy(descs_saved,bufdes->descriptors,2*bufdes->num_descriptors);
      memcpy(ed_saved,bufdes->entry_descs,bufdes->num_entry_descs*sizeof(bufr_entry_desc));

      num_descs_saved = bufdes->num_descriptors;
      num_ed_saved = bufdes->num_entry_descs;
      has_QAInfo_saved = bufdes->has_QAInfo;

      master_version_saved = bufr3_tables_master_version;
      local_version_saved  = bufr3_tables_local_version;
      strcpy(local_path_saved,bufr3_tables_local_path);
   }

   return 0;
}


/**************************************************************************
 *
 * BUFR3_get_entry_texts:
 * Get textual descriptions of the entries in entry_descs
 *
 * Input:
 * bufdes  pointer to a bufr3data object returned by BUFR3_decode_sections.
 * len     Length of the strings in array text.
 * ftn_style
 *         If set to 1, text strings are NOT zero terminated and padded
 *         with blanks up to len.
 *         If set to 0, text strings are zero terminated and have a maximum
 *         length of len-1.
 *
 * Output:
 * text    User supplied array for receiving the textual description strings.
 *         Must have a total length of len*bufdes->num_entry_descs characters.
 *
 * Return value:
 * 0 upon successful completion
 * A negative error number in the case of an error
 *
 * BUFR3_parse_descriptors must be called before this routine.
 *
 **************************************************************************/

int BUFR3_get_entry_texts(bufr3data *bufdes, char *text, int len, int ftn_style)
{
   int i, F, X, Y;
   tab_b_entry *tab_b_ptr;
   tab_d_entry *tab_d_ptr;
   char aux[4096];

   if(!bufdes->num_entry_descs) {
      strcpy(error_string,"BUFR3_parse_descriptors must be called before BUFR3_get_entry_texts");
      return -BUFR3_ERR_CALLSEQ;
   }

   /* Make sure that the actual tables are loaded */

   if(bufr3_read_tables(bufdes->sect1)<0) {
      sprintf(error_string,"ERROR reading BUFR tables: %s",bufr3_tables_get_error_string());
      return -BUFR3_ERR_TABLES;
   }

   /* Loop over the entry_descs */

   for(i=0;i<bufdes->num_entry_descs;i++)
   {
      F = bufdes->entry_descs[i].F;
      X = bufdes->entry_descs[i].X;
      Y = bufdes->entry_descs[i].Y;

      tab_b_ptr = bufr3_get_tab_b_entry(X,Y);

      switch(bufdes->entry_descs[i].type)
      {
         case ED_DATA: /* Simple data entries */
            my_strcpy(text+i*len,tab_b_ptr->name,len,ftn_style);
            break;

         case ED_ASSOC_FIELD: /* Associated field */
            my_strcpy(text+i*len,"Q-BITS FOR FOLLOWING VALUE",len,ftn_style);
            break;

         case ED_UNKNOWN: /* Unknown descriptor */
            my_strcpy(text+i*len,"Unknown Descriptor",len,ftn_style);
            break;

         case ED_CHARACTER: /* Character */
            sprintf(aux,"%3.3d CHARACTERS",Y);
            my_strcpy(text+i*len,aux,len,ftn_style);
            break;

         case ED_CHANGE_REF: /* Change reference value */
            if(tab_b_ptr) {
#ifdef CHGREF_COMPAT
               strcpy (aux,tab_b_ptr->name);
#else
               sprintf(aux,"Change reference value of %s",tab_b_ptr->mnem);
#endif
            } else {
               /* Changing the reference value of an undefined entry doesn't make much sense,
                  but we consider it not as an error */
               sprintf(aux,"Change reference value of 0%2.2d%3.3d",X,Y);
            }
            my_strcpy(text+i*len,aux,len,ftn_style);
            break;

         case ED_LOOP_START: /* Data replication/repetition */
            sprintf(aux,"Start of Loop - %d%2.2d%3.3d",F,X,Y);
            my_strcpy (text+i*len,aux,len,ftn_style);
            break;

         case ED_LOOP_END:
            my_strcpy (text+i*len,"End Loop",len,ftn_style);
            break;

         case ED_LOOP_COUNTER:
            my_strcpy(text+i*len,"Loop Counter",len,ftn_style);
            break;

         case ED_DATA_DESC_OP:
         case ED_QAINFO:
            /* put in default - may be overriden by more specific values */
            my_strcpy(text+i*len,"Unknown 2XXYYY data description operator",len,ftn_style);
            switch(X)
            {
              case 1:
                if(Y==0) {
                   my_strcpy(text+i*len,"Cancel change data width",len,ftn_style);
                } else {
                   sprintf(aux,"Change data width by %d bits",Y-128);
                   my_strcpy(text+i*len,aux,len,ftn_style);
                }
                break;
              case 2:
                if(Y==0) {
                   my_strcpy (text+i*len,"Cancel change scale",len,ftn_style);
                } else {
                   sprintf(aux,"Change scale by %d",Y-128);
                   my_strcpy(text+i*len,aux,len,ftn_style);
                }
                break;
              case 3:
                if(Y==255) {
                   my_strcpy(text+i*len,"End change reference value",len,ftn_style);
                } else if (Y==0) {
                   my_strcpy(text+i*len,"Revert to original reference values",len,ftn_style);
                } else {
                   my_strcpy(text+i*len,"Start change reference value",len,ftn_style);
                }
                break;
              case 4:
                if(Y==0) {
                   my_strcpy(text+i*len,"Cancel associated field",len,ftn_style);
                } else {
                   sprintf(aux,"Add associated field of %d bits",Y);
                   my_strcpy(text+i*len,aux,len,ftn_style);
                }
                break;
              case 6:
                sprintf(aux,"Length of local descriptor: %d bits",Y);
                my_strcpy(text+i*len,aux,len,ftn_style);
                break;
              case 21:
                my_strcpy(text+i*len,"Data not present indicator",len,ftn_style);
                break;
              case 22:
                if(Y==  0) my_strcpy(text+i*len,"Quality information follows",len,ftn_style);
                break;
              case 23:
                if(Y==255) my_strcpy(text+i*len,"Substituted values operator marker",len,ftn_style);
                if(Y==  0) my_strcpy(text+i*len,"Substituted values operator",len,ftn_style);
                break;
              case 24:
                if(Y==255) my_strcpy(text+i*len,"First order stat. val. marker operator",len,ftn_style);
                if(Y==  0) my_strcpy(text+i*len,"First order statistical values follow",len,ftn_style);
                break;
              case 25:
                if(Y==255) my_strcpy(text+i*len,"Difference stat. val. marker operator",len,ftn_style);
                if(Y==  0) my_strcpy(text+i*len,"Difference statistical values follow",len,ftn_style);
                break;
              case 32:
                if(Y==255) my_strcpy(text+i*len,"Replaced/retained value marker op.",len,ftn_style);
                if(Y==  0) my_strcpy(text+i*len,"Replaced/retained values follow",len,ftn_style);
                break;
              case 35:
                if(Y==  0) my_strcpy(text+i*len,"Cancel backward data reference",len,ftn_style);
                break;
              case 36:
                if(Y==  0) my_strcpy(text+i*len,"Define data present bit-map",len,ftn_style);
                break;
              case 37:
                if(Y==255) my_strcpy(text+i*len,"Cancel use defined data present bit-map",len,ftn_style);
                if(Y==  0) my_strcpy(text+i*len,"Use defined data present bit-map",len,ftn_style);
               break;
            }
            break;

         case ED_SEQ_START:
            tab_d_ptr = bufr3_get_tab_d_entry(X,Y);
            my_strcpy(text+i*len,"Sequence: ",len,ftn_style);
            if(len>10) my_strcpy(text+i*len+10,tab_d_ptr->text,len-10,ftn_style);
            break;

         case ED_SEQ_END:
            my_strcpy(text+i*len,"End Sequence",len,ftn_style);
            break;
      }
   }

   return 0;
}


/**************************************************************************
 *
 * BUFR3_get_entry_units:
 * Get units of the entries in entry_descs
 *
 * Input:
 * bufdes  pointer to a bufr3data object returned by BUFR3_decode_sections.
 * len     Length of the strings in array text.
 * ftn_style
 *         If set to 1, text strings are NOT zero terminated and padded
 *         with blanks up to len.
 *         If set to 0, text strings are zero terminated and have a maximum
 *         length of len-1.
 *
 * Output:
 * text    User supplied array for receiving the unit strings.
 *         Must have a total length of len*bufdes->num_entry_descs characters.
 *
 * Return value:
 * 0 upon successful completion
 * A negative error number in the case of an error
 *
 * BUFR3_parse_descriptors must be called before this routine.
 *
 **************************************************************************/

int BUFR3_get_entry_units(bufr3data *bufdes, char *text, int len, int ftn_style)
{
   int i, X, Y;
   tab_b_entry *tab_b_ptr;

   if(!bufdes->num_entry_descs) {
      strcpy(error_string,"BUFR3_parse_descriptors must be called before BUFR3_get_entry_units");
      return -BUFR3_ERR_CALLSEQ;
   }

   /* Make sure that the actual tables are loaded */

   if(bufr3_read_tables(bufdes->sect1)<0) {
      sprintf(error_string,"ERROR reading BUFR tables: %s",bufr3_tables_get_error_string());
      return -BUFR3_ERR_TABLES;
   }

   /* Loop over the entry_descs */

   for(i=0;i<bufdes->num_entry_descs;i++)
   {
      X = bufdes->entry_descs[i].X;
      Y = bufdes->entry_descs[i].Y;

      tab_b_ptr = bufr3_get_tab_b_entry(X,Y);

      switch(bufdes->entry_descs[i].type)
      {
         case ED_DATA: /* Simple data entries */
            my_strcpy(text+i*len,tab_b_ptr->unit,len,ftn_style);
            break;

         case ED_ASSOC_FIELD: /* Associated field */
            my_strcpy(text+i*len,"CODE TABLE",len,ftn_style);
            break;

         case ED_CHARACTER: /* Character */
            my_strcpy(text+i*len,"CCITT IA5",len,ftn_style);
            break;

         case ED_CHANGE_REF: /* Change reference value */
            if(tab_b_ptr) my_strcpy(text+i*len,tab_b_ptr->unit,len,ftn_style);
            break;

         default: /* Empty string */
            if(ftn_style)
               memset(text+i*len,' ',len);
            else
               text[i*len] = 0;
      }
   }

   return 0;
}



/**************************************************************************
 *
 * BUFR3_decode_data: Decodes the data in a BUFR record
 *
 * Input: pointer to a bufr3data object returned by BUFR3_decode_sections.
 *
 * Output: Only via the bufr3data object.
 *
 * Return value:
 * 0 upon successful completion
 * A negative error number in the case of an error
 *
 * BUFR3_parse_descriptors must be called before!
 *
 * NB: The data given to BUFR3_decode_sections must still be alive
 *     when BUFR3_decode_data is called since only a pointer to this
 *     data is kept and no copy is made for performance reasons.
 *
 **************************************************************************/

int BUFR3_decode_data(bufr3data *bufdes)
{
   int nrep, res;

   if(!bufdes->num_entry_descs) {
      strcpy(error_string,"BUFR3_parse_descriptors must be called before BUFR3_get_entry_texts");
      return -BUFR3_ERR_CALLSEQ;
   }

   bit_position = 0;

   /* allocate the result arrays */

   bufdes->results = (bufr_entry **) malloc(bufdes->num_subsets*sizeof(bufr_entry *));
   bufdes->num_results = (int *) malloc(bufdes->num_subsets*sizeof(int));
   bufdes->nli_results = (int *) malloc(bufdes->num_subsets*sizeof(int));
   bufdes->max_results = (int *) malloc(bufdes->num_subsets*sizeof(int));
   if(!bufdes->results || !bufdes->num_results || !bufdes->nli_results || !bufdes->max_results) {
      strcpy(error_string,"malloc failed");
      if(bufdes->results) free(bufdes->results); bufdes->results = 0;
      if(bufdes->num_results) free(bufdes->num_results); bufdes->num_results = 0;
      if(bufdes->nli_results) free(bufdes->nli_results); bufdes->nli_results = 0;
      if(bufdes->max_results) free(bufdes->max_results); bufdes->max_results = 0;
      return -BUFR3_ERR_MALLOC;
   }
   memset(bufdes->results,0,bufdes->num_subsets*sizeof(bufr_entry *));
   memset(bufdes->num_results,0,bufdes->num_subsets*sizeof(int));
   memset(bufdes->nli_results,0,bufdes->num_subsets*sizeof(int));
   memset(bufdes->max_results,0,bufdes->num_subsets*sizeof(int));


   if(bufdes->is_compressed)
      nrep = 1;
   else
      nrep = bufdes->num_subsets;

   bufdes->max_num_results = 0;
   bufdes->max_nli_results = 0;

   for(nsubset=0;nsubset<nrep;nsubset++)
   {
#ifdef DEBUG
      printf("------------------ start of subset %d\n",nsubset);
      fflush(stdout);
#endif
      QAInfo_mode=0;
      num_QA_entries=0;
      num_QA_edescs=0;
      QA_def_bitmap=0;
      num_QA_present=0;
      num_QA_dptotal=0;

      res = decode_bufr(bufdes, 0);
      if(res) return res;
      if(bufdes->num_results[nsubset] > bufdes->max_num_results)
         bufdes->max_num_results = bufdes->num_results[nsubset];
      if(bufdes->nli_results[nsubset] > bufdes->max_nli_results)
         bufdes->max_nli_results = bufdes->nli_results[nsubset];
   }

   bufdes->bits_used = bit_position;

   /* Clean up (not absolutly necessary - so it may be omitted in error case) */

   if(max_QA_edescs) {
      free(QA_edescs);
      QA_edescs = 0;
      max_QA_edescs = 0;
   }
   if(max_QA_present) {
      free(QA_present);
      QA_present = 0;
      max_QA_present = 0;
   }

   return 0;
}

/**************************************************************************
 *
 * BUFR3_encode: Encode BUFR3 data
 *
 * iednr:
 *    Edition number, should normally be 3
 *
 * sect1, sect2, sect3:
 *    Completly filled sections 1, 2 and 3 including lengths
 *    in bytes 1-3 and all other information necessary to
 *    create the BUFR (like the flag if the BUFR is compressed or
 *    the descriptors in section 3).
 *    sect2 may be a NULL pointer if it is not present as indicated
 *    in section 1
 *
 * entries:
 *    Pointer to an array of pointers to the entries going into
 *    the BUFR data. Must be integer values which are already
 *    scaled but still have the reference value added.
 *    Undefined values have to be set to BUFR3_UNDEF_VALUE.
 *
 * centries:
 *    like entries, but contains pointers to character strings
 *    for entries which are character values.
 *    Undefined character values must have a BUFR3_UNDEF_VALUE
 *    in the entries array, other the values in the entries
 *    array don't matter.
 *    The character strings need not be 0 terminated (since the
 *    length is known) and a 0 byte is treated like every other byte.
 *
 * num_entries:
 *    Array with the length of each subset in entries.
 *
 * set_desc_idx:
 *    Flag if the desc_idx array should be allocated and set during encoding
 *
 * pbufdes:
 *    receives a pointer to a BUFR3 data object or NULL in case of an error
 *
 * Return value:
 * 0 upon successful completion
 * A negative error number in the case of an error
 *
 **************************************************************************/

int BUFR3_encode
   (int iednr, unsigned char *sect1, unsigned char *sect2, unsigned char *sect3,
    int **entries, unsigned char ***centries, int *num_entries, int set_desc_idx,
    bufr3data **pbufdes)
{
   bufr3data *bufdes;
   int nrep, l, res;
   unsigned char *s;

   *pbufdes = 0;

   /* Allocate the buffer structure, fill with zeros */

   bufdes = (bufr3data *) malloc(sizeof(bufr3data));
   if(!bufdes) {
      strcpy(error_string,"malloc failed");
      return -BUFR3_ERR_MALLOC;
   }

   memset(bufdes,0,sizeof(bufr3data));

   /* Get length of sections 1, 2 and 3 */

   bufdes->sect1_length = sect1[0]*256*256 + sect1[1]*256 + sect1[2];
   bufdes->sect3_length = sect3[0]*256*256 + sect3[1]*256 + sect3[2];
   if( (sect1[7]&0x80) == 0x80 ) {
      bufdes->sect2_length = sect2[0]*256*256 + sect2[1]*256 + sect2[2];
   } else {
      bufdes->sect2_length = 0;
   }

   bufdes->iednr = iednr;

   /* Safety chechks */

   if(bufdes->sect1_length<18) {
      sprintf(error_string,"Section 1 length: %d (must be at least 18)",bufdes->sect1_length);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }
   if(bufdes->sect3_length<8) {
      sprintf(error_string,"Section 3 length: %d (must be at least 8)",bufdes->sect3_length);
      free(bufdes);
      return -BUFR3_ERR_SECTIONS;
   }

   /* The length of every section must be a even number of bytes */

#ifdef PRINT_WARNINGS
   if(bufdes->sect1_length & 1) {
      fprintf(stderr,"WARNING: Section 1 length: %d not even\n",bufdes->sect1_length);
   }
   if(bufdes->sect2_length & 1) {
      fprintf(stderr,"WARNING: Section 2 length: %d not even\n",bufdes->sect2_length);
   }
   if(bufdes->sect3_length & 1) {
      fprintf(stderr,"WARNING: Section 3 length: %d not even\n",bufdes->sect3_length);
   }
#endif

   /* Offset to binary data */

   bufdes->bin_offset = 8 + bufdes->sect1_length + bufdes->sect2_length + bufdes->sect3_length + 4;

   /* Allocate the data the first time (it may be extented during the
      fill process), set section pointers, copy in sections, set some other
      values in bufdes */

   bufdes->totalbytes = bufdes->bin_offset + 16384 + 2 + 4;
   /* 2 bytes to have space for padding, 4 Bytes for section 5 */
   bufdes->bufr_data  = (unsigned char *) malloc(bufdes->totalbytes);
   if(!bufdes->bufr_data) {
      free(bufdes);
      strcpy(error_string,"malloc failed");
      return -BUFR3_ERR_MALLOC;
   }
   memset(bufdes->bufr_data,0,bufdes->totalbytes);

   bufdes->sect1 = bufdes->bufr_data+8; /* Section 1 starts at offset 8 */
   memcpy(bufdes->sect1,sect1,bufdes->sect1_length);

   if(bufdes->sect2_length)
   {
      bufdes->sect2 = bufdes->sect1 + bufdes->sect1_length;
      memcpy(bufdes->sect2,sect2,bufdes->sect2_length);
      bufdes->sect3 = bufdes->sect2 + bufdes->sect2_length;
   }
   else
   {
      bufdes->sect2 = 0;
      bufdes->sect3 = bufdes->sect1 + bufdes->sect1_length;
   }

   memcpy(bufdes->sect3,sect3,bufdes->sect3_length);

   bufdes->sect4 = bufdes->sect3 + bufdes->sect3_length;

   /* Additional variables */

   bufdes->num_subsets   = bufdes->sect3[4]*256 + bufdes->sect3[5];
   bufdes->is_observed   = (bufdes->sect3[6]&0x80) == 0x80;
   bufdes->is_compressed = (bufdes->sect3[6]&0x40) == 0x40;
   bufdes->descriptors   = bufdes->sect3 + 7;
   bufdes->num_descriptors = (bufdes->sect3_length-7)/2;

   bufdes->entries = entries;
   bufdes->centries = centries;
   bufdes->num_entries = num_entries;

   /* Allocate desc_idx if wanted */

   if(set_desc_idx)
   {
      bufdes->desc_idx = (int **) malloc(bufdes->num_subsets*sizeof(int *));
      if(!bufdes->desc_idx) {
         free(bufdes->bufr_data);
         free(bufdes);
         strcpy(error_string,"malloc failed");
         return -BUFR3_ERR_MALLOC;
      }
      memset(bufdes->desc_idx,0,bufdes->num_subsets*sizeof(int *));
      for(nsubset=0;nsubset<bufdes->num_subsets;nsubset++)
      {
         bufdes->desc_idx[nsubset] = (int *) malloc(num_entries[nsubset]*sizeof(int));
         if(!bufdes->desc_idx[nsubset]) {
            free(bufdes->bufr_data);
            BUFR3_destroy(bufdes); /* destroys all previously allocated memory */
            strcpy(error_string,"malloc failed");
            return -BUFR3_ERR_MALLOC;
         }
      }
   }


   /* Parse the descriptors exactly as in the case when reading the BUFR */

   res = BUFR3_parse_descriptors(bufdes);
   if(res) {
      free(bufdes->bufr_data);
      BUFR3_destroy(bufdes);
      return res;
   }

   /* Encode the data */

   bit_position = 0;

   if(bufdes->is_compressed)
      nrep = 1;
   else
      nrep = bufdes->num_subsets;

   for(nsubset=0;nsubset<nrep;nsubset++)
   {
#ifdef DEBUG
      printf("------------------ start of subset %d\n",nsubset);
      fflush(stdout);
#endif
      QAInfo_mode=0;
      num_QA_entries=0;
      num_QA_edescs=0;
      QA_def_bitmap=0;
      num_QA_present=0;
      num_QA_dptotal=0;

      nentry = 0;
      res = encode_bufr(bufdes, 0);
      if(res) {
         free(bufdes->bufr_data);
         BUFR3_destroy(bufdes);
         return res;
      }

      /* check if the correct number of entries has been encoded */

      if(bufdes->num_entries[nsubset] != nentry) {
         sprintf(error_string,"Number of entries encoded=%d not equal number of entries=%d for subset %d",
                  nentry,bufdes->num_entries[nsubset],nsubset);
         free(bufdes->bufr_data);
         BUFR3_destroy(bufdes);
         return -BUFR3_ERR_NUM_ENTRIES;
      }
   }

   /* Check the correct number of entries for the other subsets if BUFR is compressed */

   if(bufdes->is_compressed) {
      for(nsubset=1;nsubset<bufdes->num_subsets;nsubset++) {
         if(bufdes->num_entries[nsubset] != nentry) {
            sprintf(error_string,"Number of entries encoded=%d not equal number of entries=%d for subset %d",
                     nentry,bufdes->num_entries[nsubset],nsubset);
            free(bufdes->bufr_data);
            BUFR3_destroy(bufdes);
            return -BUFR3_ERR_NUM_ENTRIES;
         }
      }
   }

   /* Clean up (not absolutly necessary - so it may be omitted in error case) */

   if(max_QA_edescs) {
      free(QA_edescs);
      QA_edescs = 0;
      max_QA_edescs = 0;
   }
   if(max_QA_present) {
      free(QA_present);
      QA_present = 0;
      max_QA_present = 0;
   }

   bufdes->bits_used = bit_position;

   /* The section pointers are not longer valid since the position of
      bufr_data may have been moved (due to realloc).
      We have to set them again */

   bufdes->sect1 = bufdes->bufr_data+8; /* Section 1 starts at offset 8 */
   if(bufdes->sect2_length)
   {
      bufdes->sect2 = bufdes->sect1 + bufdes->sect1_length;
      bufdes->sect3 = bufdes->sect2 + bufdes->sect2_length;
   }
   else
   {
      bufdes->sect2 = 0;
      bufdes->sect3 = bufdes->sect1 + bufdes->sect1_length;
   }
   bufdes->sect4 = bufdes->sect3 + bufdes->sect3_length;
   bufdes->descriptors  = bufdes->sect3 + 7;


   /* Get length of binary data, length of section 4 */

   l = (bufdes->bits_used+15)/16; /* length in words of 2 bytes */
   bufdes->bin_length = l*2;


   bufdes->sect4_length = bufdes->bin_length+4;

   /* Add length to start of section 4 */

   bufdes->sect4[0] = (bufdes->sect4_length>>16)&0xff;
   bufdes->sect4[1] = (bufdes->sect4_length>> 8)&0xff;
   bufdes->sect4[2] = (bufdes->sect4_length    )&0xff;

   /* Get total length of BUFR, add sections 0 and 5 */

   bufdes->nbytes = bufdes->sect1_length + bufdes->sect2_length +
                    bufdes->sect3_length + bufdes->sect4_length + 12;

   s = bufdes->bufr_data;

   s[0] = 'B';
   s[1] = 'U';
   s[2] = 'F';
   s[3] = 'R';
   s[4] = ((bufdes->nbytes)>>16)&0xff;
   s[5] = ((bufdes->nbytes)>> 8)&0xff;
   s[6] = ((bufdes->nbytes)    )&0xff;
   s[7] = iednr; /* Edition number */

   bufdes->sect5 = bufdes->sect4 + bufdes->sect4_length;
   s = bufdes->sect5;
   s[0] = '7';
   s[1] = '7';
   s[2] = '7';
   s[3] = '7';

   *pbufdes = bufdes;
   return 0;
}

/**************************************************************************
 *
 * BUFR3_destroy: Destroy a bufr3data object
 *
 * Input: pointer to a bufr3data object returned by BUFR3_decode_sections.
 *
 * bufdes->bufr_data is NOT free'd by this routine!
 *
 **************************************************************************/

void BUFR3_destroy(bufr3data *bufdes)
{
   int i;

   if(bufdes->entry_descs) free(bufdes->entry_descs);
   if(bufdes->char_data) free(bufdes->char_data);
   if(bufdes->results) {
      for (i=0;i<bufdes->num_subsets;i++)
         if(bufdes->results[i]) free(bufdes->results[i]);
      free(bufdes->results);
   }
   if(bufdes->num_results) free(bufdes->num_results);
   if(bufdes->nli_results) free(bufdes->nli_results);
   if(bufdes->max_results) free(bufdes->max_results);
   if(bufdes->desc_idx) {
      for (i=0;i<bufdes->num_subsets;i++)
         if(bufdes->desc_idx[i]) free(bufdes->desc_idx[i]);
      free(bufdes->desc_idx);
   }
   free(bufdes);
}

/**************************************************************************
 *
 * The following are auxiliary routines for writing the BUFR data.
 * The file must be opened before writing with the C-stdio-routine "fopen"
 *
 * Note that there is NO BUFR3_close_raw since a simple fclose will do it
 *
 **************************************************************************/

int BUFR3_write_raw(FILE *fd, bufr3data *bufdes)
{
   int nb;

   nb = fwrite(bufdes->bufr_data,1,bufdes->nbytes,fd);
   if(nb != bufdes->nbytes) {
      strcpy(error_string,"BUFR3_write_raw: Error writing BUFR data");
      return -BUFR3_ERR_WRITE;
   }

   return 0;
}

int BUFR3_write_dwd(FILE *fd, bufr3data *bufdes)
{
   unsigned char c[4];
   int nb;

   /* If we are at the beginning of the file, write 4 zero bytes */

   if(ftell(fd) == 0) {
      c[0] = c[1] = c[2] = c[3] = 0;
      nb=fwrite(c,1,4,fd);
      if(nb != 4) goto WriteErr;
   }

   /* 4 header Bytes with the record length (BIG Endian) */

   c[0] = (bufdes->nbytes>>24) & 0xff;
   c[1] = (bufdes->nbytes>>16) & 0xff;
   c[2] = (bufdes->nbytes>> 8) & 0xff;
   c[3] = (bufdes->nbytes    ) & 0xff;

   nb=fwrite(c,1,4,fd);
   if(nb != 4) goto WriteErr;

   /* BUFR data */

   nb = fwrite(bufdes->bufr_data,1,bufdes->nbytes,fd);
   if(nb != bufdes->nbytes) goto WriteErr;

   /* 4 trailing bytes */

   nb=fwrite(c,1,4,fd);
   if(nb != 4) goto WriteErr;

   return 0;

WriteErr:
   strcpy(error_string,"BUFR3_write_dwd: Error writing BUFR data");
   return -BUFR3_ERR_WRITE;
}

int BUFR3_close_dwd(FILE *fd)
{
   unsigned char c[4];

   /* Write 4 trailing 0 Bytes and close the file,
      we dont check for errors here */

   c[0] = c[1] = c[2] = c[3] = 0;
   if(fwrite(c,1,4,fd)!=4) {
      strcpy(error_string,"BUFR3_close_dwd: Error writing trailor");
      return -BUFR3_ERR_WRITE;
   }

   fclose(fd);

   return 0;
}

/**************************************************************************
 *
 * BUFR3_get_error_string:
 *    returns a pointer to a string with the most recent error
 *
 * This is a pointer to a static storage area, the user should not
 * make an attempt to free() it
 *
 **************************************************************************/

char *BUFR3_get_error_string(void)
{
   return error_string;
}
