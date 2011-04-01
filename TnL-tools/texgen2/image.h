#ifndef IMAGE_H
#define IMAGE_H

#define MAX_COLORS 256

class Image {
    int w,h;
    bool greyscale;
    unsigned int *palette;
    unsigned char *pixels;
public:
    Image(int w=0, int h=0, bool greyscale=false);
    Image(const char *filename);
    ~Image();
    
    void saveTo(const char *filename);
    
    bool hasPixel(int x, int y) { return x>=0 && y>=0 && x<w && y<h; }
    unsigned char & pixelAt(int x, int y) { return pixels[y*w+x]; }
    
    unsigned int & paletteAt(int idx) { return palette[idx]; }
    
    bool isGreyscale() { return greyscale; }
    
    int getWidth() { return w; }
    int getHeight() { return h; }
    
    void dump();
};

#endif
