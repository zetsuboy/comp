#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <iterator>
#include <sstream>

class Lexem {
public:
	int id;
	int line_id;
	std::string lex_type;
	std::string value;
	std::string default_value = "";
	bool error = false;
	std::string error_message = "";

	Lexem(int _id, int _line_id, std::string _lex_type, std::string _value) {
		id = _id;
		line_id = _line_id;
		lex_type = _lex_type;
		value = _value;
	}

	Lexem(int _id, int _line_id, bool _error, std::string _error_message) {
		id = _id;
		line_id = _line_id;
		error = _error;
		error_message = _error_message;
	}

	Lexem(int _id, int _line_id, std::string _lex_type, std::string _value, std::string _default_value) {
		id = _id;
		line_id = _line_id;
		lex_type = _lex_type;
		value = _value;
		default_value = _default_value;
	}
};

class SyntaxA {
private:
	std::vector<std::string> Words = { "program", "var", "integer", "real", "bool", "string", "begin", "end", "if", "then", "else", "while", "do", "true", "false", "uses"};
	std::vector<std::string> Delimiter = { ".", ";", ",", "(", ")" };
	std::vector<std::string> Operators = { "+", "-", "*", "/", "=", ">", "<" };
	std::vector<std::string> DoubleOperators = { ">=", "<=" };
	std::vector<char> HexadecimalChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	std::vector<char> DecimalChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	std::vector<char> OctalChars = { '0', '1', '2', '3', '4', '5', '6', '7'};
	std::vector<char> BinaryChars = { '0', '1'};
	std::map<char, int> CharsToInt = { {'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8},  {'9', 9}, {'a', 10}, {'b', 11},  {'c', 12},  {'d', 13},  {'e', 14},  {'f', 15}};

	std::string input = "";
	std::string buf = "";
	std::vector<char> current_system = DecimalChars;
	int current_base = 10;
	long long num_buf = 0, double_counter = 1;
	double double_buf = 0;
	int lex_id = 0;
	int line_id = 1;
	int line_skip_count = 0;
	int i = 0;
	enum States { S, NUM, DOUBLE, STR, OPER, DLM, FIN, ID, ER, ASGN, SINGLE_COM, MULTI_COM };
	enum Types {INTEGER, REAL, STRING, IDENTIFIER, WORDS, COMMENT, OPERATOR, DELIMITER, END};
	std::string TypesName[9] = { "Integer", "Real", "String", "Identifier", "Reserved word", "Comment", "Operator", "Delimiter", "End"};
	int state = S;

	std::pair<int, std::string> SearchLexem(std::vector<std::string> lexes, std::string buffer) {
		auto ptr = std::find(lexes.begin(), lexes.end(), buffer);
		if (ptr != lexes.end())
			return std::make_pair(std::distance(lexes.begin(), ptr), buffer);
		else
			return std::make_pair(-1, "");
	}

public:


	SyntaxA(char* path) {
		std::ifstream infile(path);
		if (infile) {
			std::stringstream str;
			str << infile.rdbuf();
			input = str.str();
			infile.close();
		}
	}

	Lexem operator++() {
		while (state != FIN && i < input.size()) {
			switch (state) {
			case S: {
				lex_id = i + 1;
				if (input[i] == ' ' || input[i] == '\t' || input[i] == '\0' || input[i] == '\r')
					i++;
				else if (input[i] == '\n') {
					line_id++;
					i++;
					line_skip_count = i;
				}
				else if (isalpha(input[i])) {
					buf = input[i];
					state = ID;
					i++;
				}
				else if (isdigit(input[i])) {
					buf = input[i];
					state = NUM;
					current_system = DecimalChars;
					current_base = 10;
					num_buf = (int)(input[i] - '0');
					i++;
				}
				else if (input[i] == '$') {
					state = NUM;
					buf = input[i];
					current_system = HexadecimalChars;
					current_base = 16;
					num_buf = 0;
					i++;
				}
				else if (input[i] == '&') {
					state = NUM;
					buf = input[i];
					current_system = OctalChars;
					current_base = 8;
					num_buf = 0;
					i++;
				}
				else if (input[i] == '%') {
					state = NUM;
					buf = input[i];
					current_system = BinaryChars;
					current_base = 2;
					num_buf = 0;
					i++;
				}
				else if (input[i] == '{') {
					state = MULTI_COM;
					i++;
				}
				else if (input[i] == '/' && input[i + 1] == '/') {
					state = SINGLE_COM;
					i += 2;
				}
				else if (input[i] == ':') {
					state = ASGN;
					buf = input[i];
					i++;
				}
				else if (input[i] == '.') {
					try {
						if (input[i + 1] == '.')
						{
							buf = "..";
							Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[OPERATOR], buf);
							i += 2;
							return *lex;
						}
					}
					catch(std::exception) { }
					state = FIN;
					buf = ".";
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[END], buf);
					return *lex;

				}
				else if (input[i] == '\'') {
					state = STR;
					i++;
					buf = "";
				}
				else {
					state = DLM;
				}
				break;
			}
			case ID: {
				if (isalnum(input[i]) || input[i] == '_') {
					buf += input[i];
					i++;
				}
				else {
					state = S;
					std::pair<int, std::string> p = SearchLexem(Words, buf);
					if (p.first == -1) {
						Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[IDENTIFIER], buf);
						return *lex;
					}
					else {
						Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[WORDS], buf);
						return *lex;
					}
				}
				break;
			}
			case NUM: {
				std::string sss(1, input[i]);
				if (isdigit(input[i])) {
					num_buf = num_buf * current_base + CharsToInt[input[i]];
					buf += input[i];
					if (num_buf > INT_MAX) {
						state = ER;
					}
					i++;
				}
				else if (input[i] == '.') {
					try {
						if (input[i + 1] == '.') {
							Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[INTEGER], std::to_string(num_buf), buf);
							state = S;
							return *lex;
						}
					}
					catch (std::exception) { }
					double_buf = num_buf;
					i++;
					state = DOUBLE;
					double_counter = 10;
				}
				else if (input[i] == ' ' || input[i] == '\n' || input[i] == ';' || input[i] == ',' || input[i] == ')') {
					state = S;
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[INTEGER], std::to_string(num_buf), buf);
					return *lex;
				}
				else if (SearchLexem(Operators, sss).first != -1) {
					state = OPER;
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[REAL], std::to_string(num_buf), buf);
					buf = sss;
					i++;
					lex_id = i;
					return *lex;
				}
				else {
					state = ER;
				}
				break;
			}
			case DOUBLE: {
				std::string sss(1, input[i]);
				std::cout << sss << std::endl;
				if (isdigit(input[i])) {
					double_buf = double_buf + (double)(input[i] - '0') / double_counter;
					double_counter *= 10;
					i++;
				}
				else if (SearchLexem(Operators, sss).first != -1) {
					state = OPER;
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[REAL], std::to_string(num_buf), buf);
					buf = sss;
					i++;
					lex_id = i;
					return *lex;
				}
				else if (input[i] == 'e') {
					i++;
					char sign;
					if (input[i] == '+')
						sign = '+';
					else if (input[i] == '-')
						sign = '-';
					else
					{
						state = ER;
						break;
					}
				}
				else if (input[i] == ' ' || input[i] == ';' || input[i] == ',' || input[i] == ')') {
					state = S;
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[REAL], std::to_string(double_buf));
					return *lex;
				}
				else if (input[i] == '\n') {
					state = S;
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[REAL], std::to_string(double_buf));
					return *lex;
				}
				else {
					state = ER;
				}
				break;
			}
			case STR: {
				while (input[i] != '\'') {
					buf += input[i];
					i++;
				}
				Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[STRING], buf);
				i++;
				state = S;
				return *lex;
			}
			case DLM: {
				state = S;
				buf = "";
				buf += input[i];
				i++;
				std::pair<int, std::string> p_del = SearchLexem(Delimiter, buf);
				std::pair<int, std::string> p_oper = SearchLexem(Operators, buf);
				if (p_del.first != -1) {
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[DELIMITER], buf);
					return *lex;
				}
				else if (p_oper.first != -1) {
					state = OPER;
				}
				else {
					state = ER;
				}
				break;
			}
			case OPER: {
				std::string old_buf = buf;
				buf += input[i];
				state = S;
				std::pair<int, std::string> p_oper = SearchLexem(DoubleOperators, buf);
				if (p_oper.first != -1) {
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[OPERATOR], buf);
					return *lex;
					i++;
				}
				else {
					buf = old_buf;
					Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[OPERATOR], buf);
					return *lex;
				}
				break;
			}
			case ASGN: {
				if (input[i] == '=') {
					buf = ":=";
					i++;
				}
				Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[OPERATOR], buf);
				state = S;
				return *lex;
				break;
			}
			case SINGLE_COM: {
				if (input[i] == '\n')
				{
					state = S;
					line_id++;
				}
				i++;
				break;
			}
			case MULTI_COM: {
				if (input[i] == '}')
					state = S;
				if (input[i] == '\n')
					line_id++;
				i++;
				break;
			}
			case ER: {
				state = FIN;
				Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, true, "Error at line " + std::to_string(line_id) + " symbol " + std::to_string(lex_id - line_skip_count));
				return *lex;
				
			}
			case FIN: {
				Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, TypesName[END], buf);
				return *lex;
			}
			}
		}
		if (i >= input.size()) {
			Lexem* lex = new Lexem(lex_id - line_skip_count, line_id, true, "Syntax error, '.' expected, but \"end of file\" found");
			return *lex;
		}
	}
};

int main(int argc, char* argv[]) {
	SyntaxA s(argv[1]);
	Lexem lex = ++s;
	while (lex.error != true && lex.lex_type != "End") {
		std::cout << lex.line_id << "	" << lex.id << "	" << lex.lex_type << "	" << lex.value << "	" << lex.default_value << std::endl;
		lex = ++s;
	}
	if (lex.error == true) {
		std::cout << lex.error_message << std::endl;
	}
	else {
		std::cout << lex.line_id << "	" << lex.id << "	" << lex.lex_type << "	" << lex.value << "	" << lex.default_value << std::endl;
	}
	//std::cout << lex.line_id << "	" << lex.id << "	" << lex.lex_type << "	" << lex.value << "	" << lex.default_value << std::endl;
	return 0;
}