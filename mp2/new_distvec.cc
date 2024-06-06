#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <queue>
using namespace std;

int n; // 전체 라우터의 개수
pair<int, int> route[100][100]; // 라우팅 테이블 <cost, next>
int link[100][100]; // i와 j 테이블이 연결되어 있으면 cost, 아니면 0 저장

void print(ofstream& outputfile) {
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			if (route[i][j].first != 10000) {
				outputfile << j << ' ';
				if (i == j) outputfile << i << ' ';
				else outputfile << route[i][j].second << ' ';
				outputfile << route[i][j].first << endl;
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
			if (i == j) route[i][j].first = 0; // 자기 자신으로 가는 cost는 0
			else route[i][j].first = 10000; // 경로가 없으면 INF
			route[i][j].second = -1; // next는 -1로 초기화
		}
	}
}

void bellman_ford(int start) {
	for(int i = 0; i < n - 1; i++) {  // n-1번 반복
		// 매 반복마다 모든 간선 확인
		for(int pass = 0; pass < n; pass++) {
			for(int dest = 0; dest < n; dest++) {
				if (link[pass][dest] && route[start][pass].first != 10000 && (route[start][dest].first >= route[start][pass].first + link[pass][dest])) {
					route[start][dest].first = route[start][pass].first + link[pass][dest];
				}
			}
		}
	}
}

// 라우팅테이블의 next 정보를 설정해주는 함수
void find_next() {
	for(int src = 0; src < n; src++) {
		for(int neighbor = n - 1; neighbor >= 0; neighbor--) { // tie-breaking rule 1: 현재 루트와 새로운 루트의 cost가 같으면 id가 더 작은 노드를 다음 노드로 삼는다.
			for(int dst = 0; dst < n; dst++) {
				if (link[src][neighbor] && route[src][dst].first == link[src][neighbor] + route[neighbor][dst].first)
					route[src][dst].second = neighbor;
			}
		}
	}
}

void print_hops(ofstream& outputfile, int src, int dst) {
	outputfile << src << ' ';
	if (route[src][dst].second != dst)
		print_hops(outputfile, route[src][dst].second, dst);
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
		if (route[sender][receiver].first >= 10000)
			outputfile << "infinite hops unreachable message " << message << endl;

		// 경로가 있는 경우
		else {
			outputfile << route[sender][receiver].first << " hops ";
			print_hops(outputfile, sender, receiver);
			outputfile << "message " << message << endl;
		}
	}
	outputfile << endl;
	messagesfile.clear();
	messagesfile.seekg(0, ios::beg);
}

// 이웃 라우터에 업데이트 사실을 알리고 최단경로를 갱신하기 위해 초기화하는 함수
void init_changed_route(int src, int dst) {
	for(int i = 0; i < n; i++) {
		if (link[src][i]) {
			for(int j = 0; j < n; j++) {
				if (route[i][j].second == src && j == dst) { // src를 통해 dst로 가는 경로일 경우 INF로 초기화
					route[i][j].first = 10000;
					route[i][j].second = -1;
					init_changed_route(i, dst);
				}
			}
		}
	}
}

void change_route(ifstream& topologyfile, ifstream& changesfile, ifstream& messagesfile, ofstream& outputfile) {
	string change; int src, dst, cost;
	while (getline(changesfile, change)) {
		istringstream iss(change);
		iss >> src >> dst >> cost;
	
		// cost가 작아지는 경우 (새로운 link가 추가되는 경우 + 기존 link의 cost가 작아지는 경우)
		if ((!link[src][dst]) || (cost != -999 && cost < link[src][dst])) {
			link[src][dst] = cost;
			link[dst][src] = cost;
		}
		// cost가 커지는 경우 (기존 link가 끊어지는 경우 + 기존 link의 cost가 커지는 경우
		else {
			init_changed_route(src, dst);
			init_changed_route(dst,src);
			if (cost == -999) { //기존 link가 끊어지는 경우
				link[src][dst] = 0;
				link[dst][src] = 0;
			}
			else { // 기존 link의 cost가 커지는 경우
				link[src][dst] = cost;
				link[dst][src] = cost;
			}
			for(int i = 0; i < n; i++) {
				if (route[src][i].second == dst) { // src가 시작지, i가 목적지, second가 다음. 다음이 dst인 경우를 INF로 초기화해주고 DFS
					route[src][i].first = 10000;
					route[src][i].second = -1;
					init_changed_route(src, i);
				}
			}
			for(int i = 0; i < n; i++) {
				if (route[dst][i].second == src) {
					route[dst][i].first = 10000;
					route[dst][i].second = -1;
					init_changed_route(dst, i);
				}
			}
		}
		for(int i = 0; i < n; i++)
				bellman_ford(i);
			find_next();
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
	find_next();

	// 초기 라우팅테이블 출력 및 simulate
	ofstream outputfile("output_dv.txt");
	print(outputfile);
	simulate(messagesfile, outputfile);

	// change 이후 라우팅테이블 출력 및 simulate
	change_route(topologyfile, changesfile, messagesfile, outputfile);

	cout << "Complete. Output file written to output_dv.txt." << endl;
}