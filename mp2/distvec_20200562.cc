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

// 라우터의 라우팅 테이블에 start -> end 경로가 있으면 해당 경로가 들어있는 루트의 cost를 무한으로 설정하고 1 반환, 없으면 0 반환
int detect_path_and_delete(int node, int start, int end) {
	for(int i = 0; i < n; i++) {
		for(int j = 0; j + 1 < (int)route[node][i].first.size(); j++) {
			if ((route[node][i].first[j] == start && route[node][i].first[j + 1] == end)
			|| (route[node][i].first[j] == end && route[node][i].first[j + 1] == start)) {
				vector<int> v;
				route[node][i].first.clear();
				route[node][i].first = v;
				route[node][i].second = 10000;
				return (1);
			}
		}
	}
	return (0);
}


// 라우팅테이블 업데이트가 감지되면 이웃 라우터들에게 업데이트 소식을 알리고 필요하다면 라우팅테이블을 수정하는 함수
void signal_change(int node, int start, int end) {
	// 현재 노드 검사
	if (detect_path_and_delete(node, start, end)) {
		for(int i = 0; i < n; i++) { // 주위 노드도 검사하기 위해 재귀호출
			if(link[node][i]) {
				signal_change(i, start, end);
			}
		}
	}
}

void change_route(ifstream& topologyfile, ifstream& changesfile, ifstream& messagesfile, ofstream& outputfile) {
	string change; int start, end, cost;
	while (getline(changesfile, change)) {
		istringstream iss(change);
		iss >> start >> end >> cost;

		/* cost가 기존보다 커지거나 링크가 끊어지는 경우(cost가 무한대가 되는 경우)
		최단경로 알고리즘을 적용해도 업데이트되지 않으므로
		기존 경로를 거치는 루트의 cost를 무한대로 만들어줘야 함
		*/

		// cost가 기존보다 작아지는 경우
		if (cost != -999 && cost <= link[start][end]) {
			link[start][end] = cost;
			link[end][start] = cost;
			// 별도 라우팅테이블 업데이트 불필요
		}
		// cost가 기존보다 커지는 경우
		else if (cost != - 999 && cost > link[start][end]) {
			link[start][end] = cost;
			link[end][start] = cost;
			signal_change(start, start, end);
			signal_change(end, start, end);
		}
		// 링크가 끊어지는 경우
		else {
			signal_change(start, start, end);
			// 링크가 끊어지는 경우 해당 경로를 거치는 라우터의 루트를 전부 업데이트해준 뒤 링크를 끊어야 함
			link[start][end] = 0;
			link[end][start] = 0;
		}

		/* 기존 경로를 거치는 루트의 cost를 무한대로 만들어준 후
		본격적으로 최단경로 알고리즘을 적용해서 라우팅 테이블 수정 */
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