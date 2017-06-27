#pragma once

/* Image type - contains height, width, and data */
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct Image Image;

int ImageLoadBMP(const char *filename, Image *image);
