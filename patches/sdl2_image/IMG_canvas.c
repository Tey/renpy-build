/*
  SDL_image:  An example image loading library for use with SDL
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "SDL_endian.h"

#include "SDL_image.h"

struct CANVASheader {
    Uint8  magic[6];    // "CANVAS"
    Uint8  hdr_len[2];  // Header size
    Uint8  width[2];    // Image width
    Uint8  height[2];   // Image height
};

#define BE16(p) ((p)[1] + ((p)[0] << 8))

/* Load a CANVAS frame from an SDL datasource */
SDL_Surface *IMG_LoadCANVAS_RW(SDL_RWops *src)
{
    Sint64 start;
    const char *error = NULL;
    struct CANVASheader hdr;
    Uint16 hdr_len;
    Uint16 width;
    Uint16 height;

    void *pixels = NULL;
    SDL_Surface *img = NULL;

    // Canvas pixel are stored as RGBA, 8-bit for each component (32-bit big endian)

    if ( !src ) {
        /* The error message has been set in SDL_RWFromFile */
        return NULL;
    }
    start = SDL_RWtell(src);

    if (!SDL_RWread(src, &hdr, sizeof(hdr), 1)) {
        error = "Error reading CANVAS data";
        goto error;
    }

    if (SDL_memcmp(hdr.magic, "CANVAS", sizeof(hdr.magic))) {
        error = "Bad CANVAS magic";
        goto error;
    }

    hdr_len = BE16(hdr.hdr_len);
    width = BE16(hdr.width);
    height = BE16(hdr.height);

    if (hdr_len > sizeof(hdr)) {
      // Skip extra header fields (might be padding)
      SDL_RWseek(src, hdr_len - sizeof(hdr), RW_SEEK_CUR);
    }

    pixels = SDL_malloc(width * height * 4);
    if (!pixels) {
        error = "Out of memory";
        goto error;
    }

    if (!SDL_RWread(src, pixels, width * height * 4, 1)) {
        error = "Error reading CANVAS data";
        goto error;
    }

    img = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, width * 4,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#else
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
#endif
    if (img == NULL) {
        error = "Out of memory";
        goto error;
    }
    pixels = NULL;

    return img;

error:
    SDL_RWseek(src, start, RW_SEEK_SET);
    if ( pixels ) {
      SDL_free(pixels);
    }
    if ( img ) {
        SDL_FreeSurface(img);
    }
    IMG_SetError("%s", error);
    return NULL;
}
