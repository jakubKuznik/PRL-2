// Faculty: BUT FIT 
// Course: PRL 
// Project Name: Pipeline Merge Sort  
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

// WRAP-around implementation
// number of process that should be run: 4 


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

#define DEAD 0
#define ALIVE 1 

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
	
	
	// neigh-up
	// neigh-down
	vector<int> from_up(game_info[COLUMN_SIZE]);
	vector<int> from_down(game_info[COLUMN_SIZE]);
	
	vector<int> my_first_row(game_info[COLUMN_SIZE]);
	vector<int> my_last_row(game_info[COLUMN_SIZE]);

	vector<int> work_up(game_info[COLUMN_SIZE]);
	vector<int> work(game_info[COLUMN_SIZE]);
	vector<int> work_down(game_info[COLUMN_SIZE]);
	
	vector<int> out(local->size());
	
	MPI_Status status;

	int total_rows = local->size() / game_info[COLUMN_SIZE];
	int alive_neigh = 0;
	int dead_neigh  = 0;
	int LAST = game_info[COLUMN_SIZE] -1; 
	int index; 

	for (int i = 0; i < game_info[STEPS]; i++){
		
		// Get and send information to other proceses 
		int j = local->size() - game_info[COLUMN_SIZE];
		for (int i = 0; i < game_info[COLUMN_SIZE]; i++){
			my_first_row[i] = (*local)[i];
			my_last_row[i]  = (*local)[j++];
		}
		// send first and last row to each of my neighbour 
		if (rank == MAIN_PROCES){ // first process 
			MPI_Send(my_first_row.data(), game_info[COLUMN_SIZE], MPI_INT, (size-1), FROM_DOWN, MPI_COMM_WORLD);
			MPI_Send(my_last_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_UP,   MPI_COMM_WORLD);
		
			MPI_Recv(from_down.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_DOWN, MPI_COMM_WORLD, &status);
			MPI_Recv(from_up.data(), game_info[COLUMN_SIZE], MPI_INT, (size-1), FROM_UP,   MPI_COMM_WORLD, &status);
		}
		else if (rank == size-1){ // last process 
			MPI_Send(my_first_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_DOWN,  MPI_COMM_WORLD);
			MPI_Send(my_last_row.data(), game_info[COLUMN_SIZE], MPI_INT, MAIN_PROCES, FROM_UP, MPI_COMM_WORLD);
		
			MPI_Recv(from_down.data(), game_info[COLUMN_SIZE], MPI_INT, MAIN_PROCES, FROM_DOWN, MPI_COMM_WORLD, &status);
			MPI_Recv(from_up.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_UP,      MPI_COMM_WORLD, &status);
		}
		else{
			MPI_Send(my_first_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_DOWN, MPI_COMM_WORLD);
			MPI_Send(my_last_row.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_UP,   MPI_COMM_WORLD);
		
			MPI_Recv(from_down.data(), game_info[COLUMN_SIZE], MPI_INT, (rank+1), FROM_DOWN,    MPI_COMM_WORLD, &status);
			MPI_Recv(from_up.data(), game_info[COLUMN_SIZE], MPI_INT, (rank-1), FROM_UP,      MPI_COMM_WORLD, &status);
		}

		int from = 0;
		int p = 0;
		for (int r = 0; r < total_rows; r++){

			// get current working row 	
			from = r * game_info[COLUMN_SIZE]; 
			p = 0;
			for (int j = from; j < (from + game_info[COLUMN_SIZE]); j++){
				work[p++] = (*local)[j];
			}

			// if first row 				
			if (r == 0){
				for (int x = 0; x < game_info[COLUMN_SIZE]; x++){
					work_up[x]  = from_up[x]; 
				}
			}
			else{
				from = (r-1) * game_info[COLUMN_SIZE];
				p = 0;
				for (int i = from; i < (from + game_info[COLUMN_SIZE]); i++){
					work_up[p++] = (*local)[i];
				}

			}

			// if last row 
			if ((r+1) == total_rows){
				for (int x = 0; x < game_info[COLUMN_SIZE]; x++){
					work_down[x]  = from_down[x]; 
				}
			}
			else{
				from = (r+1) * game_info[COLUMN_SIZE]; 
				p = 0;
				for (int i = from; i < (from + game_info[COLUMN_SIZE]); i++){
					work_down[p++] = (*local)[i];
				}

			}
			
			// NW N NE
			// W  C  E
			// SW S SE
			for (int i = 0; i < game_info[COLUMN_SIZE]; i++){
				alive_neigh = 0;
			
				// first cell 
				if (i == 0){
					// left 
					alive_neigh += work_up[LAST];
					alive_neigh += work_down[LAST];
					alive_neigh += work[LAST];
					// right 
					alive_neigh += work_up[i+1];
					alive_neigh += work_down[i+1];
					alive_neigh += work[i+1];
				} 
				// last cell 
				if ((i+1) == game_info[COLUMN_SIZE]){
					// left 
					alive_neigh += work_up[i-1];
					alive_neigh += work_down[i-1];
					alive_neigh += work[i-1];
					// right 
					alive_neigh += work_up[0];
					alive_neigh += work_down[0];
					alive_neigh += work[0];
				}
				if (!(i == 0) && !((i+1) == game_info[COLUMN_SIZE])){
					// left 
					alive_neigh += work_up[i-1];
					alive_neigh += work_down[i-1];
					alive_neigh += work[i-1];
					// right 
					alive_neigh += work_up[i+1];
					alive_neigh += work_down[i+1];
					alive_neigh += work[i+1];
				}
					
				alive_neigh += work_up[i];
				alive_neigh += work_down[i];
				
				// each alive cell with less then 2 alive neighbours dies  
				// each alive cell with 2 or 3 neighbours stays alive
				// each alive cell with more than 3 alive neighbours dies 
				// each dead cell  with exactly 3 alive neighbours alives 
				index = (r * game_info[COLUMN_SIZE]) + i;
				if (work[i] == DEAD){
					if (alive_neigh == 3){
						out[index] = ALIVE;
					}
					else{
						out[index] = work[i];
					}
				}
				else{
					if (alive_neigh < 2){
						out[index] = DEAD;
					}
					else if (alive_neigh > 3){
						out[index] = DEAD;
					}
					else{
						out[index] = work[i];
					}
				}
			}

		}
		// copy to my local buff 
		for (int x = 0; x < local->size() ; x++){
			(*local)[x] = out[x]; 
		}
	}
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
		int j = 0;
		for (int i = 0; i < global_table.size(); i++){
			if (i == (indexes[j] + chunk_size[j])){
				j++;
			}
			if ((i % game_info[COLUMN_SIZE]) == 0){
				cout << j << ": ";
			}
			cout << global_table[i];
			if (((i+1) % game_info[COLUMN_SIZE]) == 0){
				cout << endl;
			}
		}
	}

    MPI_Finalize();
    return 0;
}

