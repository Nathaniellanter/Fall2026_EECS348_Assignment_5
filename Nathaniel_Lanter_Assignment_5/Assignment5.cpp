/*
Program: EECS 348 Assignment 5 - Recursive Sudoku Solver (writes solutions to files)
Description: Reads five Sudoku puzzles and uses recursive backtracking to find every valid solution.
             Solutions are printed to the console and written to solution1.txt - solution5.txt.
Inputs: puzzle1.txt, puzzle2.txt, puzzle3.txt, puzzle4.txt, puzzle5.txt
Outputs: Console output plus solutionN.txt for each puzzle N. Each file holds either the single line
         "No solution found." or all solutions (same format as the puzzle files) followed by
         "Solutions found: n".
Original author: Peter Lauman (Assignment 4)
Original creation date: October 15, 2026
Revised by: Nathaniel Lanter    Revision date: November 2, 2026   <-- set to the real date you edited
Collaborators / sources: Peter Lauman (original Assignment 4 code); OpenAI ChatGPT (used in original code);
                         <GenAI #1 you used>, <GenAI #2 you used> (Assignment 5 revisions)
Code authorship key used in comments: [ORIG] = Peter Lauman / ChatGPT, unchanged;
                                      [REV]  = added or changed for Assignment 5 (GenAI-assisted, then edited by Nathaniel Lanter)
*/
#include <array>      // [ORIG] gives std::array, which holds the 9x9 board.
#include <fstream>    // [ORIG] file streams for reading puzzles; [REV] now used for writing solutions too.
#include <iostream>   // [ORIG] console output through std::cout.
#include <ostream>    // [REV] std::ostream lets one print function target the console or a file.
#include <string>     // [ORIG] std::string for file names and tokens.
#include <vector>     // [ORIG] std::vector for lists of candidate digits.

class SudokuSolver {
private:
    std::array<std::array<int, 9>, 9> grid{}; // [ORIG] the board; 0 means the square is still blank.
    unsigned long long solutions = 0;        // [ORIG] running total of completed boards discovered.
    std::ofstream solutionFile;              // [REV] file handle for the current puzzle's solutionN.txt.
 
    // [ORIG] Decide if placing "value" at (row, col) breaks a Sudoku rule.
    bool allowed(int row, int col, int value) const {
        for (int i = 0; i < 9; ++i) { // [ORIG] walk along the whole row and the whole column together.
            if (grid[row][i] == value || grid[i][col] == value) return false; // [ORIG] the digit is already in that row or column.
        }
        int firstRow = (row / 3) * 3; // [ORIG] integer division snaps to the first row of this 3x3 box.
        int firstCol = (col / 3) * 3; // [ORIG] same idea for the box's first column.
        for (int r = firstRow; r < firstRow + 3; ++r) { // [ORIG] loop over the three rows of the box.
            for (int c = firstCol; c < firstCol + 3; ++c) { // [ORIG] loop over the three columns of the box.
                if (grid[r][c] == value) return false; // [ORIG] the digit already sits somewhere in the box.
            }
        }
        return true; // [ORIG] row, column, and box are all free of this digit.
    }
 
    // [ORIG] Pick the blank square with the fewest legal digits so the search branches as little as possible.
    bool chooseCell(int& bestRow, int& bestCol, std::vector<int>& bestValues) const {
        bestRow = -1; // [ORIG] -1 is the "nothing picked yet" marker for the row.
        bestCol = -1; // [ORIG] and for the column.
        bestValues.clear(); // [ORIG] throw away anything left over from an earlier call.
        for (int r = 0; r < 9; ++r) { // [ORIG] go through each row of the board.
            for (int c = 0; c < 9; ++c) { // [ORIG] and each square within that row.
                if (grid[r][c] != 0) continue; // [ORIG] only blank squares are interesting, so skip filled ones.
                std::vector<int> candidates; // [ORIG] digits that could legally go in this blank.
                for (int v = 1; v <= 9; ++v) { // [ORIG] test every digit from 1 to 9.
                    if (allowed(r, c, v)) candidates.push_back(v); // [ORIG] save the digits that pass the rules.
                }
                if (candidates.empty()) { // [ORIG] a blank with zero options means this path is hopeless.
                    bestRow = r; // [ORIG] record where the dead end is (row).
                    bestCol = c; // [ORIG] and its column.
                    bestValues.clear(); // [ORIG] an empty list tells the caller nothing can be placed.
                    return true; // [ORIG] report a blank exists, but with no way to fill it.
                }
                if (bestRow == -1 || candidates.size() < bestValues.size()) { // [ORIG] first blank seen, or one with fewer options than the best so far.
                    bestRow = r; // [ORIG] remember this row as the new best.
                    bestCol = c; // [ORIG] and its column.
                    bestValues = candidates; // [ORIG] and the digits that fit there.
                }
            }
        }
        return bestRow != -1; // [ORIG] false only when no blanks were found, i.e. the board is full.
    }
 
    // [ORIG] Depth-first search that finds every way to finish the board.
    void backtrack() {
        int row, col; // [ORIG] position of the blank we are about to fill.
        std::vector<int> values; // [ORIG] digits that may go in that blank.
        if (!chooseCell(row, col, values)) { // [ORIG] no blanks anywhere means the board is solved.
            ++solutions; // [ORIG] add one to the solution tally.
            std::cout << "Solution " << solutions << ":\n"; // [ORIG] console heading showing which solution this is.
            print(std::cout); // [REV] show the solved board on the console (print now needs a stream argument).
            print(solutionFile); // [REV] save the same board into solutionN.txt, in the same layout as the puzzle files.
            return; // [ORIG] go back up so the search can look for further solutions.
        }
        for (int value : values) { // [ORIG] try each allowed digit one after another.
            grid[row][col] = value; // [ORIG] put the digit in as a guess.
            backtrack(); // [ORIG] recurse to fill in the remaining blanks.
            grid[row][col] = 0; // [ORIG] take the guess back so the next digit starts from a clean square.
        }
    }
 
public:
    // [ORIG] Load a puzzle: needs exactly 81 tokens, each a digit 1-9 or "_" for blank.
    bool load(const std::string& filename) {
        std::ifstream input(filename); // [ORIG] open the puzzle file.
        if (!input) return false; // [ORIG] give up if the file is missing or unreadable.
        grid = {}; // [ORIG] clear the board in case an earlier puzzle was loaded.
        solutions = 0; // [ORIG] restart the solution tally at zero.
        std::string token; // [ORIG] holds one entry of the file at a time.
        for (int r = 0; r < 9; ++r) { // [ORIG] nine rows to read.
            for (int c = 0; c < 9; ++c) { // [ORIG] nine entries in each row.
                if (!(input >> token)) return false; // [ORIG] the file ran out early, so it is not a full grid.
                if (token == "_") grid[r][c] = 0; // [ORIG] an underscore becomes a blank (0).
                else if (token.size() == 1 && token[0] >= '1' && token[0] <= '9') // [ORIG] otherwise it must be one character, 1 through 9.
                    grid[r][c] = token[0] - '0'; // [ORIG] turn the character into its numeric value.
                else return false; // [ORIG] anything else is not allowed in a puzzle.
            }
        }
        if (input >> token) return false; // [ORIG] more than 81 entries means the file is malformed.
        for (int r = 0; r < 9; ++r) { // [ORIG] now verify the starting digits do not clash, row by row.
            for (int c = 0; c < 9; ++c) { // [ORIG] and column by column.
                if (grid[r][c] == 0) continue; // [ORIG] blanks cannot clash with anything.
                int value = grid[r][c]; // [ORIG] keep a copy of this starting digit.
                grid[r][c] = 0; // [ORIG] blank it out so allowed() does not compare it against itself.
                bool valid = allowed(r, c, value); // [ORIG] see if it fits the row, column, and box.
                grid[r][c] = value; // [ORIG] put the digit back.
                if (!valid) return false; // [ORIG] two starting digits conflict, so the puzzle is invalid.
            }
        }
        return true; // [ORIG] the file was well formed and the clues are consistent.
    }
 
    // [REV] Write the board to any stream (console or file) using the same layout as the puzzle input.
    void print(std::ostream& out) const {
        for (const auto& row : grid) { // [ORIG] take the board one row at a time.
            for (int c = 0; c < 9; ++c) { // [ORIG] and each column within the row.
                if (c) out << ' '; // [REV] a space goes between entries but not before the first one.
                if (row[c] == 0) out << '_'; // [REV] a blank prints as an underscore.
                else out << row[c]; // [REV] a filled square prints as its digit.
            }
            out << '\n'; // [REV] newline after each of the nine rows.
        }
        out << '\n'; // [REV] an empty line after the grid keeps consecutive grids apart.
    }
 
    // [REV] Run the search and put the results into the named solution file.
    void solve(const std::string& solutionName) {
        solutionFile.open(solutionName); // [REV] open solutionN.txt, replacing any older copy.
        backtrack(); // [ORIG] start the recursive search for all solutions.
        if (solutions == 0) { // [ORIG] the search never reached a full board.
            std::cout << "No solution found\n"; // [ORIG] tell the console user.
            solutionFile << "No solution found.\n"; // [REV] the file must contain only this one line in that case.
        } else { // [REV] at least one full board was found.
            std::cout << "Total solutions: " << solutions << "\n"; // [ORIG] console summary of the count.
            solutionFile << "Solutions found: " << solutions << "\n"; // [REV] the required closing line of the file.
        }
        solutionFile.close(); // [REV] close the stream so everything is flushed to disk.
    }
};
 
// [ORIG] Process the five puzzles one after another with a new solver object each time.
int main() {
    for (int number = 1; number <= 5; ++number) { // [ORIG] puzzle numbers 1 through 5.
        std::string filename = "puzzle" + std::to_string(number) + ".txt"; // [ORIG] name of the input file, e.g. puzzle3.txt.
        std::string solutionName = "solution" + std::to_string(number) + ".txt"; // [REV] name of the matching output file.
        SudokuSolver sudoku; // [ORIG] fresh solver so nothing carries over between puzzles.
        std::cout << "=== " << filename << " ===\n"; // [ORIG] banner showing which puzzle is being handled.
        if (!sudoku.load(filename)) { // [ORIG] try to read and check the puzzle.
            std::cout << "Unable to read puzzle or invalid starting grid.\n\n"; // [ORIG] let the user know the puzzle was rejected.
            std::ofstream bad(solutionName); // [REV] still produce a solution file for this puzzle number.
            bad << "No solution found.\n"; // [REV] a puzzle that cannot be used has no solutions.
            continue; // [ORIG] skip ahead to the next puzzle.
        }
        std::cout << "Original puzzle:\n"; // [ORIG] heading above the starting grid.
        sudoku.print(std::cout); // [REV] display the starting grid on the console.
        sudoku.solve(solutionName); // [REV] search for all solutions and save them in solutionN.txt.
        std::cout << '\n'; // [ORIG] blank line between puzzles on the console.
    }
    return 0; // [ORIG] normal exit.
}
