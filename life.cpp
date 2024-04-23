// Faculty: BUT FIT 
// Course: PRL 
// Project Name: Pipeline Merge Sort  
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

#include <iostream>
#include <fstream>
#include <queue>
#include "mpi.h"
#include <cmath>

using namespace std;

// define arguments meaning 
#define ARGS_FILE 1
#define ARGS_STEPS 2



typedef struct Table_size{
	int rows;
	int columns; 
} Table_size;



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
void first_proces(int argc, char *argv[], vector<int> *globalTable, 
				  int *steps, Table_size *table_size){

	string 		file_name = ""; 
	ifstream 	file;

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

	char c;
	while (file.get(c)){
		cout << c; 
		globalTable->push_back(c);
	}

	cout << "first proces" << endl;
	cout << "argc: " << argc << endl;
	for (int i = 0; i < argc; i++)
		cout << "argv: " << argv[i]<< endl;
	
	cout << "0 " << argv[0]<< endl;
	cout << "1 " << argv[1]<< endl;
	cout << "2 " << argv[2]<< endl;


	// close the file 
	file.close();

}

int main(int argc, char *argv[]) {
    
	// // get my rank 
    // int rank, size;
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // // get the number of process 
    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    // // Broadcast the event from process 0 to all other processes
    // MPI_Bcast(&total_nums, 1,MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Recv(&rcBuff, 1, MPI_BYTE, rank-1, UP, MPI_COMM_WORLD, &status);
    // MPI_Send(&sBuff, 1, MPI_BYTE, (rank+1), UP, MPI_COMM_WORLD);

    int rank, size;
    MPI_Init(&argc, &argv);

    // get the number of process 
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // get current procces id 
    // MPI_COMM_WORLD - prediefined constant to match all the processes  
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
		
		// number of game steps 
		int steps = 0;
		// global table of the game life 
		vector<int> global_table;
		// size of the input file 
		Table_size table_size;	

		first_proces(argc, argv, &global_table, &rank, &table_size);

    }
    // Nth process even the last one 
    else {
		cout << rank << " proces" << endl;
    }


    MPI_Finalize();
    return 0;
}

