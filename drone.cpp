/**
 * Google HashCode 2016
 * Team: Havana Blues
 * Swiss Federal Institute of Technology in Lausanne (EPFL)
 * Aimee Montero
 * Alex Vouilloz
 * Alfonso2 Peterssen
 * 11-02-2016
 */ 
#include <bits/stdc++.h>
using namespace std;

#define ALL(c) (c).begin(), (c).end()

typedef long long int64;

typedef pair<int, int> par;

const int
	MAXD = 1000,
	MAXCols = 10000,
	MAXP = 10000,
	MAXW = 20,
	MAXC = 1300;
	
const double INF = 1e20;

int Rows, Cols;
int D; // drone count
int deadline;
int maxLoad;
int P; // product count
int pWeight[MAXP];

int W; // warehouses
int wRow[MAXW];
int wCol[MAXW];

short wProd[MAXW][MAXP];

int C; // customer orders
int orderRow[MAXC];
int orderCol[MAXC];
int orderSize[MAXC];
vector<int> productsFor[MAXC];

map<int, int> prodswc[MAXW][MAXC];

void readInput() {
	cin >> Rows >> Cols;
	cin >> D;
	cin >> deadline;
	cin >> maxLoad;
	cin >> P;
	for (int i = 0; i < P; ++i)
		cin >> pWeight[i];
		
	cin >> W;
	for (int i = 0; i < W; ++i) {
		cin >> wRow[i] >> wCol[i];
		for (int j = 0; j < P; ++j)
			cin >> wProd[i][j];
	}
	
	cin >> C;
	for (int i = 0; i < C; ++i) {
		cin >> orderRow[i] >> orderCol[i];
		cin >> orderSize[i];
				
		productsFor[i].reserve(orderSize[i]);
				
		for (int j = 0; j < orderSize[i]; ++j) {
			int p;
			cin >> p;
			productsFor[i].push_back(p);
		}
		
		sort(ALL(productsFor[i]));
	}
}

struct drone {
	int r, c;
	int finishTime;
};

drone drones[MAXD];

int dist2w[MAXW];
double costw2c[MAXW][MAXC];

int commands;
double totalScore;
stringstream output;

inline int sqr(int x) { return x * x; }

int steps(int sr, int sc, int tr, int tc) {
	return (int)ceil(sqrt(sqr(sr - tr) + sqr(sc - tc)));
}

bool byReverseWeight(int i, int j) {
	if (pWeight[i] != pWeight[j])
		return pWeight[i] < pWeight[j];
	return i < j;
}

void assignTask(int droneId, int now) {

	for (int w = 0; w < W; ++w) {
		dist2w[w] =
			steps(
				drones[droneId].r, drones[droneId].c,
				wRow[w], wCol[w]);
	}
	
	for (int w = 0; w < W; ++w)
	for (int c = 0; c < C; ++c) {
		costw2c[w][c] = steps(
				wRow[w], wCol[w],
				orderRow[c], orderCol[c]);
		
		int currentLoad = 0;
		int totalTaken = 0;
		
		int totalSteps = dist2w[w] + costw2c[w][c];				
		
		map<int, int> takenProd;
		
		sort(ALL(productsFor[c]), byReverseWeight);		
		
		for (int p : productsFor[c]) {
			// amount of product p in warehouse w
			int t = wProd[w][p];
			if (takenProd.count(p))
				t -= takenProd[p];
				
			if (t > 0 && currentLoad + pWeight[p] <= maxLoad) {
				++takenProd[p];
				++totalTaken;
				currentLoad += pWeight[p];
			}
		}

		totalSteps += 2 * takenProd.size();
		
		double score = +INF;
		if (now + totalSteps < deadline)
		if (totalTaken > 0) {			
			score = 5 * pow(productsFor[c].size() - totalTaken, 1.3); // FIX
			// complete fast
			if (totalTaken == (int)productsFor[c].size()) {
				score = 0;
			}
		}
		
		prodswc[w][c] = takenProd;
		costw2c[w][c] += score;
	}
	
	double bestCost = INF;
	par bestwc = {-1, -1};
	
	for (int w = 0; w < W; ++w)
	for (int c = 0; c < C; ++c) {
		double cost =
			dist2w[w] // drone to warehouse
			+ costw2c[w][c];
			
		if (cost < bestCost) {
			bestCost = cost;
			bestwc = {w, c};
		}
	}
	
	if (bestwc.first < 0) {
		drones[droneId].finishTime = -1;
		return ;
	}
	
	int currentTime = now;
	
	int w = bestwc.first;
	int c = bestwc.second;
	
	currentTime += dist2w[w];	
	
	for (par prodCant : prodswc[w][c]) { // {prod, cantTaken}
		
		int p = prodCant.first;
		int cant = prodCant.second;
		
		// erase from warehouse
		wProd[w][p] -= cant;
		assert(wProd[w][p] >= 0);				
		
		// erase from customer order
		for (int i = 0; i < cant; ++i)
			productsFor[c].erase(find(ALL(productsFor[c]), p));
			
		// load all products of type p
		output << droneId << " " << "L" << " " << w << " " << p << " " << cant << endl;
		commands++;
		
		currentTime++;
	}
	
	currentTime += steps(wRow[w], wCol[w], orderRow[c], orderCol[c]);
	
	for (par prodCant : prodswc[w][c]) { // {prod, cantTaken}		
		int p = prodCant.first;
		int cant = prodCant.second;		
		
		// deliver all products of type p
		output << droneId << " " << "D" << " " << c << " " << p << " " << cant << endl;
		commands++;
		currentTime++;
	}
	
	if (productsFor[c].empty()) {
		totalScore += (deadline - currentTime) * 1.0 / deadline * 100;
	}
	
	drones[droneId].finishTime = currentTime;
	drones[droneId].r = orderRow[c];
	drones[droneId].c = orderCol[c];
}

int order[MAXD];

int main() {
	ios_base::sync_with_stdio(0);
	
	readInput();
	for (int i = 0; i < D; ++i) {
		drones[i].r = wRow[0];
		drones[i].c = wCol[0];
	}
	
	for (int t = 0; t < deadline; ++t)
		for (int i = 0; i < D; ++i)
			if (drones[i].finishTime == t)
				assignTask(i, t);
	
	cout << commands << endl;
	cout << output.str();	
	
	cerr << totalScore << endl;

	return 0;
}
