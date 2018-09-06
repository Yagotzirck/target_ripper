#ifndef UTILS_H
#define UTILS_H

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;

enum dirs_e{BASE_DIR,
            BRIEF_DIR,
            SOUNDS_DIR,
            MISSION_DIR,
            TEXTS_DIR,
            LEVELS_DIR,
            DATA_DIR,
            POST_DIR,
            TT_DIR,
            MD_DIR
};

typedef enum mipmapRipMode_e{
    MIP_RIP_ALL,
    MIP_RIP_BIGGEST
}mipmapRipMode_t;

typedef struct rpl_entry_s{
	DWORD entry_offset;
	char entry_name[8];	/* if name is 8 characters long there won't be 0-termination */
	DWORD entry_size;
}rpl_entry_t;

typedef struct rpl_header_s{
	DWORD identifier;	/* 0x1A4C5052 (another way to see it: 'R' 'P' 'L' 0x1A) */
	DWORD entry_count;
	rpl_entry_t *rpl_entryrecord;
} rpl_header_t;


/* function declarations */
BYTE*   RPL_header_init(rpl_header_t *rpl_header, FILE *rplFile_fp);
void    swapByte(BYTE *x, BYTE *y);
#endif /* UTILS_H */
