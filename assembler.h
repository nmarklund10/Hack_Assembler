#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <algorithm>
#include <bitset>
#include <stdio.h>

using namespace std;
typedef pair<string, int> symbol;
typedef map<string, int> symb_tab;

class assembler {
	public:
		void assemble (string filename) {
			filename = filename.substr(0, filename.find_first_of(".")+1);		//remove file extension from input file name
			file_name = filename;
			first_pass(filename + "asm");										//searches for symbols and inserts into symbol_table
			second_pass(filename + "hack"); 										//Parses the file and converts to machine code
		}
	private:
		/// Helper Functions for Assembly ****************************************
		
		void first_pass (string filename) {
			find_parentheses_symbols(filename);
			find_rest_of_symbols();
		}
		
		void second_pass(string filename) {
			line_counter = 0;
			ofstream ofs(filename);
			if (!ofs)
				error("Error:  Could not write to file \"" + filename + "\" during the second pass.");
			for (; line_counter < input_file.size(); ++line_counter) {
				if (input_file[line_counter][0] == '@')
					ofs<<parse_a_instruction(input_file[line_counter].substr(1))<<endl;
				else
					ofs<<parse_c_instruction(input_file[line_counter])<<endl;
			}
		}
		///* Helper Functions for First Pass *///

		void find_parentheses_symbols(string filename) {
			string line;
			ifstream ifs(filename);
			if (!ifs)
				error("Error:  Could not open file \"" + filename + "\" during the first pass.");
			while (getline(ifs, line)) {
				line = trim(line);
				if (line != "") {
					if (line[0] == '(') {
						size_t p = line.find(")");
						line = line.substr(1,p-1);
						line.erase(remove(line.begin(),line.end(),' '),line.end());
						if (p == string::npos || invalid_symbol_name(line))
							error("Error:  Invalid symbol name at line " + to_string(line_counter) + ".");
						if (symbol_table.find(line) != symbol_table.end())
							error("Error:  Second declaration of symbol name found at line " + to_string(line_counter) + ".");
						symbol_table.insert(symbol(line, line_counter));
					}
					else{
						input_file.push_back(line);
						++line_counter;
					}
				}
			}
		}
		
		void find_rest_of_symbols() {
			line_counter = 0;
			for (; line_counter < input_file.size(); ++line_counter) {
				if (input_file[line_counter][0] == '@' && isalpha(input_file[line_counter][1])) {
					string symb = input_file[line_counter].substr(1);
					if (invalid_symbol_name(symb))
						error("Error:  Invalid symbol name at line " + to_string(line_counter) + ".");
					if (symbol_table.find(symb) == symbol_table.end()) {
						symbol_table.insert(symbol(symb, symbol_counter));
						++symbol_counter;
					}
				}
			}
		}
		///* General Helper Functions for Finding Symbols *///
		
		string trim (string s) {						///Strips comments and leading and trailing whitespace from lines
			s = s.substr(0,s.find("//"));
			size_t a = s.find_first_not_of(" \t\n\r\v\f\b");
			if (a != string::npos) {
				s = s.substr(a);
				size_t b = s.find_last_not_of(" \t\n\r\v\f\b");
				if (b != string::npos)
					s.erase(b+1);
				return s;
			}
			return "";
		}
		
		bool invalid_symbol_name (string s) {
			if (s.find("%") != string::npos)
				return true;
			if (s.find(";") != string::npos)
				return true;
			if (s.find("@") != string::npos)
				return true;
			if (s.find("#") != string::npos)
				return true;
			if (s.find("^") != string::npos)
				return true;
			if (s.find("*") != string::npos)
				return true;
			if (s.find("(") != string::npos)
				return true;
			if (s.find(")") != string::npos)
				return true;
			if (s.find("`") != string::npos)
				return true;
			if (s.find("~") != string::npos)
				return true;
			if (s.find("/") != string::npos)
				return true;
			if (s.find("\\") != string::npos)
				return true;
			if (s.find("\"") != string::npos)
				return true;
			if (s.find("\'") != string::npos)
				return true;
			if (s.find("\?") != string::npos)
				return true;
			return false;
		}
		///* Helper Functions for Second Pass *///
		
		string parse_a_instruction (string line) {
			int value;
			if (line.find(" ") != string::npos)
				error("Parse A-instruction error at line " + to_string(line_counter) + ".");
			if(isalpha(line[0])) {  											///if symbol, look at symbol_table for value
				auto it = symbol_table.find(line);
				if (it == symbol_table.end())
					error("Error:  Could not find symbol at line " + to_string(line_counter) + ".");
				value = it->second;
			}
			else 																///if number, convert it to binary
				value = stoi(line);
			if (value > 32767 || value < 0)
				cerr<<"Error:  Value out of range at line "<<line_counter<<".\n";
			bitset<16> bin(value);
			return bin.to_string();
		}
		
		string parse_c_instruction (string line) {
			size_t equals = line.find('=');
			size_t semi = line.find(';');
			string dest = "000";
			string comp = line.substr(0, semi);
			string jump = "000";
			if (equals != string::npos) {										///if there is an equal sign, in line, find dest
				dest = get_dest(line, equals);
				comp = line.substr(equals+1);  									///comp is part after equal sign
			}
			if (semi != string::npos) {											///if semi colon is present, determine jump
				comp = comp.substr(0,comp.find(';'));							///comp is part before semi-colon
				jump = get_jump(line, semi);
			}
			comp.erase(remove(comp.begin(),comp.end(),' '),comp.end());
			pair<string, string> a_comp = get_comp_and_a(comp);
			return "111" + a_comp.first + a_comp.second + dest + jump;
		}
		///* Helper Functions for C-instruction *///
	
		string get_dest(string dest, size_t equals) {
			dest = dest.substr(0, equals);
			dest.erase(remove(dest.begin(),dest.end(),' '),dest.end());
			if (dest == "M")
				dest = "001";
			else if (dest == "D")
				dest = "010";
			else if (dest == "MD")
				dest = "011";
			else if (dest == "A")
				dest = "100";
			else if (dest == "AM")
				dest = "101";
			else if (dest == "AD")
				dest = "110";
			else if (dest == "AMD")
				dest = "111";
			else
				error("Error:  Parse dest error at line " + to_string(line_counter) + ".");
			return dest;
		}
	
		string get_jump (string jump, size_t semi) {
			jump = jump.substr(semi+1);												///jump is part after semi-colon
			jump.erase(remove(jump.begin(),jump.end(),' '),jump.end());
			if (jump == "")
				jump = "000";
			else if (jump == "JGT")
				jump = "001";
			else if (jump == "JEQ")
				jump = "010";
			else if (jump == "JGE")
				jump = "011";
			else if (jump == "JLT")
				jump = "100";
			else if (jump == "JNE")
				jump = "101";
			else if (jump == "JLE")
				jump = "110";
			else if (jump == "JMP")
				jump = "111";
			else
				error("Error:  Parse jump error at line " + to_string(line_counter) + ".");
			return jump;
		}
	
		pair<string, string> get_comp_and_a (string comp) {
			string a = "0";
			if (comp == "0")
				comp = "101010";
			else if (comp == "1")
				comp = "111111";
			else if (comp == "-1")
				comp = "111010";
			else if (comp == "D")
				comp = "001100";
			else if (comp == "A" || comp == "M") {
				if (comp == "M")
					a = "1";
				comp = "110000";
			}
			else if (comp == "!D")
				comp = "001101";
			else if (comp == "!A" || comp == "!M") {
				if (comp == "!M")
					a = "1";
				comp = "110001";
			}
			else if (comp == "-D")
				comp = "001111";
			else if (comp == "-A" || comp == "-M") {
				if (comp == "-M")
					a = "1";
				comp = "110011";
			}
			else if (comp == "D+1")
				comp = "011111";
			else if (comp == "A+1" || comp == "M+1" || comp == "1+A" || comp == "1+M") {
				if (comp == "M+1" || comp == "1+M")
					a = "1";
				comp = "110111";
			}
			else if (comp == "D-1")
				comp = "001110";
			else if (comp == "A-1" || comp == "M-1") {
				if (comp == "M-1")
					a = "1";
				comp = "110010";
			}	
			else if (comp == "D+A" || comp == "D+M" || comp == "A+D" || comp == "M+D") {
				if (comp == "D+M" || comp == "M+D")
					a = "1";
				comp = "000010";
			}
			else if (comp == "D-A" || comp == "D-M") {
				if (comp == "D-M")
					a = "1";
				comp = "010011";
			}
			else if (comp == "A-D" || comp == "M-D") {
				if (comp == "M-D")
					a = "1";
				comp = "000111";
			}
			else if (comp == "D&A" || comp == "D&M" || comp == "A&D" || comp == "M&D") {
				if (comp == "D&M" || comp == "M&D")
					a = "1";
				comp = "000000";
			}
			else if (comp == "D|A" || comp == "D|M" || comp == "A|D" || comp == "M|D") {
				if (comp == "D|M" || comp == "M|D")
					a = "1";
				comp = "010101";
			}
			else
				error("Error:  Parse comp error at line " + to_string(line_counter) + ".");
			pair<string, string> a_comp(a, comp);
			return a_comp;
		}
		///***********************************************************************
		
		// General Helper Functions *********************************************
		
		void error(string err) {
			cerr<<err<<endl;
			remove((file_name + "hack").c_str());					//Delete hack file if assembly fails
			exit(1);
		}
		//***********************************************************************
		
		/// Private Members *****************************************************
		
		string file_name;												///Stores file name for error function to delete
		size_t line_counter = 0;
		int symbol_counter = 16;
		symb_tab symbol_table = {
			{"SP", 0},		{"LCL", 1},			{"ARG", 2},		{"THIS", 3},
			{"THAT", 4},	{"SCREEN", 16384}, 	{"KBD", 24567}, {"R0", 0},
			{"R1", 1},		{"R2", 2}, 			{"R3", 3}, 		{"R4", 4},
			{"R5", 5}, 		{"R6", 6},			{"R7", 7},		{"R8", 8},
			{"R9", 9}, 		{"R10", 10},		{"R11", 11},	{"R12", 12},
			{"R13", 13},	{"R14", 14},		{"R15", 15}
		};
		vector<string> input_file;
		///***********************************************************************
};