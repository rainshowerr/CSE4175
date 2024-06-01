#include <iostream>
#include <fstream>
#include <queue>
#include <sstream>
#include <algorithm>
#include <string.h>
using namespace std;

int n;
pair<vector<int>, int> route[100][100];
int link[100][100]; // i와 j 테이블이 연결되어 있으면 cost, 아니면 0 저장

// tie-breaking rule 2
struct Compare {
    bool operator()(pair<int, int>& a, pair<int, int>& b) {
        if (a.first == b.first) {
            return a.second > b.second;
        }
        return a.first < b.first;
    }
};

priority_queue<pair<int, int>, vector<pair<int, int> >, Compare> pq[100];

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

void dijkstra(int start) {
	pq[start].push(make_pair(0, start)); // 출발 노드
	while(!pq[start].empty()) {
		int curr = pq[start].top().second; // 거쳐가는 노드
		int cost = pq[start].top().first;
		pq[start].pop();
		// if(route[start][curr] < cost) continue;
		for (int i = 0; i < n; i++) {
			if (link[curr][i]) {
				int n_curr = i; // 도착 노드
				int n_cost = link[curr][i]; // 도착 노드까지 한번에 가는 cost
				// tie-breaking rule 3 : cost가 같은 경우 parent 번호가 더 작은 경로를 선택
				if ((route[start][n_curr].second == cost + n_cost) && (start > curr)) {
					// hops 업데이트
					route[start][n_curr].first.clear(); // hops 초기화
					route[start][n_curr].first.insert(route[start][n_curr].first.end(), route[start][curr].first.begin(), route[start][curr].first.end()); // 거쳐가는 노드까지의 hops 복사
					route[start][n_curr].first.push_back(n_curr); // 거쳐가는 노드에서부터의 hop 추가
				}
				else if (route[start][n_curr].second > cost + n_cost) {
					// hops 업데이트
					route[start][n_curr].first.clear(); // hops 초기화
					route[start][n_curr].first.insert(route[start][n_curr].first.end(), route[start][curr].first.begin(), route[start][curr].first.end()); // 거쳐가는 노드까지의 hops 복사
					route[start][n_curr].first.push_back(n_curr); // 거쳐가는 노드에서부터의 hop 추가
					// cost 업데이트
					route[start][n_curr].second = cost + n_cost;
					pq[start].push(make_pair(cost + n_cost, n_curr));
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

void change_route(ifstream& topologyfile, ifstream& changesfile, ifstream& messagesfile, ofstream& outputfile) {
	string change; int start, end, cost;
	while (getline(changesfile, change)) {
		istringstream iss(change);
		iss >> start >> end >> cost;

		initialize_route_table();

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

		for(int i = 0; i < n; i++)
			dijkstra(i);
		print(outputfile);
		simulate(messagesfile, outputfile);
	}
}

int main(int ac, char* av[]) {
	// 예외처리
	if (ac != 4) {
		cout << "usage: linkstate topologyfile messagesfile changesfile" << endl;
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

	// 라우팅테이블 초기화
	for(int i = 0; i < n; i++)
		dijkstra(i);

	ofstream outputfile("output_ls.txt");
	// 초기 라우팅테이블 출력 및 simulate
	print(outputfile);
	simulate(messagesfile, outputfile);
	// change 이후 라우팅테이블 출력 및 simulate
	change_route(topologyfile, changesfile, messagesfile, outputfile);

	cout << "Complete. Output file written to output_ls.txt." << endl;
}