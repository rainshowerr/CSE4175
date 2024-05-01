#include <iostream>
#include <fstream>
#include <cstring>
#include <bitset>
#include <string>

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
	ofstream result_file(argv[3]);

	// 예외처리
	if (argc != 6) {
		cout << "usage: ./crc_decoder input_file output_file result_file generator dataword_size" << endl;
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
	if (!result_file) {
		cout << "result file open error." << endl;
		return (1);
	}
	if (!strcmp(argv[5], "4") && !strcmp(argv[5], "8")) {
		cout << "dataword size must be 4 or 8." << endl;
		return (1);
	}

	// 1byte 패딩 정보 읽어들이기
	int padding_info;
	char c;
	input_file.get(c);
	bitset<8> bit_padding(c);
	padding_info = bit_padding.to_ulong();

	// 패딩 정보를 제외한 나머지를 2진수 string codeword로 변환
	string codeword;
	while(input_file.get(c)) {
		bitset<8> binary(c);
		codeword += binary.to_string();
	}

	// 패딩 제거
	codeword = codeword.substr(padding_info, codeword.length() - padding_info);

	// codeword size로 나눠서 각각의 codeword 검증 후 dataword 추출하여 resword에 저장
	int dsize = atoi(argv[5]);
	string generator = argv[4];
	int csize = dsize + generator.length() - 1;
	int codeword_num = 0, wrong_codeword_num = 0;
	string dataword;
	for(int i = 0; i < codeword.length(); i += csize) {
		string eachCodeword = codeword.substr(i, i + csize);
		string zeros(generator.length() - 1, '0');
		if (mod2(eachCodeword, generator) != zeros) // 오류가 난 codeword 개수 기록
			wrong_codeword_num++;
		dataword += eachCodeword.substr(0, dsize); // codeword를 dataword로 변환
		codeword_num++;
	}

	// dataword를 본래의 파일로 복원
	string decoded;
	for(int i = 0; i < dataword.length(); i += 8) {
		string eachDataword = dataword.substr(i, i + 8);
		bitset<8> bit_dataword(eachDataword);
		decoded += static_cast<char>(bit_dataword.to_ulong());
	}
	output_file.write(decoded.c_str(), decoded.length());
	result_file.write(to_string(codeword_num).c_str(), to_string(codeword_num).length());
	result_file.write(" ", 1);
	result_file.write(to_string(wrong_codeword_num).c_str(), to_string(wrong_codeword_num).length());
}