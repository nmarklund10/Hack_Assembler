#include <iostream>
#include <string>
#include "assembler.h"

using namespace std;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cerr<<"Error:  No input file!  Call program as: assembler \"filename.asm\"\n";
		exit(1);
	}
	string filename = argv[1];
	if (filename.substr(filename.size()-4) != std::string(".asm")) {
		cerr<<"Error:  Input file is not .asm file.\n";
		exit(1);
	}
	assembler f;
	f.assemble(filename);
}
