#include <iostream>
#include <fstream>
#include <cstring>
#include <bitset>

using namespace std;

// 이진수 문자열을 XOR 연산으로 계산
string xor_strings(string& current_portion, string& divisor) {
    string result;
    for (int i = 0; i < current_portion.length(); i++) {
        // XOR 연산
        result += (current_portion[i] == divisor[i]) ? '0' : '1';
    }
    return result;
}

// 모듈러-2 나눗셈
string mod2_division(string& dividend, string& divisor) {
    // 나눗셈을 위해 이진수 복사
    string remainder = dividend;
    int divisor_len = divisor.length();

    // 나머지에서 앞부분의 가장 왼쪽 비트가 '1'인 경우만 나눗셈 진행
    while (remainder.length() >= divisor_len) {
        // 나머지에서 앞부분 자르기
    	string current_portion = remainder.substr(0, divisor_len);

        // XOR 연산을 통해 나누기
        if (current_portion[0] == '1') { // 나누기 조건
            current_portion = xor_strings(current_portion, divisor);
        }

        // 나머지 업데이트
        remainder = current_portion.substr(1) + remainder.substr(divisor_len);
    }

    return remainder; // 나머지 반환
}


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
	// dataword가 8일때는 그냥 패딩을 붙이면 되지만 4일 때는 두 결과를 합친 후 앞에 패딩을 붙여야함
	string resword;
	for(int i = 0; i < binaryText.length(); i += dsize) {
		string dataword = binaryText.substr(i, dsize);
		string zeros(generator.length() - 1, '0');
		string tempword = dataword + zeros;
		string codeword = dataword + mod2_division(tempword, generator);
		if (dsize == 4) {
			if (i % 8 == 0) {
				resword = codeword;
			}
			else {
				resword += codeword;
				if (resword.length() % 8 != 0) {
					int padding_num = 8 - (resword.length() % 8);
					bitset<8> binary(padding_num);
					string zeros(padding_num, '0');
					resword = binary.to_string() + zeros + resword;
				}
				bitset<8> binary(resword);
				string resstr = reinterpret_cast<char*>(binary.to_ulong());
				output_file.write(resstr.c_str(), resstr.length());
				//output_file.write(resword.c_str(), resword.length());
			}
		}
		else {
			resword = codeword;
			if (resword.length() % 8 != 0) {
					int padding_num = 8 - (resword.length() % 8);
					bitset<8> binary(padding_num);
					string zeros(padding_num, '0');
					resword = binary.to_string() + zeros + resword;
				}
			output_file.write(resword.c_str(), resword.length());
		}
	}
}