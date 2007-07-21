#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <convert.h>
#include <exportPNG.h>

using namespace std;

#define TEST_FLAG(x,y) if((x) & (y)) std::cout << #y << std::endl
#define DO_CHECKED(action, info, erraction) do {  \
	int error = action;                        \
	if (error) {                               \
		std::cout << info << std::endl;        \
		erraction;                             \
	}										   \
} while (0)
#define CEIL26DOT6(x) ((int)(((x) + (1<<6 - 1)) >> 6))
#define FLOOR26DOT6(x) ((int)((x) >> 6))
void render(FT_Bitmap in, unsigned char *out, int width, int xofs, int yofs) {
	for(int y=0; y<in.rows; y++) {
		for(int x=0; x<in.width; x++) {
			/*
			char c;
			unsigned char px = in.buffer[in.pitch*y + x];
			if      (px <  64) c = ' ';
			else if (px < 128) c = '·';
			else if (px < 192) c = 'o';
			else               c = '#';
			
			std::cout << c;
			*/
			
			out[(y+yofs)*width+x+xofs] = in.buffer[in.pitch*y + x];
		}
		//std::cout << std::endl;
	}
}




typedef pair<FT_ULong, FT_UInt> CharcodeGlyphPair;
typedef vector<CharcodeGlyphPair> Mapping;

Mapping getMapping(FT_Face face) {
	Mapping mapping;
	FT_ULong charcode;
	FT_UInt glyph;
	
	charcode = FT_Get_First_Char(face, &glyph);
	while (glyph != 0) {
		mapping.push_back(CharcodeGlyphPair(charcode,glyph));
		charcode = FT_Get_Next_Char(face, charcode, &glyph);
	}
	return mapping;
}

struct ByGlyph {
	bool operator() (const CharcodeGlyphPair & p1, const CharcodeGlyphPair & p2) {
		return p1.second < p2.second;
	}
};

void sortMappingByGlyph(Mapping & m) {
	sort(m.begin(), m.end(), ByGlyph());
}	

void getExactBoundingBox(FT_Face face, const Mapping & mapping, int *xmin, int *xmax, int *ymin, int *ymax) {
	*xmin=0x7fffffff; *xmax=0; *ymin=0x7fffffff; *ymax=0;
	for(int i=0; i<mapping.size(); i++) {
		FT_UInt glyph = mapping[i].second;
		int error = FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT);
		if (error) {
			cout << "error." << endl;
			continue;
		}
		
		*xmax = max(*xmax, CEIL26DOT6(face->glyph->metrics.horiBearingX + face->glyph->metrics.width));
		*xmin = min(*xmin, FLOOR26DOT6(face->glyph->metrics.horiBearingX));
		
		*ymax = max(*ymax, CEIL26DOT6(face->glyph->metrics.horiBearingY));
		*ymin = min(*ymin, FLOOR26DOT6(face->glyph->metrics.horiBearingY - face->glyph->metrics.height));
	}
}

	
void convert(const char *infile, const char *outfile, int size) {
	std::cout << "Converting font in file " << infile << " to file " << outfile;
	std::cout << " at size " << size << std::endl;
	
	FT_Library library;
	DO_CHECKED(FT_Init_FreeType( &library ), "Error initializing FreeType 2", return);
	
	FT_Face face;
	DO_CHECKED(FT_New_Face(library, infile, 0, &face ), "Error loading font", return);
		
	std::cout << face->family_name << " " << face->style_name << std::endl;
	std::cout << "Flags :" << std::endl;
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_SCALABLE);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_FIXED_SIZES);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_FIXED_WIDTH);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_SFNT);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_HORIZONTAL);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_VERTICAL);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_KERNING);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_FAST_GLYPHS);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_MULTIPLE_MASTERS);
	TEST_FLAG(face->face_flags,FT_FACE_FLAG_GLYPH_NAMES);
	
	DO_CHECKED(FT_Set_Pixel_Sizes( face, 0, size ), "Error etting pixel size.", return);
	
	Mapping mapping = getMapping(face);
	sortMappingByGlyph(mapping);
	int n_chars = mapping.size();
	
	int box_w, box_h, xmin, xmax, ymin, ymax;
	getExactBoundingBox(face, mapping, &xmin, &xmax, &ymin, &ymax);
	box_w = xmax + 1;
	box_h = ymax - ymin + 1;
	
	float ratio = (float) box_w / box_h;
	int n_x = (int) floor(sqrt(n_chars / ratio));
	int n_y = (int) ceil(n_chars / (float)n_x);
	
	cout << "Using layout of " << n_x << " x " << n_y << " glyphs." << endl;
	cout << "Required area is " << n_x*box_w << ", " << n_y*box_h << endl;
	
	int min_size = std::max(n_x*box_w, n_y*box_h);
	int min_pow2_size = 1;
	while (min_pow2_size < min_size) min_pow2_size <<= 1;
		
	std::cout << "Image size: " << min_pow2_size << std::endl;
	
	unsigned char * pixels = new unsigned char[min_pow2_size * min_pow2_size];
	if (!pixels) {
		std::cout << "Out of memory." << std::endl;
		return;
	}
	for(int i=0; i<min_pow2_size*min_pow2_size; i++) pixels[i]=0;
		
	ofstream font((std::string(outfile)+".font").c_str());
	
	font << face->family_name << std::endl;
	font << face->style_name << std::endl;
	font << "height " << CEIL26DOT6(face->size->metrics.height) << std::endl;
	font << "box_w " << box_w << std::endl;
	font << "box_h " << box_h << std::endl;
	
	char glyph_buffer[64];
	for (int y=0, c=0; y<n_y; y++) for (int x=0; x<n_x; x++, c++) {
		if (c >= mapping.size())
			break;
		std::cout << "rendering glyph " << mapping[c].second << " (charcode "<<mapping[c].first<<")";
		int error = FT_Get_Glyph_Name(face, mapping[c].second, glyph_buffer, 64);
		if (!error) {
			std::cout << " ("<< glyph_buffer <<")";
		}
		
		error = FT_Load_Glyph(face, mapping[c].second, FT_LOAD_RENDER);
		if (error) {
			std::cout << " [IGNORED]" << std::endl;
			continue;
		}
		std::cout << endl;
		
		render(face->glyph->bitmap, pixels, min_pow2_size,
			x*box_w + face->glyph->bitmap_left,
			y*box_h - face->glyph->bitmap_top + ymax);
		
		font << "charcode  " << mapping[c].first << endl;
		font << "name       " << (glyph_buffer[0]==0?"N/A":glyph_buffer) << endl;
		font << "box        " << x << " " << y << endl;
		font << "advance    " << CEIL26DOT6(face->glyph->metrics.horiAdvance) << endl;
		font << "width      " << CEIL26DOT6(face->glyph->metrics.width) << endl;
		font << "height     " << CEIL26DOT6(face->glyph->metrics.height) << endl;
		font << "bearingx   " << CEIL26DOT6(face->glyph->metrics.horiBearingX) << endl;
		font << "bearingy   " << CEIL26DOT6(face->glyph->metrics.horiBearingY) << endl;
	}
	
	exportPNG(outfile, pixels, min_pow2_size, min_pow2_size);
	
	delete [] pixels;
}
