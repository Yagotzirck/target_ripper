#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pic_utils.h"
#include "tga_utils.h"

/* macros for handling  2 bytes per pixel 565 pictures  */
#define BITMASK_RED(x)      ((x >> 11) & 0x1F)
#define BITMASK_GREEN(x)    ((x >> 5) & 0x3F)
#define BITMASK_BLUE(x)     (x & 0x1F)


/* palette positioning macros */
#define MD_PALETTES_OFFSET  0x846356
#define PALETTE_INDEX(i) ((i) * 768)

/* local functions declarations */
static int findFirstUnusedPixel(const BYTE entry_buf[], const WORD offsets[], WORD height);
static void imgFlip(BYTE dest[], const BYTE src[], WORD width, WORD height);
static void imgFlipMirror(BYTE dest[], const BYTE src[], WORD width, WORD height);

void pic565Handler(const BYTE *entry_buf, DWORD size, const char path[]){

    typedef struct palette565ToRGB24_s{
        BYTE red_blue[32];
        BYTE green[64];
    }palette565ToRGB24_t;

    static const palette565ToRGB24_t palette565ToRGB24 =
        {{  0x00, 0x00, 0x08, 0x10, 0x18, 0x21, 0x29, 0x31,             /* red, blue */
            0x39, 0x42, 0x4A, 0x52, 0x5A, 0x63, 0x6B, 0x73,
            0x7B, 0x84, 0x8C, 0x94, 0x9C, 0xA5, 0xAD, 0xB5,
            0xBD, 0xC6, 0xCE, 0xD6, 0xDE, 0xE7, 0xEF, 0xF7},

         {  0x00, 0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18,             /* green */
            0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38,
            0x3C, 0x41, 0x45, 0x49, 0x4D, 0x51, 0x55, 0x59,
            0x5D, 0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79,
            0x7D, 0x82, 0x86, 0x8A, 0x8E, 0x92, 0x96, 0x9A,
            0x9E, 0xA2, 0xA6, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA,
            0xBE, 0xC3, 0xC7, 0xCB, 0xCF, 0xD3, 0xD7, 0xDB,
            0xDF, 0xE3, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFB}};


    DWORD pixel565, tga_pixel_index, rplEntryCounter, rgb24Size, width, height;
    BYTE *rgb24Buf;
    FILE *tga_fp;

    if((tga_fp = fopen(path, "wb")) == NULL){
        fprintf(stderr, "Couldn't create %s; skipping to next entry\n", path);
        return;
    }
    rgb24Size = ((size - 8) / 2) * 3;

    width = *(DWORD *)entry_buf;
    height = *(DWORD *)(entry_buf + 4);

    set_tga_hdr(NO_PALETTE, IMGTYPE_TRUECOLOR, 0, 0, width, height, 24, TOP_LEFT);

    write_tga_hdr(tga_fp);

    if((rgb24Buf = malloc(rgb24Size)) == NULL){
        fprintf(stderr, "Couldn't allocate %u bytes for %s; skipping to next entry\n", rgb24Size, path);
        return;
    }



    tga_pixel_index = 0;

    for(rplEntryCounter = 8; rplEntryCounter < size; rplEntryCounter += 2){
        pixel565 = *(WORD *)(entry_buf + rplEntryCounter);
        rgb24Buf[tga_pixel_index++] = palette565ToRGB24.red_blue[BITMASK_BLUE(pixel565)];   /* blue */
        rgb24Buf[tga_pixel_index++] = palette565ToRGB24.green[BITMASK_GREEN(pixel565)];     /* green */
        rgb24Buf[tga_pixel_index++] = palette565ToRGB24.red_blue[BITMASK_RED(pixel565)];    /* red */
        }

    fwrite(rgb24Buf, 1, rgb24Size, tga_fp);

    free(rgb24Buf);
    fclose(tga_fp);

}


void spriteHandler(BYTE entry_buf[], const char path[], enum imgFlipType_e imgFlipType){

    typedef struct sprite_hdr_s {
        WORD height;
        WORD width;
        WORD unknown1;   /* no idea(maybe x offset?), seems to be irrelevant for the ripping process anyway */
        WORD yOffset;    /* the height at which the sprite top gets drawn, relative to the floor; irrelevant for the ripping process */
        WORD pal_idx;
    }sprite_hdr_t;

    typedef struct block_header_s{
        WORD size;
        WORD rowOffset;
    }block_header_t;

    sprite_hdr_t *sprite_hdr;

    const block_header_t *blkHdr;

    target_palette_t trgtPal[256];

    FILE *tga_fp;
    FILE *MD_fp;    /* used to open MD.RPL which contains the palettes used by the sprites/textures */

    const WORD *offsets = entry_buf + sizeof(sprite_hdr_t);
    BYTE *pixelBuf, *flippedBuf, *shrunkBuf;
    BYTE *bufToWrite;
    const BYTE *rowPtr;
    BYTE *tgaRowBegin;
    unsigned int i, rawDataSize, blockCnt;
    int firstUnusedPixel;
    int shrunk_size;
    enum tgaImageType imgType;

    void (*imgFlip_f[2])(BYTE dest[], const BYTE src[], WORD width, WORD height) = {
        imgFlip,
        imgFlipMirror
    };


    /* acquire the sprite header and adjust the width value */
    sprite_hdr = entry_buf;
    ++sprite_hdr->width;

    rawDataSize = sprite_hdr->width * sprite_hdr->height;

    /* acquire palette */
    if((MD_fp = fopen("MD.RPL", "rb")) == NULL){
        perror("Couldn't open MD.RPL");
        exit(EXIT_FAILURE);
    }
    fseek(MD_fp, MD_PALETTES_OFFSET + PALETTE_INDEX(sprite_hdr->pal_idx), SEEK_SET);
    fread(trgtPal, sizeof(target_palette_t), 256, MD_fp);
    fclose(MD_fp);

    /* find the first unused pixel so that we can use it for transparency */
    firstUnusedPixel = findFirstUnusedPixel(entry_buf, offsets, sprite_hdr->height);
    if(firstUnusedPixel == -1){
        fprintf(stderr, "%s uses all 256 palette entries; can't use any pixel for transparency\n", path);
        exit(EXIT_FAILURE);
    }

    /* swap palette index 0 with the first unused pixel */
    swapPaletteEntry(&trgtPal[0], &trgtPal[firstUnusedPixel]);



    /* proceed converting the sprite format into 8-bit pixels data */
    if((pixelBuf = calloc(rawDataSize, sizeof(BYTE))) == NULL){
        fprintf(stderr, "Couldn't allocate %u bytes for %s; skipping to next entry\n", rawDataSize, path);
        return;
    }

    tgaRowBegin = pixelBuf;
    for(i = 0; i < sprite_hdr->height; ++i){
        rowPtr = &entry_buf[offsets[i]];

        do{
            blkHdr = rowPtr;

            rowPtr += sizeof(block_header_t);

            for(blockCnt = 0; blockCnt < blkHdr->size; ++blockCnt){
                tgaRowBegin[blkHdr->rowOffset + blockCnt] = rowPtr[blockCnt];
                if(tgaRowBegin[blkHdr->rowOffset + blockCnt] == 0)
                    tgaRowBegin[blkHdr->rowOffset + blockCnt] = firstUnusedPixel;
            }

            rowPtr += blkHdr->size;
        }while(blkHdr->size);

        tgaRowBegin += sprite_hdr->width;
    }


    if((flippedBuf = malloc(rawDataSize)) == NULL){
        fprintf(stderr, "Couldn't allocate %u bytes for %s's flipped image buffer; skipping to next entry\n", rawDataSize, path);
        return;
    }

    (*imgFlip_f[imgFlipType])(flippedBuf, pixelBuf, sprite_hdr->width, sprite_hdr->height);

    /* in the worst case scenario RLE encoding will occupy twice the size of unencoded data */
    if((shrunkBuf = malloc(rawDataSize * 2)) == NULL){
        fprintf(stderr, "Couldn't allocate %u bytes for %s's shrunk image buffer; skipping to next entry\n", rawDataSize * 2, path);
        return;
    }
    shrunk_size = tga_compressData(shrunkBuf, flippedBuf, rawDataSize);

    /* RLE compression resulted in increased data size */
    if(shrunk_size == -1){
        bufToWrite = flippedBuf;
        imgType = IMGTYPE_COLORMAPPED;
    }
    /* RLE compression worked fine; save the compressed data */
    else{
        bufToWrite = shrunkBuf;
        imgType = IMGTYPE_COLORMAPPED_RLE;
        rawDataSize = shrunk_size;
    }

    if((tga_fp = fopen(path, "wb")) == NULL){
        fprintf(stderr, "Couldn't create %s; skipping to next entry\n", path);
        return;
    }


    /* set and write the tga header */
    set_tga_hdr(PALETTED, imgType, 256, 32, sprite_hdr->height, sprite_hdr->width, 8, ATTRIB_BITS | TOP_LEFT);
    write_tga_hdr(tga_fp);


    /* set and write the tga palette */
    trgtToTgaPal(trgtPal, ALPHA);
    write_tga_pal(tga_fp);

    /* write the raw data */
    fwrite(bufToWrite, 1, rawDataSize, tga_fp);

    free(pixelBuf);
    free(flippedBuf);
    free(shrunkBuf);
    fclose(tga_fp);
}


void rip_font(const BYTE *entry_buf, target_palette_t fontPal[], char path[]){
    typedef struct fontSymbol_s {
        BYTE height;
        BYTE width;
        BYTE *raw_data;
        DWORD raw_data_length;
    }fontSymbol_t;

    typedef struct font_s {
        DWORD header;
        DWORD height;
        BYTE transparency;
        DWORD *offset_table;
        const BYTE *raw_data;
        DWORD offsets[256];
        fontSymbol_t symbols[256];
    }font_t;

    FILE *tga_fp;
    font_t font;

    BYTE *pixelBuf, *shrunkBuf;
    BYTE *bufToWrite;

    int shrunkSize;
    WORD CMapLen;
    enum tgaImageType imgType;
    unsigned int i, j;

    char *fontNamePtr = path + strlen(path);



    font.raw_data = entry_buf;
    font.header = *(DWORD *)font.raw_data;
    font.height = font.raw_data[0];
    font.transparency=font.raw_data[1];
    font.offset_table = (DWORD*)(font.raw_data + 8);

    /* swap palette index 0 with the pixel used for transparency */
    swapPaletteEntry(&fontPal[0], &fontPal[font.transparency]);

    for (i=0; i<256; i++) {
        font.symbols[i].width = font.offset_table[i] >> 24;
        if (font.offset_table[i] & 0x00FFFFFF)
            font.symbols[i].raw_data = font.raw_data + ( font.offset_table[i] & 0x00FFFFFF );
        else
            continue;   /* empty font */
        if(font.symbols[i].width)
            font.symbols[i].height = font.height;
        else
            continue;  /* empty font */
            /* font.symbols[i].height = 0; */

        font.symbols[i].raw_data_length = font.symbols[i].width * font.symbols[i].height;


        if((pixelBuf = malloc(font.symbols[i].raw_data_length)) == NULL){
            fprintf(stderr, "Couldn't allocate %u bytes for %s's image buffer\n", font.symbols[i].raw_data_length, path);
            exit(EXIT_FAILURE);
        }

        memcpy(pixelBuf, font.symbols[i].raw_data, font.symbols[i].raw_data_length);

        /* change the raw data pixels to match the "index 0 = transparency" criteria */
        for(j = 0; j < font.symbols[i].raw_data_length; ++j){
            if(pixelBuf[j] == 0)
                pixelBuf[j] = font.transparency;
            else
            if(pixelBuf[j] == font.transparency)
                pixelBuf[j] = 0;
        }

        sprintf(fontNamePtr, "%03d.tga", i);

        trgtToTgaPal(fontPal, ALPHA);

        if((shrunkBuf = malloc(font.symbols[i].raw_data_length * 2)) == NULL){
            fprintf(stderr, "Couldn't allocate %u bytes for %s's shrunk image buffer\n", font.symbols[i].raw_data_length * 2, path);
            exit(EXIT_FAILURE);
        }

        shrunkSize = shrink_tga(shrunkBuf, pixelBuf, font.symbols[i].raw_data_length, &CMapLen);

        /* RLE compression resulted in increased data size */
        if(shrunkSize == -1){
            bufToWrite = pixelBuf;
            imgType = IMGTYPE_COLORMAPPED;
        }
        /* RLE compression worked fine; save the compressed data */
        else{
            bufToWrite = shrunkBuf;
            imgType = IMGTYPE_COLORMAPPED_RLE;
            font.symbols[i].raw_data_length = shrunkSize;
        }

        if((tga_fp = fopen(path, "wb")) == NULL){
            fprintf(stderr, "Couldn't create %s; skipping to next entry\n", path);
            return;
        }


        /* set and write the tga header */
        set_tga_hdr(PALETTED, imgType, CMapLen, 32, font.symbols[i].width, font.symbols[i].height, 8, ATTRIB_BITS | TOP_LEFT);
        write_tga_hdr(tga_fp);


        /*write the tga palette */
        write_shrunk_tga_pal(tga_fp);

        /* write the raw data */
        fwrite(bufToWrite, 1, font.symbols[i].raw_data_length, tga_fp);

        free(pixelBuf);
        free(shrunkBuf);
        fclose(tga_fp);

    }
}


void rip_mipmap(BYTE *rawData, DWORD size, DWORD width, DWORD height, target_palette_t imgPal[], char path[], mipmapRipMode_t mipmapRipMode){
    FILE *tga_fp;

    BYTE *flippedBuf, *shrunkBuf;
    BYTE *bufToWrite;

    int shrunkSize;
    WORD CMapLen;

    DWORD mipMapIndex = 0;
    enum tgaImageType imgType;
    unsigned int i, j;

    char *sizeLabelPtr = path + strlen(path);
    DWORD mipMapSize = width * height;
    DWORD rawBytesToWrite;

    if((flippedBuf = malloc(mipMapSize)) == NULL){
        fprintf(stderr, "Couldn't allocate %u bytes for %s's flipped image buffer\n", mipMapSize, path);
        exit(EXIT_FAILURE);
    }

    if((shrunkBuf = malloc(mipMapSize * 2)) == NULL){
        fprintf(stderr, "Couldn't allocate %u bytes for %s's shrunk image buffer\n", mipMapSize * 2, path);
        exit(EXIT_FAILURE);
    }

    if(mipmapRipMode == MIP_RIP_BIGGEST)
        size = mipMapSize;


    do{
        if(mipmapRipMode == MIP_RIP_BIGGEST)
            strcpy(sizeLabelPtr, ".tga");
        else
            sprintf(sizeLabelPtr, "%ux%u.tga", height, width);

        if((tga_fp = fopen(path, "wb")) == NULL){
            fprintf(stderr, "Couldn't create %s\n", path);
            exit(EXIT_FAILURE);
        }

        /* set the tga palette */
        trgtToTgaPal(imgPal, NO_ALPHA);

        imgFlipMirror(flippedBuf, &rawData[mipMapIndex], width, height);

        shrunkSize = shrink_tga(shrunkBuf, flippedBuf, mipMapSize, &CMapLen);

        /* RLE compression resulted in increased data size */
        if(shrunkSize == -1){
            bufToWrite = flippedBuf;
            imgType = IMGTYPE_COLORMAPPED;
            rawBytesToWrite = mipMapSize;
        }
        /* RLE compression worked fine; save the compressed data */
        else{
            bufToWrite = shrunkBuf;
            imgType = IMGTYPE_COLORMAPPED_RLE;
            rawBytesToWrite = shrunkSize;
        }

        /* set and write the tga header */
        set_tga_hdr(PALETTED, imgType, CMapLen, 32, height, width, 8, ATTRIB_BITS | TOP_LEFT);
        write_tga_hdr(tga_fp);


        /* write the tga palette */
        write_shrunk_tga_pal(tga_fp);

        /* write the image data */
        fwrite(bufToWrite, 1, rawBytesToWrite, tga_fp);

        fclose(tga_fp);
        width /= 2;
        height /= 2;
        mipMapIndex += mipMapSize;
        mipMapSize = width * height;
    }while(mipMapIndex < size);

    free(flippedBuf);
    free(shrunkBuf);
    fclose(tga_fp);
}



void swapPaletteEntry(target_palette_t *x, target_palette_t *y){
    target_palette_t temp = *x;
    *x = *y;
    *y = temp;
}



/* local functions definitions */
static int findFirstUnusedPixel(const BYTE entry_buf[], const WORD offsets[], WORD height){
    typedef struct block_header_s{
        WORD size;
        WORD rowOffset;
    }block_header_t;

    BYTE usedPixels[256] = {0};

    const block_header_t *blkHdr;
    const BYTE *rowPtr;
    int i, blockCnt;

    for(i = 0; i < height; ++i){
        rowPtr = &entry_buf[offsets[i]];

        do{
            blkHdr = rowPtr;

            rowPtr += sizeof(block_header_t);

            for(blockCnt = 0; blockCnt < blkHdr->size; ++blockCnt)
                usedPixels[rowPtr[blockCnt]] = 1;

            rowPtr += blkHdr->size;
        }while(blkHdr->size);
    }

    for(i = 0; i < 256; ++i)
        if(!usedPixels[i])
            return i;

    return -1;
}


static void imgFlip(BYTE dest[], const BYTE src[], WORD width, WORD height){
    unsigned int x, y;
    unsigned int xDest, yDest = height - 1;

    for(x = 0; x < height; ++x, --yDest)
        for(y = 0, xDest = 0; y < width; ++y, ++xDest)
            dest[xDest*height + yDest] = src[x*width + y];
}

static void imgFlipMirror(BYTE dest[], const BYTE src[], WORD width, WORD height){
    unsigned int x, y;

    for(x = 0; x < height; ++x)
        for(y = 0; y < width; ++y)
            dest[y*height + x] = src[x*width + y];
}
