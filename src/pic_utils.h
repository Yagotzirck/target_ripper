#ifndef PIC_UTILS_H
#define PIC_UTILS_H

#include "utils.h"

typedef struct target_palette_s{
    BYTE red, green, blue;
}target_palette_t;

enum imgFlipType_e{
    IMG_FLIP,
    IMG_FLIP_MIRROR
};

void    pic565Handler(const BYTE *entry_buf, DWORD size, const char path[]);
void    spriteHandler(BYTE *entry_buf, const char path[], enum imgFlipType_e imgFlipType);
void    rip_font(const BYTE *entry_buf, target_palette_t fontPal[], char path[]);
void rip_mipmap(BYTE *rawData, DWORD size, DWORD width, DWORD height, target_palette_t imgPal[], char path[], mipmapRipMode_t mipmapRipMode);
void    swapPaletteEntry(target_palette_t *x, target_palette_t *y);


#endif /* PIC_UTILS_H */
