#include <cstdio>
#include <iostream>
#include <png.h>

#include <exportPNG.h>

using namespace std;

void exportPNG(const char*filename, const unsigned char *pixels, int w, int h) {
	FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
       	cout << "Error opening file "<<filename<<" for writing."<<endl;
		return;
    }
	
	png_structp png_ptr = png_create_write_struct
       (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
		cout << "Couldn't initialize PNG library." << endl;
       	return;
	}

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
		cout << "Couldn't initialize PNG library." << endl;
       	png_destroy_write_struct(&png_ptr,
         	(png_infopp)NULL);
       	return;
    }
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		cout << "Couldn't initialize PNG library." << endl;
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
      	return;
   	}
	
	png_init_io(png_ptr, fp);
	
	png_set_IHDR(png_ptr, info_ptr, w, h,
       	8, PNG_COLOR_TYPE_GRAY_ALPHA, PNG_INTERLACE_NONE,
       	PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	png_write_info(png_ptr, info_ptr);
	
	unsigned short *row = new unsigned short[w];
	for(int y=0; y<h; y++) {
	    for(int x=0; x<w; ++x) {
	        row[x] = (pixels[y*w+x] << 8) | 0xff;
	    }
		png_write_row(png_ptr, (png_bytep)row);
	}
	delete [] row;
	
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}
