#define FORWARD 201
#define RETURN 200

#include <iostream>
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int numprocs, myRank;
MPI_Comm graph_comm;
MPI_Status status;

int nnodes = 14;
int graph_index[14] = {2, 7, 11, 15, 20, 26, 28, 32, 38, 42, 44, 46, 49, 52};
int graph_edges[52] = {1, 2, 0, 2, 3, 4, 5, 0, 1, 5, 6, 1, 4, 7, 8, 1, 3, 5, 7, 9, 1, 2, 4, 6, 8, 9, 2, 5, 3, 4, 10, 11, 3, 5, 9, 10, 12, 13, 4, 5, 8, 13, 7, 8, 7, 12, 8, 11, 13, 8, 9, 12};
int graph_reorder = 0;

int create_graph() {
    if (numprocs < nnodes) {
        return 1;
    }
    MPI_Graph_create(MPI_COMM_WORLD, nnodes, graph_index, graph_edges, graph_reorder, &graph_comm);
    return 0;
}

int delete_source(int neighbours[], int neighbourNumber, int source_node) {
    int i = 0;
    
    while (i < neighbourNumber) {
        if(neighbours[i] == source_node) {
            break;
        } else {
            i ++;
        }
    }
    
    for (int j = i; j < neighbourNumber-1; j++){
        neighbours[j]=neighbours[j+1];
    }
    
    return --neighbourNumber;
}

int main( int argc, char *argv[] ) {
    
    srand(time(NULL));
    
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    if(create_graph() != 0){
        cout<<"This program needs "<<nnodes<<"nodes."<<endl;
        return 1;
    }
    
    int i, neighbourNumber, *neighbours, rank;
    int running = 1;
    int visited = 0;
    int father = -1;
    
    MPI_Comm_rank(graph_comm, &rank);
    
    MPI_Graph_neighbors_count(graph_comm, rank, &neighbourNumber);
    neighbours = new int(neighbourNumber * sizeof(int));
    MPI_Graph_neighbors(graph_comm,rank,neighbourNumber,neighbours);
    
    if(rank == 0){
        visited = 1;
        for(i = 0; i < neighbourNumber; i++){
            MPI_Send(NULL, 0, MPI_INT, neighbours[i], FORWARD, graph_comm);
        }
    }
    
    while(running){
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, graph_comm, &status);
        if(int(status.MPI_TAG) == FORWARD){
            neighbourNumber = delete_source(neighbours, neighbourNumber, status.MPI_SOURCE);
            if (visited == 0){
                father = status.MPI_SOURCE;
                visited = 1;
                printf("%f | node \"%d\" ---> node \"%d\"\n", MPI_Wtime(), father, rank);
                for(i=0; i< neighbourNumber; i++){
                    MPI_Send(NULL, 0, MPI_INT, neighbours[i], FORWARD, graph_comm);
                }
            }else{
                MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, RETURN, graph_comm);
            }
            
            if (neighbourNumber == 0){
                if(father != -1){
                    MPI_Send(NULL, 0, MPI_INT, father, RETURN, graph_comm);
                }
                    running = 0;
            }
        }else if(int(status.MPI_TAG) == RETURN){
            neighbourNumber = delete_source(neighbours, neighbourNumber, status.MPI_SOURCE);
            if (neighbourNumber == 0){
                if(father == -1){
                    printf("root --> end of program\n");
                } else {
                    MPI_Send(NULL, 0, MPI_INT, father, RETURN, graph_comm);
                }
                running = 0;
            }
        }
    }
    
    MPI_Finalize();
}

