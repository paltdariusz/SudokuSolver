#include <iostream>
//#include <mpi.h>
#include <vector>
#include <chrono>
#include <array>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

class Sudoku {
public:
    long long solveTime = 0;

    Sudoku() {
        // Initialize grid with all zeroes
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid[i][j] = 0;
            }
        }
    }

    void setValue(std::string values, int mode) {
        int row, col = 0;
        switch (mode) {
            case 0:
                // adding values to solve grid
                for (int i = 0; i < values.length(); i++) {
                    row = i / 9;
                    col = i % 9;
                    grid[row][col] = (int) values[i] - 48;
                    startingGrid[row][col] = (int) values[i] - 48;
                }
                break;
            case 1:
                // adding values to solved grid
                for (int i = 0; i < values.length(); i++) {
                    row = i / 9;
                    col = i % 9;
                    solvedGrid[row][col] = (int) values[i] - 48;
                }
                break;
        }
    }
//    void setValue(int row, int col, int value) {
//        grid[row][col] = value;
//    }

    bool solve() {
        int rank, num_procs;
        rank = 0;
        num_procs = 1;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
        // Start the solving process
        bool solved = solve(rank, num_procs, 0, 0);
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

    bool checkSolution() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] != solvedGrid[i][j]) return false;
            }
        }
        return true;
    }

    std::vector<std::string> sudokuToString() {
        std::vector<std::string> sudokuStrings(3);
        for (int i = 0; i < 3; i++) {
            sudokuStrings[i] = "";
        }
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                sudokuStrings[0] += (char) (startingGrid[i][j] + 48);
                sudokuStrings[1] += (char) (grid[i][j] + 48);
                sudokuStrings[2] += (char) (solvedGrid[i][j] + 48);
            }
        }
        return sudokuStrings;
    }

private:
    // 2D grid to store the puzzle
    int startingGrid[9][9];
    int grid[9][9];
    int solvedGrid[9][9];

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
            } else {
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
                } else {
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

int main(int argc, char **argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    std::string file_name = "data.csv";
    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cerr << "Could not open file: " << file_name << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::vector<std::string>> data;
    while (getline(file, line)) {
        std::string cell;
        std::vector<std::string> row;
        std::stringstream line_stream(line);

        while (getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        data.push_back(row);
    }
    file.close();
    data.erase(data.begin());

    // Create a Sudoku objects
    const int SUDOKU_NUM = 1000000;

    std::array<Sudoku, SUDOKU_NUM> sudokus;

    for (int i = 0; i < SUDOKU_NUM; i++) {
        sudokus[i].setValue(data[i][0], 0);
        sudokus[i].setValue(data[i][1], 1);
    }

    std::ofstream output("results.csv", std::ios::out | std::ios::app);
    if (output.is_open()) {
        output << "Sudoku,Solutions,Solved,OwnSolution,isSolutionSame,Time,NumProc\n";
    }

    // Set the initial state of the puzzle
    for (int i = 0; i < SUDOKU_NUM; i++) {
        // Record the start time
        auto start_time = std::chrono::high_resolution_clock::now();

        // Solve the puzzle
        bool solved = sudokus[i].solve();

        // Record the end time
        auto end_time = std::chrono::high_resolution_clock::now();
        int rank, num_proc;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
        if (rank == 0) {
            // Calculate and print the time taken to solve the puzzle
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            sudokus[i].solveTime = duration.count();
            std::vector<std::string> sudokuStrings = sudokus[i].sudokuToString();
            if (output.is_open()) {
                output << sudokuStrings[0] << "," << sudokuStrings[2] <<"," << solved <<"," <<sudokuStrings[1] << ",";
                output << sudokus[i].checkSolution() << " " << sudokus[i].solveTime << "," << num_proc << "\n";
            }
        }
    }
    if (output.is_open()) {
        output.close();
    }
    MPI_Finalize();
    return 0;
}