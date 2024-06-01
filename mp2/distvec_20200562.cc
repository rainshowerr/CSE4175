#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
using namespace std;

int n; // 전체 라우터의 개수
pair<vector<int>, int> route[100][100]; // 라우팅 테이블 <hops, cost>
int link[100][100]; // i와 j 테이블이 연결되어 있으면 cost, 아니면 0 저장

void print(ofstream& outputfile) {
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			if (route[i][j].second != 10000) {
				outputfile << j << ' ';
				if (i == j)
					outputfile << route[i][j].first.front() << ' ';
				else
					outputfile << route[i][j].first[1] << ' ';
				outputfile << route[i][j].second << endl;
			}
		}
		outputfile << endl;
	}
}

void initialize_link_table(ifstream& topologyfile) {
	// edge 정보 저장
	topologyfile >> n;
	string topology;
	getline(topologyfile, topology); // 개행 읽어줘서 이상한 값 들어오는 것 방지
	memset(link, 0, sizeof(link));
	while (getline(topologyfile, topology)) {
		istringstream iss(topology);
		int start, end, cost;
		iss >> start >> end >> cost;
		link[start][end] = cost;
		link[end][start] = cost;
	}
}

void initialize_route_table() {
	// 최단거리 테이블 초기화
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			vector<int> v;
			if (i == j) {
				v.push_back(i);
				route[i][j].first = v;
				route[i][j].second = 0;
			}
			else {
				route[i][j].first = v;
				route[i][j].second = 10000;
			}
		}
	}
}

void bellman_ford(int start) {
	for(int k = 0; k < n - 1; k++) {
		// 매 반복마다 모든 간선 확인
		for(int i = 0; i < n; i++){
			for(int j = 0; j < n; j++){
				if (link[i][j]) {
					int curr = i, next = j, cost = link[i][j];
					// tie-breaking rule 1
					if ((route[start][curr].second != 10000 && route[start][next].second == route[start][curr].second + cost) && curr < next) {
						// hops 업데이트
						route[start][next].first.clear();
						route[start][next].first.insert(route[start][next].first.begin(), route[start][curr].first.begin(), route[start][curr].first.end());
						route[start][next].first.push_back(next);
					}
					else if(route[start][curr].second != 10000 && route[start][next].second > route[start][curr].second + cost){
						// hops 업데이트
						route[start][next].first.clear();
						route[start][next].first.insert(route[start][next].first.begin(), route[start][curr].first.begin(), route[start][curr].first.end());
						route[start][next].first.push_back(next);
						// cost 업데이트
						route[start][next].second = route[start][curr].second + cost;
					}
				}
			}
		}
	}
}

void simulate(ifstream& messagesfile, ofstream& outputfile) {
	string line;
	while(getline(messagesfile, line)) {
		// 문자열 파싱
		istringstream iss(line);
		int sender, receiver; string message;
		iss >> sender >> receiver;
		iss.ignore();
		getline(iss, message);

		// 출력
		outputfile << "from " << sender << " to " << receiver << " cost ";
		// 경로가 없는 경우
		if (route[sender][receiver].second >= 10000)
			outputfile << "infinite hops unreachable message " << message << endl;

		// 경로가 있는 경우
		else {
			outputfile << route[sender][receiver].second << " hops ";
			for(int i = 0; i < (int)route[sender][receiver].first.size() - 1; i++)
				outputfile << route[sender][receiver].first[i] << ' ';
			outputfile << "message " << message << endl;
		}
	}
	outputfile << endl;
	messagesfile.clear();
	messagesfile.seekg(0, ios::beg);
}

void initialize_rtable(int node) {
	// 변경된 간선의 이웃 노드부터 라우팅 테이블 업데이트
	for(int i = 0; i < n; i++) {
		vector<int> v;
		route[node][i].first.clear();
		if (i == node) {
			v.push_back(i);
			route[node][i].first = v;
			route[node][i].second = 0;
		}
		else {
			route[node][i].first = v;
			route[node][i].second = 10000;
			route[i][node].first = v;
			route[i][node].second = 10000;
		}
	}
}

int detect_path(int i, int j, int start, int end) {
	for(int k = 0; k + 1 < n; k++) {
		if (route[i][j].first[k] == start && route[i][j].first[k + 1] == end)
			return (1);
	}
	return (0);
}

void change_route(ifstream& topologyfile, ifstream& changesfile, ifstream& messagesfile, ofstream& outputfile) {
	string change; int start, end, cost;
	while (getline(changesfile, change)) {
		istringstream iss(change);
		iss >> start >> end >> cost;

		// 링크가 끊어지는 경우
		if (cost == -999) {
			link[start][end] = 0;
			link[end][start] = 0;
		}
		// 링크가 새로 생기거나 cost가 수정되는 경우
		else {
			link[start][end] = cost;
			link[end][start] = cost;
		}

		bellman_ford(start);
		bellman_ford(end);

		// 변경된 간선을 지나는 경우 관련 노드의 라우팅 테이블 업데이트
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) {
				if (route[i][j].second && (detect_path(i, j, start, end) || detect_path(i, j, end, start))) {
					vector<int> v;
					if (i == j) {
						v.push_back(i);
						route[i][j].first.clear();
						route[i][j].first = v;
					}
					route[i][j].first.clear();
					route[i][j].first = v;
					route[i][j].second = 10000;
				}
			}
		}

		// 전체 라우팅 테이블 업데이트
		for(int i = 0; i < n; i++)
			bellman_ford(i);

		print(outputfile);
		simulate(messagesfile, outputfile);
	}
}


int main(int ac, char* av[]) {
	// 입력 및 예외처리
	if (ac != 4) {
		cout << "usage: distvec topologyfile messagesfile changesfile" << endl;
		return (1);
	}
	ifstream topologyfile(av[1]), messagesfile(av[2]), changesfile(av[3]);
	if (!(topologyfile && messagesfile && changesfile)) {
		cout << "Error: open input file." << endl;
		return (1);
	}
	
	// 입력 및 초기화
	initialize_link_table(topologyfile);
	initialize_route_table();

	// 이웃과 라우팅 테이블 교환
	for(int i = 0; i < n; i++)
		bellman_ford(i);

	ofstream outputfile("output_dv.txt");
	// 초기 라우팅테이블 출력 및 simulate
	print(outputfile);
	simulate(messagesfile, outputfile);
	// change 이후 라우팅테이블 출력 및 simulate
	change_route(topologyfile, changesfile, messagesfile, outputfile);

	cout << "Complete. Output file written to output_dv.txt." << endl;
}