#define GO 201
#define BACK 200
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>
int rank, numprocs;
MPI_Comm graph_comm;
MPI_Status status;
int graph_node_count = 14;
int graph_index[14] = {2, 7, 11, 15, 20, 26,28,32,38,42,44,46,49,52};
int graph_edges[52] = {1, 2, 0, 2, 3,4,5, 0, 1, 5, 6, 1, 4, 7, 8,
    1,3,5,7,9,1,2,4,6,8,9,2,5,3,4,10,11,3,5,9,10,12,13,4,5,8,13,7,8,7,12,8,11,13,8,9,12};
int create_graph() {
    if (numprocs < graph_node_count) {
        return 1;
    }
    MPI_Graph_create(MPI_COMM_WORLD, graph_node_count, graph_index, graph_edges,
                     1, &graph_comm);
    return 0;
}
int delete_source(int neighbors[], int counter, int element) {
    int i, j;
    i = 0;
    while (i < counter) {
        if(neighbors[i] == element) {
            break;
        } else {
            i ++;}
    }
    for (j = i; j < counter-1; j++){
        neighbors[j]=neighbors[j+1];
    }
    return --counter;
}
int pick_recv(int neighbors[], int neighbor_count){
    return neighbors[0];
}
int main(int argc, char *argv[]) {
    srand(time(NULL));
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(create_graph() != 0) {
        printf("This program needs \"%d\" nodes.\n", graph_node_count);
        return 1;
    }
    int marche = 1;
    int rank, neighbor_count, *neighbors;
    int ajouter = 0;
    int pere = -1;
    int next_recv;
    MPI_Comm_rank(graph_comm, &rank);
    MPI_Graph_neighbors_count(graph_comm, rank, &neighbor_count);
    neighbors = new int(neighbor_count*sizeof(int) );
    MPI_Graph_neighbors(graph_comm, rank, neighbor_count, neighbors);
    if(rank == 0){
        ajouter = 1;
        next_recv=pick_recv(neighbors, neighbor_count);
        printf("%f | \"%d\" ====> node \"%d\"\n", MPI_Wtime(), rank, neighbors[0]);
        MPI_Send(NULL, 0, MPI_INT, next_recv, GO, graph_comm);
    }while (marche){
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, graph_comm,
                 &status);
        if((int)status.MPI_TAG==GO){
            neighbor_count = delete_source(neighbors, neighbor_count,
                                           status.MPI_SOURCE);
            if (ajouter == 0 && neighbor_count != 0){
                pere = status.MPI_SOURCE;
                ajouter = 1;
                printf("%f | node \"%d\", my neighbors ", MPI_Wtime(), rank);
                for(int i = 0; i < neighbor_count; i++){
                    printf("\"%d\" ", neighbors[i]);
                }
                printf("\n");
                next_recv = pick_recv(neighbors, neighbor_count);
                printf("%f | node \"%d\" ====> node \"%d\"\n", MPI_Wtime(), rank,
                       next_recv);
                MPI_Send(NULL, 0, MPI_INT, next_recv, GO, graph_comm);
            } else {
                printf("%f | node \"%d\" ====> node \"%d\"\n", MPI_Wtime(), rank,
                       status.MPI_SOURCE);
                MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, BACK,
                         graph_comm);
                if(ajouter == 0){
                    marche = 0;
                }
            }
        }
        if((int)status.MPI_TAG==BACK){
            neighbor_count = delete_source(neighbors, neighbor_count,
                                           status.MPI_SOURCE);
            if (neighbor_count == 0){
                if(pere == -1){
                    printf("0 --> end of program\n");
                } else {
                    printf("%f | node \"%d\" ====> node \"%d\"\n", MPI_Wtime(), rank,
                           pere);
                    MPI_Send(NULL, 0, MPI_INT, pere, BACK, graph_comm);
                }
                ajouter = 0;marche = 0;
            } else {
                next_recv = pick_recv(neighbors, neighbor_count);
                printf("%f | node \"%d\" ====> node %d\n", MPI_Wtime(), rank,
                       next_recv);
                MPI_Send(NULL, 0, MPI_INT, next_recv, GO, graph_comm);
            }
        }
    }
    printf("%f | node \"%d\" --> end of task\n", MPI_Wtime(), rank);
    MPI_Finalize();
}
