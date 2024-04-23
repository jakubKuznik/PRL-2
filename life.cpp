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

/**
 * First process that parses input into the globalTable and sets the number 
 * of steps 
 * 
 * 
 * IF error exits the program 
*/
void first_proces(int argc, char *argv[], vector<int> *globalTable, int *steps){

	cout << "first proces" << endl;
	cout << "argc: " << argc << endl;
	for (int i = 0; i < argc; i++)
		cout << "argv: " << argv[i]<< endl;

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
		
		first_proces(argc, argv, &global_table, &rank);

    }
    // Nth process even the last one 
    else {
		cout << rank << " proces" << endl;
    }


    MPI_Finalize();
    return 0;
}

