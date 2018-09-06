#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rip_data.h"
#include "makedir.h"
#include "utils.h"
#include "pic_utils.h"


#define DATA_SPRITES_BEGIN  122
#define DATA_FONTS_BEGIN    387
#define DATA_FONT_PALETTES_BEGIN 392

void rip_data(void){
    extern char *dirs[];
    FILE *rplFile_fp;
    rpl_header_t rpl_header;
    char path[64];
    char entryName[9] = {0};
    char *dataDirPtr, *fontDirPtr, *namePtr;
    BYTE *entry_buf;

    target_palette_t fontPal[256];

    unsigned int i, j;

    sprintf(path, "%s%s/", dirs[BASE_DIR], dirs[DATA_DIR]);
    dataDirPtr = path + strlen(path);

    if((rplFile_fp = fopen("DATA.RPL", "rb")) == NULL){
        perror("Couldn't open RPL file");
        exit(EXIT_FAILURE);
    }


    entry_buf = RPL_header_init(&rpl_header, rplFile_fp);

    /* handle the 2 bytes per pixel 565 pictures */
    strcpy(dataDirPtr, "images/");
    makeDir(path);
    namePtr = path + strlen(path);

    for(i = 0; i < DATA_SPRITES_BEGIN; ++i){
        strncpy(entryName, rpl_header.rpl_entryrecord[i].entry_name, sizeof(rpl_header.rpl_entryrecord[0].entry_name));
        sprintf(namePtr, "%s.tga", entryName);

        fseek(rplFile_fp, rpl_header.rpl_entryrecord[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, rpl_header.rpl_entryrecord[i].entry_size, rplFile_fp);
        pic565Handler(entry_buf, rpl_header.rpl_entryrecord[i].entry_size, path);
    }

    /* handle the sprite pictures */
    strcpy(dataDirPtr, "sprites/");
    makeDir(path);
    namePtr = path + strlen(path);

    for(; i < DATA_FONTS_BEGIN; ++i){
        strncpy(entryName, rpl_header.rpl_entryrecord[i].entry_name, sizeof(rpl_header.rpl_entryrecord[0].entry_name));
        sprintf(namePtr, "%s.tga", entryName);

        fseek(rplFile_fp, rpl_header.rpl_entryrecord[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, rpl_header.rpl_entryrecord[i].entry_size, rplFile_fp);
        spriteHandler(entry_buf, path, IMG_FLIP_MIRROR);
    }

    /* handle the fonts */
    strcpy(dataDirPtr, "fonts/");
    makeDir(path);
    namePtr = path + strlen(path);

    for(; i < DATA_FONT_PALETTES_BEGIN; ++i){
        strncpy(entryName, rpl_header.rpl_entryrecord[i].entry_name, sizeof(rpl_header.rpl_entryrecord[0].entry_name));
        sprintf(namePtr, "%s/", entryName);
        makeDir(path);

        fontDirPtr = path + strlen(path);

        /* acquire raw font data */
        fseek(rplFile_fp, rpl_header.rpl_entryrecord[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, rpl_header.rpl_entryrecord[i].entry_size, rplFile_fp);

        for(j = DATA_FONT_PALETTES_BEGIN; j < rpl_header.entry_count; ++j){
            strncpy(entryName, rpl_header.rpl_entryrecord[j].entry_name, sizeof(rpl_header.rpl_entryrecord[0].entry_name));
            sprintf(fontDirPtr, "%s/", entryName);
            makeDir(path);


            /* acquire palette */
            fseek(rplFile_fp, rpl_header.rpl_entryrecord[j].entry_offset, SEEK_SET);
            fread(fontPal, 1, rpl_header.rpl_entryrecord[j].entry_size, rplFile_fp);

            rip_font(entry_buf, fontPal, path);
        }
    }



    free(entry_buf);
    free(rpl_header.rpl_entryrecord);
    fclose(rplFile_fp);
}

