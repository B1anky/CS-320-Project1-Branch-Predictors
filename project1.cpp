//Name: Brett Sackstein
//BU-ID: bsackst1
//B-Number: 00458114
#include <iostream>
#include <fstream>
#include <string>
#include  <vector>

using namespace std;

struct tableEntry{
	unsigned long long address;
	int taken;
    	tableEntry(unsigned long long addr, int tak): address(addr), taken(tak){}
    	tableEntry(){}
};

vector<tableEntry> parseInput(string file, bool singleBit);
vector<int> alwaysTakenOrNotTaken(vector<tableEntry> table, bool takenIn);
vector<int> bimodalPredictorSingleBit(vector<tableEntry> table, unsigned int size);
vector<int> bimodalPredictorTwoBits(vector<tableEntry> table, unsigned int size);
vector<int> GsharePredictor(vector<tableEntry> table, unsigned int size);
vector<int> tournamentPredictor(vector<tableEntry> table);
string shiftHistoryBitsLeft(string history, string postFix = "0");
unsigned long long historyToInt(string history);
void printResults(vector<vector<vector<int>>> resultVec, const char *argv);

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cout << "Usage: ./predictors [input trace] [output file]\n";
		return 1; //error
	}

	vector<tableEntry> traceWithOneBit = parseInput(argv[1], true);
	vector<tableEntry> traceWithTwoBits = parseInput(argv[1], false);
	vector<vector<int>> resultVec1;
	vector<vector<int>> resultVec2;
	vector<vector<int>> resultVec3;
	vector<vector<int>> resultVec4;
	vector<vector<int>> resultVec5;
	vector<vector<int>> resultVec6;
	vector<vector<vector<int>>> finalResult;
	
	//Part 1
	resultVec1.push_back(alwaysTakenOrNotTaken(traceWithOneBit, true));
	//Part 2
	resultVec2.push_back(alwaysTakenOrNotTaken(traceWithOneBit, false));
	//Part 3
	int sizes[7] = {16, 32, 128, 256, 512, 1024, 2048};
	for(int i = 0; i < 7; i++){
		resultVec3.push_back(bimodalPredictorSingleBit(traceWithOneBit, sizes[i]));
	}
	//Part 4
	for(int i = 0; i < 7; i++){
		resultVec4.push_back(bimodalPredictorTwoBits(traceWithTwoBits, sizes[i]));
	}	
	//Part 5
	for(int i = 3; i < 12 ; i++){
		resultVec5.push_back(GsharePredictor(traceWithTwoBits, i));
	}
	//Part 6
	resultVec6.push_back(tournamentPredictor(traceWithTwoBits));
	
	finalResult.push_back(resultVec1);
	finalResult.push_back(resultVec2);
	finalResult.push_back(resultVec3);
	finalResult.push_back(resultVec4);
	finalResult.push_back(resultVec5);
	finalResult.push_back(resultVec6);

	printResults(finalResult, argv[2]);
	
	return 0;
}

//Builds a vector of all table entries
vector<tableEntry> parseInput(string file, bool singleBit){
	std::vector<struct tableEntry> table;
	unsigned long long addr;
	string behavior;
	ifstream infile(file);
	while(infile >> std::hex >> addr >> behavior) {
		struct tableEntry entry;
		entry.address = addr;
		if(behavior == "T") {
			entry.taken = 1;
		}else{
			if(singleBit){
				entry.taken = 0;	
			}else{
				entry.taken = -1;
			}
			
		}
		table.push_back(entry);
	}
	return table;
}

//Either taken all or not taken all
vector<int> alwaysTakenOrNotTaken(vector<tableEntry> table, bool takenIn){ 
	//Assume taken to be true or false for all addresses
	int accuracyCnt = 0;
	for(auto iter = table.begin(); iter < table.end(); ++iter){
		if(takenIn == true){
			if(iter->taken) accuracyCnt++;
		}

		if(takenIn == false){
			if(iter->taken == false) accuracyCnt++;
		}
	}
	vector<int> resultVec;
	resultVec.push_back(accuracyCnt);
	resultVec.push_back(table.size());
	return resultVec;
}

vector<int> bimodalPredictorSingleBit(vector<tableEntry> table, unsigned int size){
	//In bimodal single bit we track either taken or not taken and switch bit when wrong
	int accuracyCnt = 0;
	vector<int> fixedTable(size, 1);
  	
	//Now we're iterating through the hashed values 
	for(auto iter = table.begin(); iter < table.end(); ++iter){
		bool pathTaken = iter->taken == 1;
		int index = iter->address % size;
		int & bimodalPath = fixedTable[index];
		bool bimodalCorrect = (bimodalPath > 0) == pathTaken;

		//updtae predictor
		if(bimodalCorrect){
			accuracyCnt++;
		}else if(!bimodalCorrect){
			if(bimodalPath == 1){
				bimodalPath = -1;
			}else if(bimodalPath == -1){
				bimodalPath = 1;
			}
		}
	}

	vector<int> resultVec;
	resultVec.push_back(accuracyCnt);
	resultVec.push_back(table.size());
	return resultVec;
}

vector<int> bimodalPredictorTwoBits(vector<tableEntry> table, unsigned int size){
	//In bimodal single bit we track either taken or not taken and switch bit when wrong
	int accuracyCnt = 0;
	vector<int> fixedTable(size, 2);
  	
	//Now we're iterating through the hashed values 
	for(auto iter = table.begin(); iter < table.end(); ++iter){
		bool pathTaken = iter->taken == 1;
		int index = iter->address % size;
		int & bimodalPath = fixedTable[index];
		bool bimodalCorrect = (bimodalPath > 0) == pathTaken;

		//updtae predictor
		if(bimodalCorrect){
			accuracyCnt++;
			if(bimodalPath == 1){
				bimodalPath = 2;
			}else if(bimodalPath == -1){
				bimodalPath = -2;
			}
		}else if(!bimodalCorrect){
			if(bimodalPath == 1 || bimodalPath == -2){
				bimodalPath = -1;
			}else if(bimodalPath == -1 || bimodalPath == 2){
				bimodalPath = 1;
			}
		}
	}

	vector<int> resultVec;
	resultVec.push_back(accuracyCnt);
	resultVec.push_back(table.size());
	return resultVec;
}

vector<int> GsharePredictor(vector<tableEntry> table, unsigned int historyBitCnt){
	string history;
	//initialize all bits as NT
	for(unsigned int i = 0; i < historyBitCnt; i++){
		history.append("0");
	}

	int accuracyCnt = 0;
	vector<int> fixedTable(2048, 2);

	//Now we're iterating through the table values 
	for(auto iter = table.begin(); iter < table.end(); ++iter){
		//We need to xor the history bits with the table address to find table entry
		bool pathTaken = iter->taken == 1;
		int index = (iter->address ^ historyToInt(history)) % 2048;
		int & GsharePath = fixedTable[index];
		bool GshareCorrect = (GsharePath > 0) == pathTaken;

		//update predictor
		if(GshareCorrect){
			accuracyCnt++;
			if(GsharePath == 1){
				GsharePath = 2;
			}else if(GsharePath == -1){
				GsharePath = -2;
			}
		}else if(!GshareCorrect){
			if(GsharePath == 1 || GsharePath == -2){
				GsharePath = -1;
			}else if(GsharePath == -1 || GsharePath == 2){
				GsharePath = 1;
			}
		}

		//update history
		if(!pathTaken){
			history = shiftHistoryBitsLeft(history);
		}else{
			history = shiftHistoryBitsLeft(history, "1");
		}
	}

	vector<int> resultVec;
	resultVec.push_back(accuracyCnt);
	resultVec.push_back(table.size());
	return resultVec;
}

//Need to create both tables, one for each Bimodal and Gshare and a new selector table on top of it
vector<int> tournamentPredictor(vector<tableEntry> table){
	int accuracyCnt = 0;
	string history = "00000000000";
	vector<int> fixedTableGshare(2048, 2);
	vector<int> fixedTableBimodal(2048, 2);
  	vector<int> selectorTable(2048, 0);
 		 
	//Now we're iterating through the table values 
	for(unsigned int i = 0; i < table.size(); i++){
		bool pathTaken = table[i].taken == 1;
		int gIndex = (table[i].address ^ historyToInt(history)) % 2048;
		int index = table[i].address % 2048;
      
		int & GsharePath = fixedTableGshare[gIndex];
		int & bimodalPath = fixedTableBimodal[index];
		int & selector = selectorTable[index];
      
		bool GshareCorrect = (GsharePath > 0) == pathTaken;
		bool bimodalCorrect = (bimodalPath > 0) == pathTaken;

		//update selector
		if(selector < 2){
			if(GshareCorrect){
				accuracyCnt++;
				if(!bimodalCorrect)
					if(selector > 0)
						selector--;
			}else{
				if(bimodalCorrect)
					if(selector < 3)
						selector++;
			}
		}else{
			if(bimodalCorrect){
				accuracyCnt++;
				if(!GshareCorrect)
					if(selector < 3)
						selector++;
			}else{
				if(GshareCorrect)
					if(selector > 0)
						selector--;
			}
		}
      
		//update predictors
		if(bimodalCorrect){
			if(bimodalPath == 1){
				bimodalPath = 2;
			}else if(bimodalPath == -1){
				bimodalPath = -2;
			}
		}else if(!bimodalCorrect){
			if(bimodalPath == 1 || bimodalPath == -2){
				bimodalPath = -1;
			}else if(bimodalPath == -1 || bimodalPath == 2){
				bimodalPath = 1;
			}
		}

		if(GshareCorrect){
			if(GsharePath == 1){
				GsharePath = 2;
			}else if(GsharePath == -1){
				GsharePath = -2;
			}
		}else if(!GshareCorrect){
			if(GsharePath == 1 || GsharePath == -2){
				GsharePath = -1;
			}else if(GsharePath == -1 || GsharePath == 2){
				GsharePath = 1;
			}
		}

		//update history
		if(!pathTaken){
			history = shiftHistoryBitsLeft(history);
		}else{
			history = shiftHistoryBitsLeft(history, "1");
		}
	}

	vector<int> resultVec;
	resultVec.push_back(accuracyCnt);
	resultVec.push_back(table.size());
	return resultVec;
}

//Used for updating the Gshare table after each single comparison
string shiftHistoryBitsLeft(string history, string postFix){
	string shiftedHistory = history.substr(1, history.size()) + postFix;
	return shiftedHistory;
}

unsigned long long historyToInt(string history){
	unsigned long long i = std::stoi(history, nullptr, 2);
	return i;
}

//prints result in the format needed for output.txt to stdout for testing
void printResults(vector<vector<vector<int>>> resultVec, const char *argv){
	std::ofstream output(argv);
	for(unsigned int x = 0; x < resultVec.size(); x++){
		for(unsigned int i = 0; i < resultVec[x].size(); i++){
			for(unsigned int j = 0; j < resultVec[x][i].size(); j++){
				if(j != resultVec[x][i].size() - 1){
					output  << resultVec[x][i][j] << ",";
				}else{
					output << resultVec[x][i][j] << ";";
				}
			}
			if(i != resultVec[x].size() - 1){
				output << " ";	
			}
		}
		output << endl;
	}
	output.close();
}
