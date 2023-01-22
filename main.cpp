#include <iostream>
#include <mpi.h>
#include <vector>
#include <chrono>

class Sudoku {
public:
    Sudoku() {
        // Initialize grid with all zeroes
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid[i][j] = 0;
            }
        }

        // Initialize MPI
        int provided;
        MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &provided);
    }

    void setValue(int row, int col, int value) {
        grid[row][col] = value;
    }

    bool solve() {
        int rank, num_procs;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
        // Start the solving process
        bool solved = solve(rank, num_procs, 0, 0);
        MPI_Finalize();
        return solved;
    }

    void printSolution() {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (rank == 0) {
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    std::cout << grid[i][j] << " ";
                }
                std::cout << std::endl;
            }
        }
    }

private:
    // 2D grid to store the puzzle
    int grid[9][9];

    // Helper method to check if a given value is valid for a given cell
    bool isValid(int row, int col, int value) {
        // Check row and column
        for (int i = 0; i < 9; i++) {
            if (grid[row][i] == value || grid[i][col] == value) {
                return false;
            }
        }

        // Check 3x3 sub-grid
        int startRow = row - row % 3;
        int startCol = col - col % 3;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (grid[i + startRow][j + startCol] == value) {
                    return false;
                }
            }
        }

        return true;
    }

    bool solve(int rank, int num_procs, int row, int col) {
        // If all cells have been filled, return true
        if (row == 9) {
            return true;
        }

        // If cell is already filled, move to the next cell
        if (grid[row][col] != 0) {
            if (col == 8) {
                if (solve(rank, num_procs, row + 1, 0)) {
                    return true;
                }
            }
            else {
                if (solve(rank, num_procs, row, col + 1)) {
                    return true;
                }
            }
            return false;
        }

        // Try all possible values for the current cell
        int start = rank;
        for (int value = 1; value <= 9; value += num_procs) {
            if (isValid(row, col, value)) {
                grid[row][col] = value;
                if (col == 8) {
                    if (solve(rank, num_procs, row + 1, 0)) {
                        return true;
                    }
                }
                else {
                    if (solve(rank, num_procs, row, col + 1)) {
                        return true;
                    }
                }
                grid[row][col] = 0;
            }
        }
        return false;
    }
};

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Create a Sudoku object
    Sudoku sudoku;

    // Set the initial state of the puzzle
    sudoku.setValue(0, 0, 8);
    sudoku.setValue(0, 1, 5);
    //...

    // Record the start time
    auto start_time = std::chrono::high_resolution_clock::now();

    // Solve the puzzle
    bool solved = sudoku.solve();

    // Record the end time
    auto end_time = std::chrono::high_resolution_clock::now();
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        if (solved) {
            // Print the solution
            sudoku.printSolution();

            // Calculate and print the time taken to solve the puzzle
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            std::cout << "Time taken to solve the puzzle: " << duration.count() << " microseconds" << std::endl;
        } else {
            std::cout << "No solution found." << std::endl;
        }
    }
    MPI_Finalize();
    return 0;
}