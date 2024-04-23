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

#define FROM_DOWN 1
#define FROM_UP 2

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

/**
 * each process calls this function and execute the GoL
*/
void game_of_life(vector<int> *local, int game_info[3], int rank, int size){
	
	// NW N NE
	// W  C  E
	// SW S SE

	// neigh-up
	// neigh-down
	vector<int> from_up(game_info[COLUMN_SIZE]);
	vector<int> from_down(game_info[COLUMN_SIZE]);
	
	vector<int> my_first_row(game_info[COLUMN_SIZE]);
	vector<int> my_last_row(game_info[COLUMN_SIZE]);

	MPI_Status status;
	MPI_Request req[2];
	
	int j = local->size() - game_info[COLUMN_SIZE];
	for (int i = 0; i < game_info[COLUMN_SIZE]; i++){
		my_first_row[i] = (*local)[i];
		my_last_row[i]  = (*local)[j++];
	}

	// send first and last row to each of my neighbour 
	if (rank == MAIN_PROCES){ // first process 
		MPI_Isend(my_first_row.data(), game_info[COLUMN_SIZE], MPI_INT, (size-1), FROM_DOWN, MPI_COMM_WORLD, &req[0]);
		MPI_Isend(my_last_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_UP,   MPI_COMM_WORLD, &req[1]);
		
		MPI_Recv(from_down.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_DOWN, MPI_COMM_WORLD, &status);
		MPI_Recv(from_up.data(), game_info[COLUMN_SIZE], MPI_INT, (size-1), FROM_UP,   MPI_COMM_WORLD, &status);
	}
	else if (rank == size-1){ // last process 
		MPI_Isend(my_first_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_DOWN,  MPI_COMM_WORLD, &req[0]);
		MPI_Isend(my_last_row.data(), game_info[COLUMN_SIZE], MPI_INT, MAIN_PROCES, FROM_UP, MPI_COMM_WORLD, &req[1]);
		
		MPI_Recv(from_down.data(), game_info[COLUMN_SIZE], MPI_INT, MAIN_PROCES, FROM_DOWN, MPI_COMM_WORLD, &status);
		MPI_Recv(from_up.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_UP,      MPI_COMM_WORLD, &status);
	}
	else{
		MPI_Isend(my_first_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_DOWN, MPI_COMM_WORLD, &req[0]);
		MPI_Isend(my_last_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_UP,   MPI_COMM_WORLD, &req[1]);
		
		MPI_Recv(from_down.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_DOWN,    MPI_COMM_WORLD, &status);
		MPI_Recv(from_up.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_UP,      MPI_COMM_WORLD, &status);
	}

	// if 0 -> use neig-up 
	int curr_row = 0;



}

int main(int argc, char *argv[]) {
    
	
	int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	// global table of the game life 
	vector<int> global_table;
	vector<int> chunk_size(size);
	vector<int> indexes(size); 		
	// information about the game	
	int game_info[3];

    if (rank == MAIN_PROCES){

		first_proces(argc, argv, &global_table, &game_info[STEPS], &game_info[COLUMN_SIZE]);
		game_info[ROWS] = global_table.size() / game_info[COLUMN_SIZE]; 

		cout << "Size:         " << global_table.size() << endl;
		cout << "Column size:  " << game_info[COLUMN_SIZE] << endl;
		cout << "Rows:         " << game_info[ROWS] << endl;
		cout << "Steps:        " << game_info[STEPS] << endl;
		
		cout << endl << "START: " << endl;
		for (int i=0; i < global_table.size(); i++){
			cout << global_table[i];
			if (((i+1) % game_info[COLUMN_SIZE]) == 0){
				cout << endl;
			}
		}
		cout << endl;
    }

	// tell how many steps, rows and colums we have 
	MPI_Bcast(&game_info, 3, MPI_INT, MAIN_PROCES, MPI_COMM_WORLD);

	// count chunk_size and indexes for each process 	
	count_index(&chunk_size, &indexes, game_info);
	// local vectors
	vector<int> local(chunk_size[rank]);
		

	// scatter the global board among ranks using sendCounts and displacements
    MPI_Scatterv(global_table.data(), chunk_size.data(), indexes.data(), MPI_INT,  
        local.data(), chunk_size[rank], MPI_INT ,MAIN_PROCES, MPI_COMM_WORLD);

	game_of_life(&local, game_info, rank, size);

	// reasembly
	MPI_Gatherv(local.data(), chunk_size[rank], MPI_INT, global_table.data(),
		chunk_size.data(), indexes.data(), MPI_INT, MAIN_PROCES, MPI_COMM_WORLD);

	// end 
	if (rank == MAIN_PROCES){
		cout << "END: " << endl;
		for (int i=0; i < global_table.size(); i++){
			cout << global_table[i];
			if (((i+1) % game_info[COLUMN_SIZE]) == 0){
				cout << endl;
			}
		}
	}


    MPI_Finalize();
    return 0;
}

