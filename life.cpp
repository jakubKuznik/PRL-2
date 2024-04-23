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

#define MAIN_PROCES 0 

#define STEPS 0
#define COLUMN_SIZE 1
#define ROWS 2

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
			global_table->push_back(c-48);
		}	
	}



	// close the file 
	file.close();

}

/**
 * Count indexes to find how many rows each process process 
*/
void count_index(vector<int> *chunk_size, vector<int> *indexes, int game_info[3]){
    
	int size;
    
	// get the number of process 
    MPI_Comm_size(MPI_COMM_WORLD, &size);



	int one_p_rows 		=  game_info[ROWS] / size;
	int rows_left  		=  game_info[ROWS] % size;
	int rows 	   		= 0;
	int rows_processed  = 0;
	for (int i = 0; i < size; i++){
		
		// Some process will process more rows
		if (rows_left > 0){
			rows = one_p_rows + 1;	
			rows_left--;
		
		}
		else{
			rows = one_p_rows;
		}

		(*indexes)[i] 		= rows_processed * game_info[COLUMN_SIZE];
		(*chunk_size)[i] 	= rows * game_info[COLUMN_SIZE];
		rows_processed += rows;
	}



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
		
	// global table of the game life 
	vector<int> global_table;
	// information about the game	
	int game_info[3];

    if (rank == MAIN_PROCES){

		first_proces(argc, argv, &global_table, &game_info[STEPS], &game_info[COLUMN_SIZE]);
		game_info[ROWS] = global_table.size() / game_info[COLUMN_SIZE]; 

		cout << "Size:         " << global_table.size() << endl;
		cout << "Column size:  " << game_info[COLUMN_SIZE] << endl;
		cout << "Rows:         " << game_info[ROWS] << endl;
		cout << "Steps:        " << game_info[STEPS] << endl;

    }

	// tell how many steps, rows and colums we have 
	MPI_Bcast(&game_info, 3, MPI_INT, MAIN_PROCES, MPI_COMM_WORLD);

	// count chunk_size and indexes for each process 	
	vector<int> chunk_size(size);
	vector<int> indexes(size); 		
	count_index(&chunk_size, &indexes, game_info);
	
	// local vectors
	vector<int> local(chunk_size[rank]);

		
	for (int i=0; i < global_table.size(); i++){
		cout << global_table[i];
		if (((i+1) % game_info[COLUMN_SIZE]) == 0){
			cout << endl;
		}
	}
	cout << endl;

	// scatter the global board among ranks using sendCounts and displacements
    MPI_Scatterv(global_table.data(), chunk_size.data(), indexes.data(), MPI_INT,  
        local.data(), chunk_size[rank], MPI_INT ,MAIN_PROCES, MPI_COMM_WORLD);
	

	// reasembly
	MPI_Gatherv(local.data(), chunk_size[rank], MPI_INT, global_table.data(),
		chunk_size.data(), indexes.data(), MPI_INT, MAIN_PROCES, MPI_COMM_WORLD);

	for (int i=0; i < global_table.size(); i++){
		cout << global_table[i];
		if (((i+1) % game_info[COLUMN_SIZE]) == 0){
			cout << endl;
		}
	}
	cout << endl;


    MPI_Finalize();
    return 0;
}

