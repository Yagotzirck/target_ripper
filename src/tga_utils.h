#ifndef TGA_UTILS_H
#define TGA_UTILS_H

#include <stdio.h>

#include "utils.h"
#include "pic_utils.h"

enum tgaImageDescriptor{
    ATTRIB_BITS =   8,
    BOTTOM_LEFT =   0x00,
    TOP_LEFT    =   0x20
};

enum tgaImageType{
    IMGTYPE_COLORMAPPED =       1,
    IMGTYPE_TRUECOLOR =         2,
    IMGTYPE_COLORMAPPED_RLE =   9
};

enum tgaColorMap{
    NO_PALETTE =    0,
    PALETTED =      1
};

enum tgaPalIsAlpha_e{
    NO_ALPHA,
    ALPHA
};

/* function prototypes */
void trgtToTgaPal(const target_palette_t *trgt_palette, enum tgaPalIsAlpha_e isAlpha);
void set_tga_hdr(enum tgaColorMap isCMapped, enum tgaImageType imgType, WORD CMapLen, BYTE CMapDepth, WORD width, WORD height, BYTE PixelDepth, enum tgaImageDescriptor ImageDesc);
void write_tga_hdr(FILE *stream);
void write_shrunk_tga_pal(FILE *stream);
void write_tga_pal(FILE *stream);
int  tga_compressData(BYTE imgDest[], const BYTE imgBuf[], DWORD size);
int shrink_tga(BYTE imgDest[], BYTE imgBuf[], DWORD size, WORD *CMapLen);


#endif /* TGA_UTILS_H */
