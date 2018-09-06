#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rip_tt.h"
#include "utils.h"
#include "pic_utils.h"
#include "makedir.h"

void rip_tt(void){
    extern char *dirs[];

    FILE *rplFile_fp;
    rpl_header_t rpl_header;
    char path[64];
    char entryName[9] = {0};
    char *namePtr;
    BYTE *entry_buf;

    unsigned int i;

    sprintf(path, "%s%s/", dirs[BASE_DIR], dirs[TT_DIR]);
    namePtr = path + strlen(path);

    if((rplFile_fp = fopen("TT.RPL", "rb")) == NULL){
        perror("Couldn't open RPL file");
        exit(EXIT_FAILURE);
    }


    entry_buf = RPL_header_init(&rpl_header, rplFile_fp);


    for(i = 0; i < rpl_header.entry_count; ++i){
        strncpy(entryName, rpl_header.rpl_entryrecord[i].entry_name, sizeof(rpl_header.rpl_entryrecord[0].entry_name));
        sprintf(namePtr, "%s.tga", entryName);

        fseek(rplFile_fp, rpl_header.rpl_entryrecord[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, rpl_header.rpl_entryrecord[i].entry_size, rplFile_fp);
        spriteHandler(entry_buf, path, IMG_FLIP_MIRROR);
    }

    free(entry_buf);
    free(rpl_header.rpl_entryrecord);
    fclose(rplFile_fp);
}
