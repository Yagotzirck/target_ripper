#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rip_md.h"
#include "makedir.h"
#include "utils.h"
#include "pic_utils.h"

#define PALETTE_ENTRIES 39
#define FLAT_ENTRIES    131
#define FLATL_ENTRIES   12
#define TEXTURE_ENTRIES 331

#define FLATDATA_INDEX 693
#define FLATNAME_INDEX 827

#define PATNNAME_INDEX 825
#define TEXTNAME_INDEX 826

#define TEXTURES_START  1
#define FLATSL_START     694
#define FLATSH_START     706

/* palette positioning macros */
#define MD_PALETTES_OFFSET  0x846356
#define PALETTE_INDEX(i) ((i) * 768)

void rip_md(mipmapRipMode_t mipmapRipMode){
    typedef struct mipmap_texture_hdr_s{
        DWORD height;
        DWORD width;
        DWORD pal_idx;
    }mipmap_texture_hdr_t;

    /* this might give some issues, depending on the order the compiler's implementation gives to bitfields
    ** (flat_size is the most significant bit in the byte, so adjust it accordingly in case it gives you issues)
    */
    typedef struct flatData_s{
        BYTE pal_idx:   7;
        BYTE flat_size: 1;
    }flatData_t;



    extern char *dirs[];
    FILE *rplFile_fp;
    rpl_header_t rpl_header;
    char path[64];
    char entryName[9] = {0};
    char *mdDirPtr, *subDirPtr, *namePtr;
    rpl_entry_t *textPtr, *flatPtr;
    mipmap_texture_hdr_t *mipHdr;

    target_palette_t imgPal[256];

    flatData_t flatData[FLAT_ENTRIES]; /* palette index + size for each flat entry */
    DWORD flatWidthHeight;

    char flatName[FLAT_ENTRIES][8];
    char textureName[TEXTURE_ENTRIES][8];
    char patnName[691][8];

    BYTE *entry_buf;

    unsigned int i, j;

    sprintf(path, "%s%s/", dirs[BASE_DIR], dirs[MD_DIR]);
    mdDirPtr = path + strlen(path);

    if((rplFile_fp = fopen("MD.RPL", "rb")) == NULL){
        perror("Couldn't open RPL file");
        exit(EXIT_FAILURE);
    }


    entry_buf = RPL_header_init(&rpl_header, rplFile_fp);

     /* initialize the name tables + the flatData palette indexes */
    fseek(rplFile_fp, rpl_header.rpl_entryrecord[PATNNAME_INDEX].entry_offset, SEEK_SET);
    fread(*patnName, 1, rpl_header.rpl_entryrecord[PATNNAME_INDEX].entry_size, rplFile_fp);

    fseek(rplFile_fp, rpl_header.rpl_entryrecord[TEXTNAME_INDEX].entry_offset, SEEK_SET);
    fread(*textureName, 1, rpl_header.rpl_entryrecord[TEXTNAME_INDEX].entry_size, rplFile_fp);

    fseek(rplFile_fp, rpl_header.rpl_entryrecord[FLATNAME_INDEX].entry_offset, SEEK_SET);
    fread(*flatName, 1, rpl_header.rpl_entryrecord[FLATNAME_INDEX].entry_size, rplFile_fp);
    /* flatname[0] is left empty for some reason, so we put the RPL name on it */
    strncpy(flatName[0], rpl_header.rpl_entryrecord[FLATSL_START].entry_name, 8);


    fseek(rplFile_fp, rpl_header.rpl_entryrecord[FLATDATA_INDEX].entry_offset, SEEK_SET);
    fread(flatData, 1, rpl_header.rpl_entryrecord[FLATDATA_INDEX].entry_size, rplFile_fp);


    /* handle the texture pictures */
    strcpy(mdDirPtr, "textures/");
    makeDir(path);
    namePtr = path + strlen(path);

    textPtr = &rpl_header.rpl_entryrecord[TEXTURES_START];

    for(i = 0; i < TEXTURE_ENTRIES; ++i){
        strncpy(entryName, textureName[i], 8);

        if(mipmapRipMode == MIP_RIP_ALL){
            sprintf(namePtr, "%s/", entryName);
            makeDir(path);
            subDirPtr = path + strlen(path);
            sprintf(subDirPtr, "%s_", entryName);
        }
        else
            strcpy(namePtr, entryName);

        fseek(rplFile_fp, textPtr[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, textPtr[i].entry_size, rplFile_fp);

        mipHdr = entry_buf;

        /* palette acquiring */
        fseek(rplFile_fp, MD_PALETTES_OFFSET + PALETTE_INDEX(mipHdr->pal_idx), SEEK_SET);
        fread(imgPal, sizeof(imgPal[0]), sizeof(imgPal) / sizeof(imgPal[0]), rplFile_fp);

        rip_mipmap(entry_buf + sizeof(mipmap_texture_hdr_t), textPtr[i].entry_size - sizeof(mipmap_texture_hdr_t), mipHdr->width, mipHdr->height, imgPal, path, mipmapRipMode);
    }

    /* handle the flat pictures */
    strcpy(mdDirPtr, "flats/");
    makeDir(path);
    namePtr = path + strlen(path);

    flatPtr = &rpl_header.rpl_entryrecord[FLATSL_START];

    for(i = 0; i < FLAT_ENTRIES; ++i){
        strncpy(entryName, flatName[i], 8);

        if(mipmapRipMode == MIP_RIP_ALL){
            sprintf(namePtr, "%s/", entryName);
            makeDir(path);
            subDirPtr = path + strlen(path);
            sprintf(subDirPtr, "%s_", entryName);
        }
        else
            strcpy(namePtr, entryName);

        fseek(rplFile_fp, flatPtr[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, flatPtr[i].entry_size, rplFile_fp);

        mipHdr = entry_buf;

        /* palette acquiring */
        fseek(rplFile_fp, MD_PALETTES_OFFSET + PALETTE_INDEX(flatData[i].pal_idx), SEEK_SET);
        fread(imgPal, sizeof(imgPal[0]), sizeof(imgPal) / sizeof(imgPal[0]), rplFile_fp);

        flatWidthHeight = flatData[i].flat_size == 1 ? 128 : 64;

        rip_mipmap(entry_buf, flatPtr[i].entry_size, flatWidthHeight, flatWidthHeight, imgPal, path, mipmapRipMode);
    }


    free(entry_buf);
    free(rpl_header.rpl_entryrecord);
    fclose(rplFile_fp);
}


