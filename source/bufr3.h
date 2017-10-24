/**************************************************************************

   bufr3.h - Include file for BUFR3 library for DWD

   Version: 1.0

   Author: Dr. Rainer Johanni, Mar. 2003

 **************************************************************************/

#include "bufr3_tables.h"                           /* martin, org: <bufr3_tables.h> */

#ifndef BUFR3_H
#define BUFR3_H


typedef enum _entry_desc_type {

/* The following types add entries to the result data
   (originating from BUFR data) */

   ED_DATA,               /* Data from F==0 descriptor */
   ED_ASSOC_FIELD,        /* Associated field ("Quality bits") */
   ED_UNKNOWN,            /* Unknown data from F==0 descriptor */
   ED_CHARACTER,          /* Character data from 205YYY descriptor */
   ED_CHANGE_REF,         /* Change reference value */
   ED_QAINFO,             /* Quality Assessment information */

/* The following types also add entries to the result data,
   this is meta data (not from BUFR data) */

   ED_LOOP_START,         /* Start of Loop, 1XXYYY descriptor */
   ED_LOOP_END,           /* Loop end marker */
   ED_LOOP_COUNTER,       /* Loop counter */

/* The following types never add entries to the result data */

   ED_DATA_DESC_OP,       /* F==2, X!=5 and no QA Info */

/* ED_SEQ_START and ED_SEQ_END also add entries */

   ED_SEQ_START,          /* F==3, 3XXYYY descriptor */
   ED_SEQ_END             /* Sequence end marker */
} entry_desc_type;

typedef struct _bufr_entry_desc
{
   entry_desc_type type;         /* type of entry descriptor, see above */
   int F;                        /* Originating F value */
   int X;                        /* Originating X value */
   int Y;                        /* Originating Y value */
   int is_char;                  /* Flag if data is character */
   int scale;                    /* Scale */
   double dscale;                /* 10 ** (-scale) */
   int bits;                     /* Number of bits this entry consumes in buffer */
   int refval;                   /* Reference value (this entry may be changed during de/encoding) */
   char mnem[BUFR3_MNEM_LEN+1];  /* Mnemnonic */
   int loopend;                  /* index of end of loop for a ED_LOOP_START entry */
   int omit;                     /* User settable flag if this entry should be
                                    omitted in the result */
} bufr_entry_desc;

typedef struct _bufr_entry {
   int data;                     /* Integer data or offset into char_data array */
   int desc;                     /* index in entry_descs array */
} bufr_entry;

typedef struct _bufr3data
{
   unsigned char *bufr_data;     /* data of whole bufr record */
   unsigned char *sect1;         /* pointer to section 1 data */
   unsigned char *sect2;         /* pointer to section 2 data */
                                 /* sect2 == 0 if section 2 not present */
   unsigned char *sect3;         /* pointer to section 3 data */
   unsigned char *sect4;         /* pointer to section 4 data */
   unsigned char *sect5;         /* pointer to section 5 data */

   int iednr;                    /* edition number (should be 3) */

   int nbytes;                   /* length of whole bufr record */
   int sect1_length;             /* length of section 1 */
   int sect2_length;             /* length of section 2 */
   int sect3_length;             /* length of section 3 */
   int sect4_length;             /* length of section 4 */

   int num_subsets;              /* Number of subsets */
   int is_observed;              /* Flag if this is observed data */
   int is_compressed;            /* Flag if this is compressed data */
   unsigned char *descriptors;   /* pointer to descriptors in section 3 */
   int num_descriptors;          /* number of descriptors */

   int bin_offset;               /* offset to binary data from start of bufr_data in bytes */
   int bin_length;               /* length of binary data */

   int bits_used;                /* bits consumed during BUFR decompression
                                    or produced during encoding */

   int ftn_options;              /* Variable to store options from FORTRAN interface */

   int has_QAInfo;               /* flag if BUFR has Quality Assessment info */

   bufr_entry_desc *entry_descs; /* pointer to array of expanded descriptors */
   int num_entry_descs;          /* number of expanded descriptors */
   int max_entry_descs;          /* allocated size of entry_descs */

   /* The following members are set only during decoding */

   unsigned char *char_data;     /* pointer to character data array */
   int char_data_len;            /* real length of char_data */
   int char_data_used;           /* number of characters used in char_data */

   bufr_entry **results;         /* decoded results (2 D array) */
   int *num_results;             /* number of results stored (1 D array) */
   int *nli_results;             /* number of results not counting the loop info entries */
   int *max_results;             /* allocated length (1 D array) */
   int max_num_results;          /* Max of num_results over all subsets */
   int max_nli_results;          /* Max of nli_results over all subsets */

   /* The following members are set only during encoding */

   int **desc_idx;               /* 2 D array for returning the index into entry_descs
                                    of the encoded entries (mainly for debug purposes
                                    and for use in the FORTRAN interface).
                                    This is ONLY allocated if requested, otherways
                                    it is set to a NULL pointer */

   /* Auxiliary data for encoding */

   int **entries;                /* 2 D array of entries for encoding */
   unsigned char ***centries;    /* 2 D array of pointers to entries which are chars */
   int *num_entries;             /* number of entries per subset */

   int totalbytes;               /* total number of bytes allocated for bufr_data */

} bufr3data;

/* Error return values */

#define BUFR3_ERR_MALLOC 1         /* malloc failed */
#define BUFR3_ERR_DESCRIPTORS 2    /* error/inconsistency in the descriptors */
#define BUFR3_ERR_DATAWIDTH 3      /* width of a data item > 32 bits */
#define BUFR3_ERR_DATARANGE 4      /* data item to be encoded < refval or bigger than bit width */
#define BUFR3_ERR_EXHAUSTED 5      /* too few bits in BUFR during decoding */
#define BUFR3_ERR_NUM_ENTRIES 6    /* The number of entries (given by the user) during
                                      encoding is not the number of entries required */
#define BUFR3_ERR_COMPRESSED 7     /* Common data (like replication factors) is different
                                      for 2 subsets of a compressed BUFR */
#define BUFR3_ERR_NO_BUFR 8        /* String BUFR not found in input file/data */
#define BUFR3_ERR_SECTIONS 9       /* Error in section data (inconsistent lengths) */
#define BUFR3_ERR_CALLSEQ 10       /* Calling sequence illegal */
#define BUFR3_ERR_TABLES 11        /* Error reading tables B or D */
#define BUFR3_ERR_WRITE 12         /* Error writing BUFR */
#define BUFR3_ERR_QAINFO 13        /* Error with Quality Assessment Info */
#define BUFR3_ERR_READ 14          /* Error reading BUFR */
#define BUFR3_ERR_HANDLE_UNIT 20   /* Illegal handle or unit in FORTRAN interface */
#define BUFR3_ERR_OPEN 21          /* Error opening file */
#define BUFR3_ERR_DIMENSION 22     /* Dimension of a parameter too small */
#define BUFR3_ERR_DELETE 23        /* Error deleting file */
#define BUFR3_ERR_INTERNAL 99      /* Internal error */

/* Prototype declarations of user interface routines */

int BUFR3_read_record(FILE *fd, int max_chars, unsigned char **pdata);
int BUFR3_decode_sections(unsigned char *data, bufr3data **pbufdes);
int BUFR3_parse_descriptors(bufr3data *bufdes);
int BUFR3_get_entry_texts(bufr3data *bufdes, char *text, int len, int ftn_style);
int BUFR3_get_entry_units(bufr3data *bufdes, char *text, int len, int ftn_style);
int BUFR3_decode_data(bufr3data *bufdes);
int BUFR3_encode
   (int iednr, unsigned char *sect1, unsigned char *sect2, unsigned char *sect3,
    int **entries, unsigned char ***centries, int *num_entries, int set_desc_idx,
    bufr3data **pbufdes);
void BUFR3_destroy(bufr3data *bufdes);

int BUFR3_write_raw(FILE *fd, bufr3data *bufdes);
int BUFR3_write_dwd(FILE *fd, bufr3data *bufdes);
int BUFR3_close_dwd(FILE *fd);

char *BUFR3_get_error_string();

/* Prototype declarations of help routines */

void my_strcpy(char *dst, char *src, int num, int ftn_style);

/* The undefined value */

#define BUFR3_UNDEF_VALUE (-1000000000)

/* A set of macros for accessing BUFR data */

#define BUFR3_DATA(bufdes,i,j)     ((bufdes)->results[i][j].data)
#define BUFR3_DESC(bufdes,i,j)     ((bufdes)->entry_descs[(bufdes)->results[i][j].desc])
#define BUFR3_IS_CHAR(bufdes,i,j)  (BUFR3_DESC(bufdes,i,j).is_char)

#define BUFR3_CHAR_PTR(bufdes,i,j) \
(((bufdes)->results[i][j].data==BUFR3_UNDEF_VALUE)?\
(unsigned char *)"":((bufdes)->char_data+(bufdes)->results[i][j].data))

#define BUFR3_DOUBLE_DATA(bufdes,i,j) \
(((bufdes)->results[i][j].data==BUFR3_UNDEF_VALUE)?\
(double)BUFR3_UNDEF_VALUE:BUFR3_DATA(bufdes,i,j)*BUFR3_DESC(bufdes,i,j).dscale)


/* FORTRAN interface */

/* Options */

#define FTN_OMIT_LOOP_INFO   1
#define FTN_OMIT_SEQ_INFO    2
#define FTN_RESORT_RESULTS   4
#define FTN_REMOVE_LOOP_INFO 0x10000 /* used only internally */

/*
   FTN_INT must correspond to the FORTRAN default INTEGER type
   FTN_REAL same for REAL
*/

#define FTN_INT int
#define FTN_REAL float

#ifdef _AIX
#define bufr3_open_file_           bufr3_open_file
#define bufr3_close_file_          bufr3_close_file
#define bufr3_read_bufr_           bufr3_read_bufr
#define bufr3_write_bufr_          bufr3_write_bufr
#define bufr3_get_num_bytes_       bufr3_get_num_bytes
#define bufr3_get_num_subsets_     bufr3_get_num_subsets
#define bufr3_get_num_descriptors_ bufr3_get_num_descriptors
#define bufr3_get_section_length_  bufr3_get_section_length
#define bufr3_get_section_         bufr3_get_section
#define bufr3_get_sections_        bufr3_get_sections
#define bufr3_get_descriptors_     bufr3_get_descriptors
#define bufr3_get_header_text_     bufr3_get_header_text
#define bufr3_get_bufr_            bufr3_get_bufr
#define bufr3_put_bufr_            bufr3_put_bufr
#define bufr3_destroy_             bufr3_destroy
#define bufr3_decode_              bufr3_decode
#define bufr3_get_data_old_        bufr3_get_data_old
#define bufr3_get_data_            bufr3_get_data
#define bufr3_get_data_ir_         bufr3_get_data_ir
#define bufr3_get_entry_descs_     bufr3_get_entry_descs
#define bufr3_get_entry_texts_     bufr3_get_entry_texts
#define bufr3_get_entry_units_     bufr3_get_entry_units
#define bufr3_get_character_value_ bufr3_get_character_value
#define bufr3_encode_              bufr3_encode
#define bufr3_get_error_message_   bufr3_get_error_message
#endif

/* Prototypes of FORTRAN interface functions */

void bufr3_open_file_
   (char *filename, FTN_INT *output, FTN_INT *raw, FTN_INT *iunit, FTN_INT *ier,
    unsigned long l_filename);

void bufr3_close_file_
   (FTN_INT *iunit, char *status, FTN_INT *ier, unsigned long l_status);

void bufr3_read_bufr_
   (FTN_INT *iunit, FTN_INT *max_chars, FTN_INT *ihandle, FTN_INT *ieof, FTN_INT *ier);

void bufr3_write_bufr_
   (FTN_INT *iunit, FTN_INT *ihandle, FTN_INT *ier);

void bufr3_get_num_bytes_
   (FTN_INT *ihandle, FTN_INT *num);

void bufr3_get_num_subsets_
   (FTN_INT *ihandle, FTN_INT *num);

void bufr3_get_num_descriptors_
   (FTN_INT *ihandle, FTN_INT *num);

void bufr3_get_section_length_
   (FTN_INT *ihandle, FTN_INT *isect, FTN_INT *num);

void bufr3_get_section_
   (FTN_INT *ihandle, FTN_INT *isect, FTN_INT *ibufsec, FTN_INT *ier);

void bufr3_get_sections_
   (FTN_INT *ihandle, FTN_INT *ibufsec0, FTN_INT *ibufsec1, FTN_INT *ibufsec2,
    FTN_INT *ibufsec3, FTN_INT *ibufsec4, FTN_INT *ier);

void bufr3_get_descriptors_
   (FTN_INT *ihandle, FTN_INT *idescr, FTN_INT *ier);

void bufr3_get_header_text_
   (FTN_INT *ihandle, char *text, FTN_INT *ier, unsigned long l_text);

void bufr3_get_bufr_
   (FTN_INT *ihandle, FTN_INT *iswap, unsigned char *ibufr, FTN_INT *ier);

void bufr3_put_bufr_
   (unsigned char *ibufr, FTN_INT *ihandle, FTN_INT *ier);

void bufr3_destroy_
   (FTN_INT *ihandle, FTN_INT *ier);

void bufr3_decode_
   (FTN_INT *ihandle, char *mnem_list, FTN_INT *mnem_list_len,
    FTN_INT *idesc_list, FTN_INT *idesc_list_len, FTN_INT *ioptions,
    FTN_INT *max_subset_len, FTN_INT *num_entry_descs, FTN_INT *ier, unsigned long l_mnem_list);

void bufr3_get_data_old_
   (FTN_INT *ihandle, char *ytex1, FTN_INT *ibufdat, FTN_REAL *fbufdat,
    char *ybufdat, FTN_INT *nbufdat, FTN_INT *dim1, FTN_INT *dim2, FTN_INT *ier,
    unsigned long l_ytex1, unsigned long l_ybufdat);

void bufr3_get_data_
   (FTN_INT *ihandle, FTN_INT *ibufdat, FTN_INT *idescidx,
    FTN_INT *nbufdat, FTN_INT *dim1, FTN_INT *dim2, FTN_INT *ier);

void bufr3_get_data_ir_
   (FTN_INT *ihandle, FTN_INT *ibufdat, FTN_REAL *fbufdat, FTN_INT *idescidx,
    FTN_INT *nbufdat, FTN_INT *dim1, FTN_INT *dim2, FTN_INT *ier);

void bufr3_get_entry_descs_
   (FTN_INT *ihandle, FTN_INT *itype, FTN_INT *ifxy, FTN_INT *is_char,
    FTN_INT *iscale, FTN_REAL *scale, FTN_INT *nbits, FTN_INT *irefval,
    char *ymnem, FTN_INT *ier, unsigned long l_ymnem);

void bufr3_get_entry_texts_
   (FTN_INT *ihandle, char *ytext, FTN_INT *ier, unsigned long l_ytext);

void bufr3_get_entry_units_
   (FTN_INT *ihandle, char *yunit, FTN_INT *ier, unsigned long l_yunit);

void bufr3_get_character_value_
   (FTN_INT *ihandle, FTN_INT *ibufdat, char *s, FTN_INT *ier, unsigned long l_s);

void bufr3_encode_
   (FTN_INT *ibufsec0, FTN_INT *ibufsec1, FTN_INT *ibufsec2, FTN_INT *ibufsec3,
    FTN_INT *ibufsec4, FTN_INT *idescr, FTN_INT *nbufdat, FTN_INT *ibufdat,
    char *ybufdat, FTN_INT *setmne, char *ybufmne, FTN_INT *ihandle, FTN_INT *ier,
    unsigned long l_ybufdat, unsigned long l_ybufmne);

void bufr3_get_error_message_(char *s, unsigned long l_s);

#endif
