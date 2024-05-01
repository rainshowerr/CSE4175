#include <iostream>
#include <fstream>
#include <cstring>
#include <bitset>

using namespace std;

// 이진수 문자열을 XOR 연산으로 계산
string xor_string(string& current, string& divisor) {
    string result;
    for (int i = 0; i < current.length(); i++) {
        // XOR 연산
        result += (current[i] == divisor[i]) ? '0' : '1';
    }
    return result;
}

// 모듈러-2 나눗셈
string mod2(string& dividend, string& divisor) {
	// remaindero에 dividend 할당하고 시작
    string remainder = dividend;

    // divisor_len보다 remainder가 작아지면 루프 탈출
    while (remainder.length() >= divisor.length()) {
        // remainder에서 앞부분 자르기
    	string current = remainder.substr(0, divisor.length());

        // 1 X X X일 경우에만 나눗셈 진행
        if (current[0] == '1') {
            current = xor_string(current, divisor);
        }

        // 나머지 업데이트
        remainder = current.substr(1) + remainder.substr(divisor.length());
    }

    return remainder; // 나머지 반환
}

int main(int argc, char *argv[]) {
	ifstream input_file(argv[1]);
	ofstream output_file(argv[2]);

	// 예외처리
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

	// input_file의 text를 2진수 string dataword으로 변환
	string dataword;
	char c;
	while(input_file.get(c)) {
		bitset<8> binary(c);
		dataword += binary.to_string();
	}

	// 2진수 string dataword를 2진수 string codeword로 변환
	string generator = argv[3];
	int dsize = atoi(argv[4]);
	string codeword;
	for(int i = 0; i < dataword.length(); i += dsize) {
		string eachDataword = dataword.substr(i, dsize);
		string zeros(generator.length() - 1, '0');
		string tempword = eachDataword + zeros; // 각각의 dataword 뒤에 0들을 붙여서 모듈러연산 준비
		string eachCodeword = eachDataword + mod2(tempword, generator); // 모듈러연산으로 codeword 구하기
		codeword += eachCodeword;
	}

	// codeword가 8로 나누어 떨어지게끔 codeword 앞에 1byte짜리 패딩 정보와 패딩 비트를 추가함
	int padding_num;
	padding_num = !(codeword.length() % 8) ? 0 : 8 - (codeword.length() % 8); // codeword가 8로 나누어떨어지면 패딩을 추가하지 않음
	bitset<8> binary(padding_num);
	string zeros(padding_num, '0');
	codeword = binary.to_string() + zeros + codeword;

	//2진수 string codeword를 8글자당 1byte로 압축
	string encoded;
	for(int i = 0; i < codeword.length(); i += 8) {
		bitset<8> bits(codeword.substr(i, i + 8));
		char c = static_cast<char>(bits.to_ulong());
		encoded += c;
	}

	// write
	output_file.write(encoded.c_str(), encoded.length());
}