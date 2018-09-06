#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rip_post.h"
#include "utils.h"
#include "pic_utils.h"
#include "makedir.h"

/* local functions declarations */
static void makePostDirs(char path[], const char *spriteDirs[]);

enum spriteDirs_e{
    CO_DIR,
    RA_DIR,
    PA_DIR,
    SW_DIR,
    AZ_DIR,
    GA_DIR,
    LA_DIR,
    ST_DIR,
    TA_DIR,
    ZO_DIR,
    OF_DIR
};


void rip_post(void){
    extern char *dirs[];

    const char *spriteDirs[] = {
        "CO",
        "RA",
        "PA",
        "SW",
        "AZ",
        "GA",
        "LA",
        "ST",
        "TA",
        "ZO",
        "OF"
    };

    FILE *rplFile_fp;
    rpl_header_t rpl_header;
    char path[64];
    char entryName[9] = {0};
    char *namePtr;
    BYTE *entry_buf;

    unsigned int i;

    sprintf(path, "%s%s/", dirs[BASE_DIR], dirs[POST_DIR]);
    namePtr = path + strlen(path);

    makePostDirs(path, spriteDirs);

    if((rplFile_fp = fopen("POST.RPL", "rb")) == NULL){
        perror("Couldn't open RPL file");
        exit(EXIT_FAILURE);
    }


    entry_buf = RPL_header_init(&rpl_header, rplFile_fp);


    for(i = 0; i < rpl_header.entry_count; ++i){
        strncpy(entryName, rpl_header.rpl_entryrecord[i].entry_name, sizeof(rpl_header.rpl_entryrecord[0].entry_name));
        sprintf(namePtr, "%.2s/%s.tga", entryName, entryName);

        fseek(rplFile_fp, rpl_header.rpl_entryrecord[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, rpl_header.rpl_entryrecord[i].entry_size, rplFile_fp);
        spriteHandler(entry_buf, path, IMG_FLIP);
    }

    free(entry_buf);
    free(rpl_header.rpl_entryrecord);
    fclose(rplFile_fp);
}


/* local functions definitions */
void makePostDirs(char path[], const char *spriteDirs[]){
    char *postPtr = path + strlen(path);
    int i;

    for(i = 0; i < 11; ++i){
    strcpy(postPtr, spriteDirs[i]);
    makeDir(path);
    }

    *postPtr = '\0';
}
