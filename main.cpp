#include <iostream>
#include <array>
#include <omp.h>
#include <chrono>

class Sudoku {
private:
    std::array<std::array<int, 9>, 9> grid;

public:
    // Konstruktor
    Sudoku() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid[i][j] = 0;
            }
        }
    }

    // Metoda do ustawiania wartości na planszy
    void setValue(int row, int col, int value) {
        grid[row][col] = value;
    }

    void printSolution() {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                std::cout << grid[row][col] << " ";
            }
            std::cout << std::endl;
        }
    }

    // Metoda do rozwiązywania problemu
    bool solve() {
        bool solved = false;
#pragma omp parallel
        {
#pragma omp single
            {
                solved = solve(0, 0);
            }
        }
        return solved;
    }

private:
    // Metoda rekurencyjna do rozwiązywania problemu
    bool solve(int row, int col) {
        // Jeśli wszystkie pola zostały wybrane, to znaleziono rozwiązanie
        if (row == 9) {
            return true;
        }

        // Przeskakiwanie zapełnionych pól
        if (grid[row][col] != 0) {
            if (col == 8) {
                return solve(row + 1, 0);
            } else {
                return solve(row, col + 1);
            }
        }

        // Próba wstawienia każdej liczby z zakresu 1-9
        for (int i = 1; i <= 9; i++) {
            if (isValid(row, col, i)) {
                grid[row][col] = i;
                if (col == 8) {
                    if (solve(row + 1, 0)) {
                        return true;
                    }
                } else {
                    if (solve(row, col + 1)) {
                        return true;
                    }
                }
                grid[row][col] = 0;
            }
        }
        return false;
    }

    bool isValid(int row, int col, int value) {
        // Sprawdzanie czy podana wartość już występuje w wierszu
        if (checkRow(row, value)) {
            return false;
        }
        // Sprawdzanie czy podana wartość już występuje w kolumnie
        if (checkCol(col, value)) {
            return false;
        }
        // Sprawdzanie czy podana wartość już występuje w kwadracie 3x3
        if (checkSquare(row, col, value)) {
            return false;
        }
        return true;
    }

    bool checkRow(int row, int value) {
        for (int col = 0; col < 9; col++) {
            if (grid[row][col] == value) {
                return true;
            }
        }
        return false;
    }

    bool checkCol(int col, int value) {
        for (int row = 0; row < 9; row++) {
            if (grid[row][col] == value) {
                return true;
            }
        }
        return false;
    }

    bool checkSquare(int row, int col, int value) {
        int squareRow = row - row % 3;
        int squareCol = col - col % 3;
        for (int i = squareRow; i < squareRow + 3; i++) {
            for (int j = squareCol; j < squareCol + 3; j++) {
                if (grid[i][j] == value) {
                    return true;
                }
            }
        }
        return false;
    }
};

int main() {
    // Tworzenie obiektu klasy Sudoku
    Sudoku sudoku;

    // Wprowadzanie danych do planszy
    sudoku.setValue(0, 0, 8);
    sudoku.setValue(0, 1, 5);
    sudoku.setValue(0, 7, 2);
    sudoku.setValue(0, 8, 9);
    sudoku.setValue(1, 0, 7);
    sudoku.setValue(1, 1, 2);
    sudoku.setValue(2, 2, 4);
    sudoku.setValue(3, 3, 1);
    sudoku.setValue(3, 4, 2);
    sudoku.setValue(4, 6, 3);
    sudoku.setValue(4, 7, 4);
    sudoku.setValue(5, 8, 2);
    sudoku.setValue(6, 8, 6);
    sudoku.setValue(7, 8, 8);
    sudoku.setValue(8, 0, 6);
    sudoku.setValue(8, 4, 5);
    sudoku.setValue(8,8,1);
    // Rozwiązywanie problemu
    if (sudoku.solve()) {
        // Jeśli rozwiązanie znalezione, wyświetlenie rozwiązania
        sudoku.printSolution();

    }
    else {
        std::cout << "Nie znaleziono rozwiazania." << std::endl;
    }

    return 0;
}