#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RMFF_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RMFF_H_

//#define UINT32 unsigned int
//#define UINT16 unsigned short 
//#define UINT8 unsigned char

void *xbuffer_init(int chunk_size);
void *xbuffer_free(void *buf);
void *xbuffer_copyin(void *buf, int index, const void *data, int len);
void *xbuffer_ensure_size(void *buf, int size);
void *xbuffer_strcat(void *buf, char *data);

#define RMFF_HEADER_SIZE 0x12

#define FOURCC_TAG( ch0, ch1, ch2, ch3 ) \
	(((long)(unsigned char)(ch3)       ) | \
	( (long)(unsigned char)(ch2) << 8  ) | \
	( (long)(unsigned char)(ch1) << 16 ) | \
	( (long)(unsigned char)(ch0) << 24 ) )

#define FF_BE_16(x)  ((((UINT8*)(x))[0] << 8) | ((UINT8*)(x))[1])
#define FF_BE_32(x)  ((((UINT8*)(x))[0] << 24) | \
	(((UINT8*)(x))[1] << 16) | \
	(((UINT8*)(x))[2] << 8) | \
	((UINT8*)(x))[3])


#define RMF_TAG   FOURCC_TAG('.', 'R', 'M', 'F')
#define PROP_TAG  FOURCC_TAG('P', 'R', 'O', 'P')
#define MDPR_TAG  FOURCC_TAG('M', 'D', 'P', 'R')
#define CONT_TAG  FOURCC_TAG('C', 'O', 'N', 'T')
#define DATA_TAG  FOURCC_TAG('D', 'A', 'T', 'A')
#define INDX_TAG  FOURCC_TAG('I', 'N', 'D', 'X')
#define PNA_TAG   FOURCC_TAG('P', 'N', 'A',  0 )

#define MLTI_TAG  FOURCC_TAG('M', 'L', 'T', 'I')

/* prop flags */
#define PN_SAVE_ENABLED         0x01
#define PN_PERFECT_PLAY_ENABLED 0x02
#define PN_LIVE_BROADCAST       0x04

/*
* rm header data structs
*/

typedef struct {

	UINT32 object_id;
	UINT32 size;
	UINT16 object_version;

	UINT32 file_version;
	UINT32 num_headers;
} rmff_fileheader_t;

typedef struct {

	UINT32 object_id;
	UINT32 size;
	UINT16 object_version;

	UINT32 max_bit_rate;
	UINT32 avg_bit_rate;
	UINT32 max_packet_size;
	UINT32 avg_packet_size;
	UINT32 num_packets;
	UINT32 duration;
	UINT32 preroll;
	UINT32 index_offset;
	UINT32 data_offset;
	UINT16 num_streams;
	UINT16 flags;

} rmff_prop_t;

typedef struct {

	UINT32  object_id;
	UINT32  size;
	UINT16  object_version;

	UINT16  stream_number;
	UINT32  max_bit_rate;
	UINT32  avg_bit_rate;
	UINT32  max_packet_size;
	UINT32  avg_packet_size;
	UINT32  start_time;
	UINT32  preroll;
	UINT32  duration;
	UINT8   stream_name_size;
	char      *stream_name;
	UINT8   mime_type_size;
	char      *mime_type;
	UINT32  type_specific_len;
	char      *type_specific_data;

	int       mlti_data_size;
	char      *mlti_data;

} rmff_mdpr_t;

typedef struct {

	UINT32  object_id;
	UINT32  size;
	UINT16  object_version;

	UINT16  title_len;
	char      *title;
	UINT16  author_len;
	char      *author;
	UINT16  copyright_len;
	char      *copyright;
	UINT16  comment_len;
	char      *comment;

} rmff_cont_t;

typedef struct {

	UINT32 object_id;
	UINT32 size;
	UINT16 object_version;

	UINT32 num_packets;
	UINT32 next_data_header; /* rarely used */
} rmff_data_t;

typedef struct {

	rmff_fileheader_t *fileheader;
	rmff_prop_t *prop;
	rmff_mdpr_t **streams;
	rmff_cont_t *cont;
	rmff_data_t *data;
} rmff_header_t;

typedef struct {

	UINT16 object_version;

	UINT16 length;
	UINT16 stream_number;
	UINT32 timestamp;
	UINT8 reserved;
	UINT8 flags;

} rmff_pheader_t;

/*
* constructors for header structs
*/

rmff_fileheader_t *rmff_new_fileheader(UINT32 num_headers);

rmff_prop_t *rmff_new_prop (
							UINT32 max_bit_rate,
							UINT32 avg_bit_rate,
							UINT32 max_packet_size,
							UINT32 avg_packet_size,
							UINT32 num_packets,
							UINT32 duration,
							UINT32 preroll,
							UINT32 index_offset,
							UINT32 data_offset,
							UINT16 num_streams,
							UINT16 flags );

rmff_mdpr_t *rmff_new_mdpr(
						   UINT16   stream_number,
						   UINT32   max_bit_rate,
						   UINT32   avg_bit_rate,
						   UINT32   max_packet_size,
						   UINT32   avg_packet_size,
						   UINT32   start_time,
						   UINT32   preroll,
						   UINT32   duration,
						   const char *stream_name,
						   const char *mime_type,
						   UINT32   type_specific_len,
						   const char *type_specific_data );

rmff_cont_t *rmff_new_cont(
						   const char *title,
						   const char *author,
						   const char *copyright,
						   const char *comment);

rmff_data_t *rmff_new_dataheader(
								 UINT32 num_packets, UINT32 next_data_header);

/*
* reads header infos from data and returns a newly allocated header struct
*/
rmff_header_t *rmff_scan_header(const char *data);

/*
* scans a data packet header. Notice, that this function does not allocate
* the header struct itself.
*/
void rmff_scan_pheader(rmff_pheader_t *h, char *data);

/*
* reads header infos from stream and returns a newly allocated header struct
*/
//rmff_header_t *rmff_scan_header_stream(int fd);

/*
* prints header information in human readible form to stdout
*/
void rmff_print_header(rmff_header_t *h);

/*
* does some checks and fixes header if possible
*/
void rmff_fix_header(rmff_header_t *h);

/*
* returns the size of the header (incl. first data-header)
*/
int rmff_get_header_size(rmff_header_t *h);

/*
* dumps the header <h> to <buffer>. <max> is the size of <buffer>
*/
int rmff_dump_header(rmff_header_t *h, char *buffer, int max);

/*
* dumps a packet header
*/
void rmff_dump_pheader(rmff_pheader_t *h, char *data);

/*
* frees a header struct
*/
void rmff_free_header(rmff_header_t *h);


/************************************************************************/
/*              ¶îÍâµÄº¯Êý                                              */
/************************************************************************/
int select_mlti_data(const char *mlti_chunk, int mlti_size, int selection, char **out);

#endif