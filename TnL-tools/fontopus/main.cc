#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <convert.h>

using namespace std;

void usage() {
	cout << "Fontopus - Converts a TrueType or Type1 font into a png file plus glyph info." << endl;
	cout << "Usage: fontopus [-s <size>] [-o <outfile>] [-h] <infile>" << endl;
	cout << "where"<<endl;
	cout << "  <infile>         is the name of the font file to be read." << endl;
	cout << "  -s, --font-size  sets the desired font size in pixels (default 12)." << endl;
	cout << "  -f, --first-char sets the first char which is to be exported (defaults to 0)" << endl;
	cout << "  -l, --last-char  sets the last char which is to be exported (optional)" << endl;
	cout << "  -o, --outfile    set the name of the output .png file to write." << endl;
	cout << "                   The info file will be named <outfile>.font" << endl;
	cout << "  -h               shows this help text." << endl;
}

static struct option options [] = {
	{"font-size", 1, 0,'s'},
	{"first-char", 1, 0,'f'},
	{"last-char", 1, 0,'l'},
	{"output", 1, 0, 'o'},
	{"help", 0, 0, 'h'},
	{0,0,0,0}
};


int main(int argc, char **argv)
{
	int fontsize = 12;
	unsigned int min_charcode = 0;
	unsigned int max_charcode = UINT_MAX;
	char * filename = "output.png";
	
	int option_index;
	while (true) {
		int c = getopt_long(argc, argv, "s:f:l:o:h", options, &option_index);
		if (c == -1) break;
		switch(c) {
		case 's':
			fontsize = atoi(optarg);
			std::cout << "font size: " << fontsize << std::endl;
			break;
		case 'f':
			min_charcode = atoi(optarg);
			std::cout << "first char: " << min_charcode << std::endl;
			break;
		case 'l':
			max_charcode = atoi(optarg);
			std::cout << "last char: " << max_charcode << std::endl;
			break;
		case 'o':
			filename = optarg;
			std::cout << "output file: " << filename << std::endl;
			break;
		case 'h':
		default:
			usage();
			return 0;
		}
	}
	
	if (optind >= argc) {
		std::cout << "input filename missing" << std::endl;
		usage();
		return -1;
	}
	
	convert(argv[optind], filename, fontsize, min_charcode, max_charcode);
			
	return 0;
}
