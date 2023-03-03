#include <chrono>
#include <cstring>
#include <thread>

#include <mpi.h>

#include "solver.hpp"
#include "../matrix/matrix.hpp"

using std::memcpy;

using std::this_thread::sleep_for;
using std::chrono::microseconds;

void solveSeq(int rows, int cols, int iterations, double td, double h, int sleep, double ** matrix)
{
    double c, l, r, t, b;
    
    double h_square = h * h;

    double * linePrevBuffer = new double[cols];
    double * lineCurrBuffer = new double[cols];

    for(int k = 0; k < iterations; k++) {

        memcpy(linePrevBuffer, matrix[0], cols * sizeof(double));
        for(int i = 1; i < rows - 1; i++) {

            memcpy(lineCurrBuffer, matrix[i], cols * sizeof(double));
            for(int j = 1; j < cols - 1; j++) {
                c = lineCurrBuffer[j];
                t = linePrevBuffer[j];
                b = matrix[i + 1][j];
                l = lineCurrBuffer[j - 1];
                r = lineCurrBuffer[j + 1];


                sleep_for(microseconds(sleep));
                matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
            }

            memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
        }
    }
}

void solvePar(int rows, int cols, int iterations, double td, double h, int sleep, double ** matrix, int rank, int nbProc) 
{

    double * lineTopBuffer = new double[cols];
    double * lineBottomBuffer = new double[cols];

    double c, l, r, t, b;
    
    double h_square = h * h;

    double * linePrevBuffer = new double[cols];
    double * lineCurrBuffer = new double[cols];

    for(int k = 0; k < iterations; k++) {


        memcpy(linePrevBuffer, matrix[0], cols * sizeof(double));
        for(int i = 1; i < rows - 1; i++) {
                        // receive top array from rank - 1 (if rank > 0)
            if(rank > 0){
                MPI_Recv(lineTopBuffer, cols, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
            }
            // receive bottom array from rank + 1 (if rank < nbProc -1)
            if(rank < nbProc - 1){
                MPI_Recv(lineBottomBuffer, cols, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
            }
        // Send top array to rank - 1 (if rank > 0)
            if(rank > 0){
                MPI_Send(lineTopBuffer, cols, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
            }
            // Send bottom array to rank + 1 (if rank < nbProc -1)
            if(rank < nbProc - 1){
                MPI_Send(lineBottomBuffer, cols, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            }

            memcpy(lineCurrBuffer, matrix[i], cols * sizeof(double));
            for(int j = 1; j < cols - 1; j++) {
                if(rank != nbProc && i == rows - 2){
                    c = lineCurrBuffer[j];
                    t = linePrevBuffer[j];
                    b = lineTopBuffer[j];
                    l = lineCurrBuffer[j - 1];
                    r = lineCurrBuffer[j + 1];

                    sleep_for(microseconds(sleep));
                    matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }else if(rank != nbProc && i == 1){
                    c = lineCurrBuffer[j];
                    t = lineBottomBuffer[j];
                    b = matrix[i + 1][j];
                    l = lineCurrBuffer[j - 1];
                    r = lineCurrBuffer[j + 1];


                    sleep_for(microseconds(sleep));
                    matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }else{
                    c = lineCurrBuffer[j];
                    t = linePrevBuffer[j];
                    b = matrix[i + 1][j];
                    l = lineCurrBuffer[j - 1];
                    r = lineCurrBuffer[j + 1];


                    sleep_for(microseconds(sleep));
                    matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }
            }

            memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
            
        }

        memcpy(lineTopBuffer,      matrix[0], cols * sizeof(double));
        memcpy(lineBottomBuffer, matrix[rows-1], cols * sizeof(double));


    }

    sleep_for(microseconds(sleep));

   

}
