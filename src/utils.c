#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "pic_utils.h"

/* local functions declarations */
static BYTE *buf_alloc_biggest_entry(rpl_header_t *rpl_header);

/* global data */
char *dirs[] = {"./Target rip/",
                "BRIEF",
                "SOUNDS",
                "MISSION",
                "TEXTS",
                "LEVELS",
                "DATA",
                "POST",
                "TT",
                "MD"
};


/* functions definitions */
BYTE *RPL_header_init(rpl_header_t *rpl_header, FILE *rplFile_fp){

    fread(&rpl_header->identifier, sizeof(rpl_header->identifier), 1, rplFile_fp);
    fread(&rpl_header->entry_count, sizeof(rpl_header->entry_count), 1, rplFile_fp);

    rpl_header->rpl_entryrecord = malloc(rpl_header->entry_count * sizeof(rpl_entry_t));
    if(!rpl_header->rpl_entryrecord){
        fprintf(stderr, "Couldn't malloc %u bytes for the RPL entries record/n", rpl_header->entry_count * sizeof(rpl_entry_t));
        exit(EXIT_FAILURE);
        }


    fread(rpl_header->rpl_entryrecord, sizeof(rpl_entry_t), rpl_header->entry_count, rplFile_fp);

    return buf_alloc_biggest_entry(rpl_header);
}

void swapByte(BYTE *x, BYTE *y){
    BYTE temp = *x;
    *x = *y;
    *y = temp;
}

/* local functions definitions */
static BYTE *buf_alloc_biggest_entry(rpl_header_t *rpl_header){
    BYTE *buf;
    unsigned int biggest_size = 0;
    unsigned int i;

    for(i = 0; i < rpl_header->entry_count; ++i)
        if(biggest_size < rpl_header->rpl_entryrecord[i].entry_size)
            biggest_size = rpl_header->rpl_entryrecord[i].entry_size;

    buf = malloc(biggest_size);
    if(!buf){
        fprintf(stderr, "Couldn't malloc %u bytes for the RPL entry buffer\n", biggest_size);
        exit(EXIT_FAILURE);
    }

    return buf;
}
