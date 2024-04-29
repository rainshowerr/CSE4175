#include <iostream>
#include <fstream>
#include <cstring>
#include <bitset>

using namespace std;

int main(int argc, char *argv[]) {
	ifstream input_file(argv[1]);
	ofstream output_file(argv[2]);

	if (argc != 5) {
		cout << "usage: ./crc_encoder input_file output_file generator dataword_size" << endl;
		return (1);
	}
	if (!input_file) {
		cout << "input file open error." << endl;
		return (1);
	}
	if (!output_file) {
		cout << "output file open error." << endl;
		return (1);
	}
	if (!strcmp(argv[4], "4") && !strcmp(argv[4], "8")) {
		cout << "dataword size must be 4 or 8." << endl;
		return (1);
	}

	string binaryText;
	char c;
	while(input_file.get(c)) {
		bitset<8> binary(c);
		binaryText += binary.to_string();
	}
	string generator = argv[3];
	int dsize = atoi(argv[4]);
	for(int i = 0; i < binaryText.length(); i += dsize) {
		string dataword = binaryText.substr(i, dsize);
		string zeros(generator.length() - 1, '0');
		dataword += zeros;
		
	}
}