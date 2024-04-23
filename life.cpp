// Faculty: BUT FIT 
// Course: PRL 
// Project Name: Pipeline Merge Sort  
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

// number of process: 4 


#include <iostream>
#include <fstream>
#include <queue>
#include "mpi.h"
#include <cmath>

using namespace std;

// define arguments meaning 
#define ARGS_FILE 1
#define ARGS_STEPS 2


/**
 * First process that parses input into the globalTable and sets the number 
 * of steps 
 * 
 * 
 * IF error exits the program 
 * 
 * File format: 
 * 00000000
 * 00111000
 * 01110000
 * 00000000
*/
void first_proces(int argc, char *argv[], vector<int> *global_table, 
				  int *steps, int *column_size){

	string 		file_name = ""; 
	ifstream 	file;

	// Open file 
	if (argc != 3){
		cerr << "Error: wrong arguments" << endl;
		exit(1);
	}

	file_name 	= argv[ARGS_FILE];
	*steps 		= atoi(argv[ARGS_STEPS]);	

	
	file.open(file_name);
	if (!file){
		cerr << "Error: Unable to open file" << endl;
		exit(1);
	}

	// Parse file into the global_table 	
	char c;
	bool first_row  = false;
	int  count 		= 0;
	*column_size 	= 0;
	while (file.get(c)){
		
		if (c == '\n'){
			if (first_row == false){
				first_row = true;
				*column_size = count;
			}
		}
		else {
			count++;
			global_table->push_back(c);
		}	
	}



	// close the file 
	file.close();

}

int main(int argc, char *argv[]) {

	// NW N NE
	// W  C  E
	// SW S SE

    int rank, size;
    MPI_Init(&argc, &argv);

    // get the number of process 
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // get current procces id 
    // MPI_COMM_WORLD - prediefined constant to match all the processes  
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		
	// number of game steps 
	// global table of the game life 
	vector<int> global_table;
    if (rank == 0){
		
		// size of the input file 
		int steps;
		int column_size;	
		int rows; 
		first_proces(argc, argv, &global_table, &rank, &column_size);
		rows = global_table.size() / column_size; 

		cout << "Size:         " << global_table.size() << endl;
		cout << "Column size:  " << column_size << endl;
		cout << "Rows:         " << rows << endl;

		for (int i=0; i < global_table.size(); i++){
			cout << static_cast<char>((global_table)[i]);
			if (((i+1) % column_size) == 0){
				cout << endl;
			}
		}



    }

	// tell how many steps, rows and colums we have 
	//MPI_Bcast()








    MPI_Finalize();
    return 0;
}

