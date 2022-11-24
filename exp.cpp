#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <iomanip>
#include <math.h>

using namespace std;

// Parses file to save formula information
void parse(string filename, vector<vector<int>>& formula, int& v, int& c) {
    // Save each line
    string line;
    ifstream myfile(filename);

    while(getline(myfile, line)) {
        // Iterate through each word in the line
        if(line.empty()) continue;
        if(line[0] == '\n') continue;
        
        // If first character is c, continue past comment
        if(line[0] == 'c') continue;
        // If first character is %, return
        if(line[0] == '%') return;

        // If first character is p, save variable and clause info
        if(line[0] == 'p') {
            stringstream myline(line.substr(5));
            myline >> v;
            myline >> c;
        }
        // Else, save clause in formula vector
        else {
            stringstream myline(line);
            vector<int> clause;
            int temp;
            while(myline >> temp) {
                clause.push_back(temp);
            }

            // Pop off the last 0
            clause.pop_back();

            if(clause.size()) formula.push_back(clause);
        }

    }
}

// Prints information in formula
void printFormula(vector<vector<int>>& formula, int v, int c) {
    cout << "Variables: " << v << "\tClauses: " << c << endl;
    for(int i = 0; i < formula.size(); ++i) {
        cout << "C" << i+1 << ": ";
        for(int n : formula[i]) cout << n << " ";
        cout << endl;
    }
    cout << endl;
}

// Prints specific clauses of formula
void printSpecificClauses(vector<vector<int>>& formula, unordered_set<int> vars) {
    for(int i = 0; i < formula.size(); ++i) {
        bool varFound = false;
        for(int n : formula[i]) {
            if(vars.find(n) != vars.end() || vars.find(-n) != vars.end()) {
                varFound = true;
                break;
            }
        }

        if(varFound) {
            cout << "C" << i+1 << ": ";
            for(int n : formula[i]) cout << n << " ";
            cout << endl;
        }
    }
    cout << endl;
}


// Counts clauses where a variable from the set appears
int countSpecificClauses(vector<vector<int>>& formula, unordered_set<int> vars) {
    int count = 0;
    for(int i = 0; i < formula.size(); ++i) {
        bool varFound = false;
        for(int n : formula[i]) {
            if(vars.find(n) != vars.end() || vars.find(-n) != vars.end()) {
                varFound = true;
                break;
            }
        }

        if(varFound) ++count;
    }

    return count;
}


// Returns number of positive literals minus number of negative literals for a variable
int balancedSurvey(vector<vector<int>>& formula, int variable) {
    variable = abs(variable);
    int count = 0;
    for(int i = 0; i < formula.size(); ++i) {
        for(int n : formula[i]) {
            if(n == variable) {
                ++count;
                break;
            } else if(n == -variable) {
                --count;
                break;
            }
        }
    }

    return count;
}


// Finds all clauses that a variable is in
vector<int> findClauses(vector<vector<int>>& formula, int var, bool print = false) {
    vector<int> v;
    if(print) cout << "Clauses with var " << var << ": ";
    for(int i = 0; i < formula.size(); ++i) {
        for(int n : formula[i]) {
            if(var == n || var*-1 == n) {
                v.push_back(i+1);
                if(print) cout << i+1 << " ";
            }
        }
    }

    if(print) cout << endl;
    return v;
}

// Finds all "friend" variables for a certain variable
unordered_set<int> findFriends(vector<vector<int>>& formula, int var, bool print = false) {
    unordered_set<int> s;
    if(print) cout << "Friends with var " << var << ": ";
    vector<int> clauses = findClauses(formula, var);
    for(int c : clauses) {
        for(int n : formula[c-1]) {
            if(var != n && var*-1 != n) {
                s.insert(n);
            }
        }
    }

    if(print) {
        for(int v : s) cout << v << " ";
        cout << endl;
    }
    return s;
}

// Find value of a clause (-1 = undetermined, 0 = unsatisfied, 1 = satisfied)
int clauseValue(vector<vector<int>>& formula, unordered_set<int>& s, int clause) {
    bool allZeroes = true;

    for(int v : formula[clause-1]) {
        if(s.find(v) != s.end()) return 1;
        if(s.find(-1*v) == s.end()) allZeroes = false;
    }

    if(allZeroes) return 0;
    return -1;
}

// Return a list of unit clauses
vector<int> unitClauses(vector<vector<int>>& formula, unordered_set<int>& s) {
    vector<int> v;
    for(int i = 0; i < formula.size(); ++i) {
        if(clauseValue(formula, s, i+1) == -1) {
            int unassigned = 0;
            for(int v : formula[i]) {
                if(s.find(v) == s.end() && s.find(-1*v) == s.end()) ++unassigned;
                if(unassigned > 1) break;
            }
            if(unassigned == 1) v.push_back(i+1);
        }
    }
    return v;
}

// Push back clause numbers for unit clauses in a queue
void unitClauses(vector<vector<int>>& formula, unordered_set<int>& s, queue<int>& q) {
    for(int i = 0; i < formula.size(); ++i) {
        if(clauseValue(formula, s, i+1) == -1) {
            int unassigned = 0;
            for(int v : formula[i]) {
                if(s.find(v) == s.end() && s.find(-1*v) == s.end()) ++unassigned;
                if(unassigned > 1) break;
            }
            if(unassigned == 1) q.push(i+1);
        }
    }
}

// Returns unit clauses with one of the three variables as the remaining literal
vector<int> unitClausesHeur(vector<vector<int>>& formula, unordered_set<int>& s, int a, int b, int c) {
    vector<int> v;
    for(int i = 0; i < formula.size(); ++i) {
        if(clauseValue(formula, s, i+1) == -1) {
            int unassigned = 0;
            for(int v : formula[i]) {
                if(s.find(v) == s.end() && s.find(-1*v) == s.end()) {
                    ++unassigned;
                    if(abs(v) != abs(a) && abs(v) != abs(b) && abs(v) != abs(c)) unassigned = 2;
                }
                if(unassigned > 1) break;
            }
            if(unassigned == 1) v.push_back(i+1);
        }
    }
    return v;
}

// Calculate formula's value (-1 = unassigned, 0 = unsatisfied, 1 = satisfied)
int formulaValue(vector<vector<int>>& formula, unordered_set<int>& s, int& conflict, bool print = false) {
    bool allOnes = true;
    for(int i = 0; i < formula.size(); ++i) {
        int clauseVal = clauseValue(formula, s, i+1);

        if(clauseVal == 0) {
            if(print) cout << "Conflict in clause " << i+1 << endl;
            conflict = i+1;
            return 0;
        }
        if(clauseVal != 1) allOnes = false;
    }

    if(allOnes) return 1;
    return -1;
}

int formulaValue(vector<vector<int>>& formula, unordered_set<int>& s, bool print = false) {
    int temp = -1;
    return formulaValue(formula, s, temp, print);
}

// Checks if a certain variable is in a clause
bool varInClause(vector<vector<int>>& formula, int var, int clause) {
    for(int n : formula[clause-1]) if(abs(n) == abs(var)) return true;
    return false;
}

// Returns number of unassigned variables in a clause
int numUnassigned(vector<vector<int>>& formula, unordered_set<int>& s, int clause) {
    int total = 0;
    for(int n : formula[clause-1]) if(s.find(n) == s.end() && s.find(-1*n) == s.end()) ++total;
    return total;
}

// Function to print time
void printTime(clock_t start, clock_t end) {
    double t = (double)(end - start) / (double(CLOCKS_PER_SEC));
    cout << endl << "Time taken: " << fixed << t << setprecision(3);
    cout << " sec" << endl << endl;
}

// Uses survey heuristic to return result of the "vote"
// Only considers unsatisfied clauses
int survey(vector<vector<int>>& formula, unordered_set<int>& s, int v) {
    vector<int> clauses = findClauses(formula, v);

    int result = 0;
    // Iterate through clauses
    for(int c : clauses) {
        // Only consider unsatisfied clauses
        if(clauseValue(formula, s, c) != -1) continue;

        // Iterate through variables in clause
        for(int n : formula[c-1]) {
            if(abs(n) == abs(v)) {
                // Add vote
                if(n < 0) --result;
                else ++result;
            }
        }
    }

    // Return result of the "votes"
    return result;
}


// Solve the formula
// Possible heuristics for decisions: order, random, random survey, max survey, max appearances, jeroslow-wang, mom
bool solve(vector<vector<int>>& formula, unordered_set<int>& s, int vars, string heuristic, bool cdcl, bool print, int& counter) {
    vector<int> stack, decisions;
    queue<int> units;
    unordered_map<int, int> antecedents;

    // Initialize antecedents
    if(cdcl) for(int i = 1; i <= vars; ++i) antecedents[i] = 0;

    int oldCounter = counter;
    while(formulaValue(formula, s) != 1) {
        ++counter;

        int conflict = -1;
        // Check for unsatisfiability to backtrack
        if(formulaValue(formula, s, conflict, print) == 0) {
            // If decisions is empty, return false
            if(decisions.empty()) return false;

            // Add learnt clause
            if(cdcl) {
                vector<int> learnt;
                unordered_set<int> removeRepeats;
                unordered_set<int> seen;
                queue<int> learnq;

                // Iterate through variables in conflicting clause to form queue
                for(int v : formula[conflict-1]) {
                    learnq.push(v);
                    seen.insert(abs(v));
                }

                // Continue learning until queue is empty
                while(!learnq.empty()) {
                    // Remove last element
                    int curr = learnq.front();
                    learnq.pop();
                    seen.insert(abs(curr));

                    // If curr is a decision variable, add to learnt clause
                    if(antecedents[abs(curr)] == 0) {
                        removeRepeats.insert(curr);
                        if(print) cout << "Learnt clause added " << curr << endl;
                    } else {
                        if(print) cout << "Learnt clause explored clause " << antecedents[abs(curr)] << " from variable " << curr << endl;
                        vector<int> a = formula[antecedents[abs(curr)] - 1];
                        for(int v : a) if(seen.find(abs(v)) == seen.end()) learnq.push(v);
                    }
                }

                // Create learnt clause
                for(int v : removeRepeats) learnt.push_back(v);

                // Add learnt clause
                formula.push_back(learnt);

                if(print) {
                    cout << "Learnt clause added: ";
                    for(int n : learnt) cout << n << " ";
                    cout << endl;
                }
            }

            // Remove last decision (it is now a deduction)
            int lastDec = decisions.back();
            decisions.pop_back();
            if(cdcl) antecedents[lastDec] = formula.size();

            // Undo deductions upto lastDec
            while(stack.back() != lastDec) {
                int temp = stack.back();
                if(print) cout << "Undid " << temp << endl;
                s.erase(temp);
                stack.pop_back();
                if(cdcl) antecedents[abs(temp)] = 0;
            }

            // Flip last decision
            int temp = stack.back();
            if(print) cout << "Flipped " << temp << " to " << -1*temp << endl;
            s.erase(temp);
            s.insert(temp * -1);
            stack.pop_back();
            stack.push_back(temp * -1);

            // Clear unit clauses
            units = queue<int>();
        }

        // Choose between decision and deduction
        unitClauses(formula, s, units);

        // Decision
        if(units.empty()) {
            // Max survey heuristic
            if(heuristic == "max survey") {
                // Find variable with the highest survey score (either positive or negative) to set value
                int maxSurvey = 0, maxVar = 0;
                for(int i = 1; i <= vars; ++i) {
                    // Make sure variable is unassigned
                    if(s.find(i) != s.end() || s.find(-1*i) != s.end()) continue;

                    // Find survey score
                    int currSurvey = survey(formula, s, i);

                    // Set max
                    if(abs(maxSurvey) <= abs(currSurvey)) {
                        maxSurvey = currSurvey;
                        if(currSurvey < 0) maxVar = -1 * i;
                        else maxVar = i;
                    }
                }

                // Set value
                s.insert(maxVar);
                stack.push_back(maxVar);
                decisions.push_back(maxVar);
                if(print) cout << "Tried " << maxVar << " with survey score " << maxSurvey << endl;
            }
            // Max appearances heuristic
            else if(heuristic == "max appearances") {
                // Find variable that appears the most
                int maxAppearances = 0, maxVar = 0;
                for(int i = 1; i <= vars; ++i) {
                    // Make sure variable is unassigned
                    if(s.find(i) != s.end() || s.find(-1*i) != s.end()) continue;

                    // Count appearances
                    int a = findClauses(formula, i).size();
                    if(a >= maxAppearances) {
                        maxAppearances = a;
                        maxVar = i;
                    }
                }

                // Set value
                s.insert(maxVar);
                stack.push_back(maxVar);
                decisions.push_back(maxVar);
                if(print) cout << "Tried " << maxVar << " with appearances score " << maxAppearances << endl;
            }
            // Jeroslow-wang method
            else if(heuristic == "jeroslow-wang") {
                // Weights variables in shorter clauses exponentially higher
                double maxSum = 0;
                int maxVar = 0;
                for(int i = 1; i <= vars; ++i) {
                    // Make sure variable is unassigned
                    if(s.find(i) != s.end() || s.find(-1*i) != s.end()) continue;

                    // Add up to sum
                    double currSum = 0;
                    vector<int> currClauses = findClauses(formula, i);
                    // Iterate through clauses
                    for(int c : currClauses) {
                        currSum += pow(2, -1 * numUnassigned(formula, s, c));
                    }

                    // Set max
                    if(maxSum == 0 || currSum >= maxSum) {
                        maxSum = currSum;
                        maxVar = i;
                    }
                }

                // Set value
                s.insert(maxVar);
                stack.push_back(maxVar);
                decisions.push_back(maxVar);
                if(print) cout << "Tried " << maxVar << " with jeroslow-wang score " << maxSum << endl;

            }
            // MOM: maximum occurrences of clauses of minimum size
            else if(heuristic == "mom") {
                // Constant k
                double k = 1;

                // Create a list of smallest clauses
                int smallest = -1;
                for(int i = 0; i < formula.size(); ++i) {
                    int unassigned = numUnassigned(formula, s, i+1);
                    if(unassigned == 0) continue;
                    if(clauseValue(formula, s, i+1) != -1) continue;

                    if(smallest == -1) smallest = unassigned;
                    else smallest = min(smallest, unassigned);
                }
                if(print) cout << "Size of minimum clause set to " << smallest << endl;

                vector<int> smallClauses;
                for(int i = 0; i < formula.size(); ++i) {
                    if(numUnassigned(formula, s, i+1) == smallest) {
                        smallClauses.push_back(i+1);
                    }
                }

                double maxSum = 0;
                int maxVar = 0;
                for(int i = 1; i <= vars; ++i) {
                    // Make sure variable is unassigned
                    if(s.find(i) != s.end() || s.find(-1*i) != s.end()) continue;

                    // Find sum
                    double fx = 0, fx2 = 0;
                    for(int c : smallClauses) {
                        vector<int> currClause = formula[c-1];
                        for(int v : currClause) {
                            if(v == i) ++fx;
                            if(v == -1*i) ++fx2;
                        }
                    }
                    double currSum = ((fx + fx2) * pow(2, k)) + (fx * fx2);

                    // Set max
                    if(currSum >= maxSum) {
                        maxSum = currSum;
                        maxVar = i;
                    }
                }
                // Set value
                s.insert(maxVar);
                stack.push_back(maxVar);
                decisions.push_back(maxVar);
                if(print) cout << "Tried " << maxVar << " with mom score " << maxSum << endl;
            }
            // Random or random survey heuristic
            else if(heuristic.substr(0,6) == "random") {
                // Choose random variable
                int v = rand() % vars + 1;
                while(s.find(v) != s.end() || s.find(-1*v) != s.end()) {
                    ++v;
                    if(v > vars) v = 1;
                }

                // Assign random value
                if(heuristic == "random") {
                    if(rand() % 2 == 0) v *= -1;
                } else if(heuristic == "random survey") {
                    if(survey(formula, s, v) < 0) v *= -1;
                }

                s.insert(v);
                stack.push_back(v);
                decisions.push_back(v);
                if(print) cout << "Tried " << v << " randomly" << endl;
            }
            // In order
            else {
                // Choose the first unassigned variable and assign a value
                for(int i = 1; i <= vars; ++i) {
                    if(s.find(i) == s.end() && s.find(-1*i) == s.end()) {
                        s.insert(i);
                        stack.push_back(i);
                        decisions.push_back(i);
                        if(print) cout << "Tried " << i << " (in order)" << endl;
                        break;
                    }
                }
            }
        }
        // Deduction
        else {
            int unitClause = units.front();
            units.pop();

            // Set variable
            for(int n : formula[unitClause-1]) {
                if(s.find(n) == s.end() && s.find(-1*n) == s.end()) {
                    // Set value depending on literal
                    s.insert(n);
                    stack.push_back(n);
                    if(cdcl) antecedents[abs(n)] = unitClause;
                    if(print) cout << "Unit Clause " << unitClause << " forced " << n << endl;

                    break;
                }
            }
        }

    }

    return true;
}

// Runs the algorithm
double run(string filename, string heuristic, int& counter, bool cdcl, bool print) {

    // Parse file to save formula information
    vector<vector<int>> formula;
    int vars, clauses;
    parse(filename, formula, vars, clauses);

    if(print) printFormula(formula, vars, clauses);


    // Initiate variables
    unordered_set<int> s;

    // Calculate time program took to run
    clock_t start = clock();

    // Solve the formula
    bool satisfied = solve(formula, s, vars, heuristic, cdcl, print, counter);

    clock_t end = clock();

    if(print) cout << endl;

    cout << filename << ":" << endl;
    if(!satisfied) cout << "formula not satisfied" << endl;
    else {
        cout << "formula satisfied with assignment: ";
        for(int i = 1; i <= vars; ++i) {
            if(s.find(i) != s.end()) cout << i << " ";
            else cout << -1*i << " ";
        }
        cout << endl;
    }

    if(print) {
        if(formula.size() != clauses) {
            cout << endl << "Learnt Clauses: " << endl;
            for(int i = clauses; i < formula.size(); ++i) {
                cout << "C" << i+1 << ": ";
                for(int n : formula[i]) cout << n << " ";
                cout << endl;
            }
        }
    }
    clauses = formula.size();

    if(print) cout << "Counter: " << counter << endl;

    // Output time taken
    printTime(start, end);
    cout << endl;

    return ((double)(end - start) / (double(CLOCKS_PER_SEC)));
}


// Converts a number to corresponding file name
string numToFile(int i, int vars = 20) {
    string file;
    if(vars == 17941) {
        file = "uf17941-0";
    } else if(vars == 16883) {
        file = "uf16883-0";
    }
    else if(vars == 600) {
        file = "uf600-0";
    }
    else if(vars == 250) {
        file = "uf250-0";
    }
    else if(vars == 100) {
        file = "uf100-0";
    }
    else if(vars == 75) {
        file = "uf75-0";
    }
    else if(vars == 50) {
        file = "uf50-0";
    }
    else if(vars == 20){
        file = "uf20-0";
    } else {
        cout << "Error in vars numToFile line 642" << endl;
    }
    string digits;

    while(i > 0) {
        char digit = '0' + (i % 10);
        i /= 10;
        digits += digit;
    }

    for(int d = digits.size()-1; d >= 0; --d) {
        file += digits[d];
    }
    return file + ".cnf";
}


// Function to run for all files
void runAll(int min = 1, int max = 1000, int vars = 20, string heuristic = "order", bool cdcl = false, bool print = false) {
    // Calculate time program took to run
    clock_t start = clock();


    string file;

    if(min < 1) {
        file = "example1.txt";
        int counter = 0;

        run(file, heuristic, counter, cdcl, print);
    }
    else {
        if(max > 1000) max = 1000;

        int sum = 0;

        // Loop through the 1000 files
        for(int i = min; i <= max; ++i) {
            // Get file name
            file = numToFile(i, vars);

            int counter = 0;

            cout << i << ". ";
            run(file, heuristic, counter, cdcl, print);

            sum += counter;
        }

        double average = (sum * 1.0) / (max - min + 1);
        cout << endl << endl << "Average iterations for " << heuristic << ": " << average << endl;
    }

    // Calculate time
    clock_t end = clock();
    // Print time taken by program
    printTime(start, end);
}


// Generates combinations of variables
void generateCombinationsH(vector<int>& v, vector<vector<int>>& combinations, vector<int>& c) {
    if(c.size() >= v.size()) {
        combinations.push_back(c);
        return;
    }

    c.push_back(v[c.size()]);
    generateCombinationsH(v, combinations, c);
    c.pop_back();

    c.push_back(-v[c.size()]);
    generateCombinationsH(v, combinations, c);
    c.pop_back();
}

vector<vector<int>> generateCombinations(vector<int> vars) {
    vector<vector<int>> combinations;
    vector<int> c;
    generateCombinationsH(vars, combinations, c);
    return combinations;
}


// Generates combinations of 3 variables given the max number of variables
void nCombosH(vector<vector<int>>& combos, vector<int>& c, int max, int len) {
    if(c.size() == len) {
        combos.push_back(c);
        return;
    }

    int min = 1;
    if(c.size() > 0) min = c.back() + 1;

    for(int i = min; i <= max; ++i) {
        c.push_back(i);
        nCombosH(combos, c, max, len);
        c.pop_back();
    }
}
vector<vector<int>> nCombos(int max, int len = 3) {
    vector<vector<int>> combos;
    vector<int> c;
    nCombosH(combos, c, max, len);
    return combos;
}


// Generates combinations of 3 variables given the variables to use
void nCombosH(vector<vector<int>>& combos, vector<int>& c, vector<int>& nums, int len, int ind = 0) {
    if(c.size() == len) {
        combos.push_back(c);
        return;
    }

    if(ind >= nums.size()) return;

    for(int i = ind; i < nums.size(); ++i) {
        c.push_back(nums[i]);
        nCombosH(combos, c, nums, len, i+1);
        c.pop_back();
    }
}
vector<vector<int>> nCombos(vector<int>& nums, int len = 3) {
    vector<vector<int>> combos;
    vector<int> c;
    nCombosH(combos, c, nums, len);
    return combos;
}

// Randomly form combinations
vector<vector<int>> randomCombos(int vars, int total, int len = 3) {
    vector<vector<int>> combos;

    while(combos.size() < total) {
        int r = rand() % vars + 1;

        int r2 = r;
        while(r2 == r) r2 = rand() % vars + 1;

        if(len == 2) {
            vector<int> temp = {r, r2};
            combos.push_back(temp);
            continue;
        }

        int r3 = r;
        while(r3 == r || r3 == r2) r3 = rand() % vars + 1;

        vector<int> temp = {r, r2, r3};

        if(len == 4) {
            int r4 = r;
            while(r4 == r || r4 == r2 || r4 == r3) r4 = rand() % vars + 1;
            temp.push_back(r4);
        }

        combos.push_back(temp);
    }

    return combos;
}


// Checks if the given variables are in a clause
bool isClause(vector<vector<int>>& formula, unordered_set<int>& s, int& clause) {
    int temp = 1;
    for(vector<int> c : formula) {
        bool newVar = false;
        for(int n : c) {
            if(s.find(n) == s.end() && s.find(-1*n) == s.end()) {
                newVar = true;
                break;
            }
        }

        if(newVar == false) {
            clause = temp;
            return true;
        }
        ++temp;
    }

    return false;
}

bool isClause(vector<vector<int>>& formula, unordered_set<int>& s) {
    int temp = -1;
    return isClause(formula, s, temp);
}


// Sample down the combination vector to a vector of size n
vector<vector<int>> sample(vector<vector<int>>& v, int n) {
    if(n >= v.size()) return v;

    vector<vector<int>> v2(n);
    unordered_set<int> indices = {-1};

    for(int i = 0; i < n; ++i) {
        int r = -1;
        while(indices.find(r) != indices.end()) r = rand() % v.size();
        indices.insert(r);

        v2[i] = v[r];
    }

    return v2;
}


// Count pairs as double for occurrences
int countWeightedClauses(vector<vector<int>>& formula, unordered_set<int>& s) {
    int count = 0;
    for(vector<int> c : formula) {
        for(int n : c) {
            if(s.find(n) != s.end() || s.find(-n) != s.end()) ++count;
        }
    }

    return count;
}


// Counts all clauses which have two out of the three variables from the combination
int countPairs(vector<vector<int>>& formula, unordered_set<int>& s) {
    int count = 0;
    for(vector<int> c : formula) {
        int num = 0;
        for(int n : c) if(s.find(n) != s.end() || s.find(-n) != s.end()) ++num;
        if(num == 2) ++count;
    }
    return count;
}

// Checks if a pair exists (returns false if no pair exists or is already a clause)
bool checkPairClause(vector<vector<int>>& formula, unordered_set<int>& s) {
    int pairs = 0;
    for(vector<int> c : formula) {
        int num = 0;
        for(int n : c) if(s.find(n) != s.end() || s.find(-n) != s.end()) ++num;
        if(num == 3) return false;
        if(num == 2) ++pairs;
    }

    return pairs > 0;
}

int balancedSurveyAll(vector<vector<int>>& formula, unordered_set<int>& s) {
    int count = 0;
    for(int n : s) count += abs(balancedSurvey(formula, n));
    return count;
}


// Finds auxiliary variable based on initial assignment
int findAux(vector<vector<int>>& formula, unordered_set<int>& s, int mode) {
    if(mode == 1) {
        // "inside" appearances

        unordered_set<int> banned;
        unordered_map<int, int> m;

        // Only focus on clauses that have one of the variables from the original combination
        for(int i = 0; i < formula.size(); ++i) {
            int count = 0;
            for(int n : formula[i]) {
                if(s.find(abs(n)) != s.end()) ++count;
            }

            if(count == 1) {
                for(int n : formula[i]) if(s.find(abs(n)) == s.end()) ++m[abs(n)];
            } else if(count == 2) {
                for(int n : formula[i]) if(s.find(abs(n)) == s.end()) banned.insert(abs(n));
            }
        }

        // Add the valid variable that appears the most
        priority_queue<pair<int, int>> pq;
        for(pair<int, int> p : m) pq.push(make_pair(p.second, p.first));

        if(pq.empty()) return 0;

        int aux = -1;
        while(aux < 0) {
            int temp = pq.top().second;
            pq.pop();
            if(banned.find(temp) == banned.end()) aux = temp;
        }

        return aux;
    } else if(mode == 2) {
        // "outside" appearances

        unordered_map<int, int> m;
        
        // Only focus on clauses that have none of the variables from original combination
        int max = 0, result = 0;
        for(int i = 0; i < formula.size(); ++i) {
            bool ban = false;
            for(int n : formula[i]) {
                if(s.find(abs(n)) != s.end()) ban = true;
            }

            if(ban) continue;

            for(int n : formula[i]) {
                ++m[abs(n)];

                if(m[abs(n)] >= max) {
                    max = m[abs(n)];
                    result = abs(n);
                }
            }
        }

        // Return result
        return result;
    } else {
        // Survey

        unordered_set<int> banned;
        unordered_map<int, int> m;

        // Only focus on clauses that have one of the variables from the original combination
        for(int i = 0; i < formula.size(); ++i) {
            int count = 0;
            for(int n : formula[i]) {
                if(s.find(abs(n)) != s.end()) ++count;
            }

            if(count == 1) {
                for(int n : formula[i]) {
                    if(s.find(abs(n)) == s.end()) {
                        if(n > 0) ++m[n];
                        else --m[abs(n)];
                    }
                }
            } else if(count == 2) {
                for(int n : formula[i]) if(s.find(abs(n)) == s.end()) banned.insert(abs(n));
            }
        }

        // Add the valid variable that has the most "lopsided" survey value
        priority_queue<pair<int, int>> pq;
        for(pair<int, int> p : m) pq.push(make_pair(abs(p.second), p.first));

        if(pq.empty()) return 0;

        int aux = -1;
        while(aux < 0) {
            int temp = pq.top().second;
            pq.pop();
            if(banned.find(temp) == banned.end()) aux = temp;
        }

        return aux;
    }
}


// Original heuristic which detects which variable combinations leads to conflicts
bool heur(vector<vector<int>>& formula, vector<vector<int>>& all, bool checkClause, bool print, bool hideAll, int& exp) {
    exp = 0;
    float deductions = 0, count = 0;
    int iterations = 0;

    bool conflictCaused = false;
    for(vector<int> current : all) {
        // Check if assignment is already a clause
        unordered_set<int> s;
        for(int curr : current) s.insert(curr);

        /*
        int clauseFound = -1;
        bool isClauseB = isClause(formula, s, clauseFound);
        if(checkClause) {
            if(isClauseB) continue;
        }*/

        conflictCaused = false;

        // exp = balancedSurveyAll(formula, s);

        // Choose the variables
        if(!hideAll) {
            cout << ++iterations << ". Variables chosen: ";
            for(int curr : current) cout << curr << " ";
        }

        /*if(!checkPairClause(formula, s)) {
           if(!hideAll) cout << " is already a clause or doesn't have a unit propagation " << endl;
            continue;
        }*/

        /* 
        if(!hideAll) if(isClauseB) cout << " is already a clause, clause " << clauseFound << ") ";
        if(!hideAll) cout << endl << endl;
        */

        // Generate variable combinations
        vector<vector<int>> combinations = generateCombinations(current);

        // Iterate through variable combinations/orderings
        for(vector<int> o : combinations) {
            // Make the three assignments
            unordered_set<int> s;
            for(int i : o) s.insert(i);

            // conflictCaused = false;

            if(!hideAll) if(print) {
                cout << "\tVariable assignments: ";
                for(int i : o) cout << i << " ";
                cout << endl;
            }
            
            // Check for unit
            queue<int> units;
            unitClauses(formula, s, units);

            int conflict = -1;
            int val = formulaValue(formula, s, conflict);
            if(val == 1) if(!hideAll) if(print) cout << "\tFormula satisfied" << endl;
            if(val == 0) {
                if(!hideAll) if(print) cout << "\tFormula has a conflict in clause " << conflict << endl;
                if(!hideAll) {
                    cout << "\t\tLearnt Clause: ";
                    for(int i : o) cout << -i << " "; 
                    cout << endl;
                }
                conflictCaused = true;
                ++count;
            }

            if(val != -1) {
                if(!hideAll) if(print) cout << endl;
                continue;
            }

            // Iterate until no unit clauses remain
            int temp = 0;
            while(!units.empty()) {
                int unitClause = units.front();
                units.pop();

                // Set variable
                for(int n : formula[unitClause-1]) {
                    if(s.find(n) == s.end() && s.find(-1*n) == s.end()) {
                        // Set value depending on literal
                        s.insert(n);
                        if(!hideAll) if(print) cout << "\tUnit Clause " << unitClause << " forced " << n << endl;
                        ++temp;
                        break;
                    }
                }

                // Update unit clauses
                if(units.empty()) unitClauses(formula, s, units);

                // Check for formula value
                conflict = -1;
                int val = formulaValue(formula, s, conflict);
                if(val == 1) if(!hideAll) if(print) cout << "\tFormula satisfied" << endl;
                if(val == 0) {
                    if(!hideAll) if(print) cout << "\tFormula has a conflict in clause " << conflict << endl;
                    if(!hideAll) {
                        cout << "\t\tLearnt Clause: ";
                        for(int i : o) cout << -i << " "; 
                        cout << endl;
                    }
                    conflictCaused = true;
                    deductions += temp;
                    ++count;
                }

                if(val != -1) break;
            }
            if(!hideAll) if(print) cout << endl;
            if(conflictCaused) break;
            
        }
        // All orders of the combination done
        if(!hideAll) cout << endl;
        if(!hideAll) if(print) cout << endl;
        // if(conflictCaused) cerr << "\t" << current[0] << " " << current[1] << " " << current[2] << endl;
        if(conflictCaused) ++exp;
        //return conflictCaused;
    }

    return conflictCaused;
}


// Second heuristic which detects which variable combinations leads to conflicts, while also adding an auxiliary variable based on variable combos
bool heur2(vector<vector<int>>& formula, vector<vector<int>>& all, bool checkClause, bool print, bool hideAll, int& exp, int mode = 1, int numAux = 1) {
    exp = 0;
    float deductions = 0, count = 0;
    int iterations = 0;

    bool conflictCaused = false;
    for(vector<int> current : all) {
        // Check if assignment is already a clause
        unordered_set<int> s2;
        for(int curr : current) s2.insert(curr);

        int clauseFound = -1;
        bool isClauseB = isClause(formula, s2, clauseFound);
        if(checkClause) {
            if(isClauseB) continue;
        }

        conflictCaused = false;

        // exp = balancedSurveyAll(formula, s2);

        // Choose the variables
        if(!hideAll) {
            cout << ++iterations << ". Variables chosen: ";
            for(int curr : current) cout << curr << " ";
        }

        if(!hideAll) if(isClauseB) cout << " is already a clause, clause " << clauseFound << ") ";
        if(!hideAll) cout << endl << endl;

        // Add auxiliary variable
        for(int i = 0; i < numAux; ++i) {
            int aux = findAux(formula, s2, mode);
            s2.insert(aux);
            current.push_back(aux);
            if(!hideAll) cout << "Auxiliary Variable: " << aux << endl;
        }
        

        // Generate variable combinations
        vector<vector<int>> combinations = generateCombinations(current);

        // Iterate through variable combinations/orderings
        for(vector<int> o : combinations) {
            // Make the assignments
            unordered_set<int> s;
            for(int i : o) s.insert(i);

            // conflictCaused = false;

            if(!hideAll) if(print) {
                cout << "\tVariable assignments: ";
                for(int i : o) cout << i << " ";
                cout << endl;
            }
            
            // Check for unit
            queue<int> units;
            unitClauses(formula, s, units);

            int conflict = -1;
            int val = formulaValue(formula, s, conflict);
            if(val == 1) if(!hideAll) if(print) cout << "\tFormula satisfied" << endl;
            if(val == 0) {
                if(!hideAll) if(print) cout << "\tFormula has a conflict in clause " << conflict << endl;
                if(!hideAll) {
                    cout << "\t\tLearnt Clause: ";
                    for(int i : o) cout << -i << " "; 
                    cout << endl;
                }
                conflictCaused = true;
                ++count;
            }

            if(val != -1) {
                if(!hideAll) if(print) cout << endl;
                continue;
            }

            // Iterate until no unit clauses remain
            int temp = 0;
            while(!units.empty()) {
                int unitClause = units.front();
                units.pop();

                // Set variable
                for(int n : formula[unitClause-1]) {
                    if(s.find(n) == s.end() && s.find(-1*n) == s.end()) {
                        // Set value depending on literal
                        s.insert(n);
                        if(!hideAll) if(print) cout << "\tUnit Clause " << unitClause << " forced " << n << endl;
                        ++temp;
                        break;
                    }
                }

                // Update unit clauses
                if(units.empty()) unitClauses(formula, s, units);

                // Check for formula value
                conflict = -1;
                int val = formulaValue(formula, s, conflict);
                if(val == 1) if(!hideAll) if(print) cout << "\tFormula satisfied" << endl;
                if(val == 0) {
                    if(!hideAll) if(print) cout << "\tFormula has a conflict in clause " << conflict << endl;
                    if(!hideAll) {
                        cout << "\t\tLearnt Clause: ";
                        for(int i : o) cout << -i << " "; 
                        cout << endl;
                    }
                    conflictCaused = true;
                    deductions += temp;
                    ++count;
                }

                if(val != -1) break;
            }
            if(!hideAll) if(print) cout << endl;
            if(conflictCaused) break;
            
        }
        // All orders of the combination done
        if(!hideAll) cout << endl;
        if(!hideAll) if(print) cout << endl;
        // if(conflictCaused) cerr << "\t" << current[0] << " " << current[1] << " " << current[2] << endl;
        //return conflictCaused;
        if(conflictCaused) ++exp;
    }

    return conflictCaused;
}


// Returns vector of variables that occur the most in the formula
vector<int> mostOccurred(vector<vector<int>>& formula, int vars, int length, int mode) {
        // Variable/Weighted Occurrences
        if(mode == 1) {
            // Find variable occurrences for each var
            unordered_map<int, int> m;
            for(vector<int> c : formula) {
                for(int n : c) ++m[abs(n)];
            }
            priority_queue<pair<int, int>> pq;
            for(pair<int, int> p : m) {
                pq.push(make_pair(p.second, p.first));
            }

            // Take top var combos
            vector<int> topList;
            for(int i = 0; i < length; ++i) {
                topList.push_back(pq.top().second);
                pq.pop();
            }

            return topList;
        }
        // Balanced Survey
        else if(mode == 2) {
            // Find variable occurrences for each var
            unordered_map<int, int> m;
            for(vector<int> c : formula) {
                for(int n : c) {
                    if(n < 0) --m[abs(n)];
                    else ++m[n];
                }
            }
            priority_queue<pair<int, int>> pq;
            for(pair<int, int> p : m) {
                pq.push(make_pair(abs(p.second), p.first));
            }

            // Take top var combos
            vector<int> topList;
            for(int i = 0; i < length; ++i) {
                topList.push_back(pq.top().second);
                pq.pop();
            }

            return topList;
        }
        else {
            vector<int> v;
            return v;
        }
}

// Most Occurred function for pairs, so returns all combinations
vector<vector<int>> mostOccurredPairs(vector<vector<int>>& formula, int vars, int length) {
    // Randomly generate 10000 combinations and pick the 1000 with the highest pair number
    vector<vector<int>> all = nCombos(100, 3);
    all = sample(all, length*5);

    priority_queue<pair<int, int>> pq;
    for(int i = 0; i < all.size(); ++i) {
        unordered_set<int> s = {all[i][0], all[i][1], all[i][2]};
        int pairs = countPairs(formula, s);
        pq.push(make_pair(pairs, i));
    }

    // Take top var combos
    vector<vector<int>> ans;
    for(int i = 0; i < length; ++i) {
        int ind = pq.top().second;
        pq.pop();
        ans.push_back(all[ind]);
    }

    return ans;
}


// Runs the original heuristic
int runHeuristic(int fileNumber, int vs, bool hideClauses = false, bool print = false, bool hideAll = false, int len = 3, int samples = -1) {

    string file = "example1.txt";
    if(fileNumber > 0) file = numToFile(fileNumber, vs);
    string filename = "SATexamples/" + file;

    // Parse file to save formula information
    vector<vector<int>> formula;
    int vars, clauses;
    parse(filename, formula, vars, clauses);

    if(print && !hideAll) printFormula(formula, vars, clauses);

    // Generate all combinations of variables
    vector<vector<int>> all = nCombos(vs, len);
    if(samples > 0) all = sample(all, samples);

    //all = sample(all, 500);

    // Run the heuristic
    int e = 0;
    heur(formula, all, hideClauses, print, hideAll, e);
    return e;
}

int runHeuristic(int fileNumber, int vs, bool hideClauses, bool print, bool hideAll, int len, vector<vector<int>>& all, bool& conflictCaused, int sampleAmount = -1) {
    string file = "example1.txt";
    if(fileNumber > 0) file = numToFile(fileNumber, vs);
    string filename = "SATexamples/" + file;

    // Parse file to save formula information
    vector<vector<int>> formula;
    int vars, clauses;
    parse(filename, formula, vars, clauses);

    if(print && !hideAll && all.size() == 1) {
        unordered_set<int> s;
        for(int i : all[0]) s.insert(i);
        printSpecificClauses(formula, s);
        cout << countSpecificClauses(formula, s) << " clauses" << endl << endl;
    }
    else if(print && !hideAll) printFormula(formula, vars, clauses);

    if(all.empty()) {
        all = nCombos(vars, len);
        if(sampleAmount > 0) all = sample(all, sampleAmount);
    }

    // Run the heuristic
    int e = 0;
    conflictCaused = heur(formula, all, hideClauses, print, hideAll, e);
    return e;
}

float runHeuristic(int fileNumber, int vs, bool hideClauses, bool print, bool hideAll, int len, vector<vector<int>>& all, int sampleAmount = -1) {
    bool temp;
    return runHeuristic(fileNumber, vs, hideClauses, print, hideAll, len, all, temp, sampleAmount);
}


// Heuristic that chooses combinations based on pairs
vector<vector<int>> pairHeuristic(vector<vector<int>>& formula, int vars, int length, int k = 5) {
    vector<vector<int>> combinations;

    // Find variable occurrences for each var
    unordered_map<int, int> m;
    for(vector<int> c : formula) {
        for(int n : c) ++m[abs(n)];
    }
    priority_queue<pair<int, int>> pq;
    for(pair<int, int> p : m) {
        pq.push(make_pair(p.second, p.first));
    }


    int count = 3;
    while(count*(count-1)/2 < length/k) ++count;
    
    unordered_set<int> used;
    while(pq.size() && combinations.size() < length) {
        int most = pq.top().second;
        pq.pop();
        used.insert(most);

        // Focus only on clauses that most appears in, and count variable occurrences in these clauses
        vector<int> clauses;
        unordered_map<int, int> m2;
        for(int i = 0; i < formula.size(); ++i) {
            for(int n : formula[i]) {
                if(abs(n) == most) {
                    clauses.push_back(i);
                    break;
                }
            }
            if(clauses.size() > 0 && clauses.back() == i) {
                for(int n : formula[i]) if(abs(n) != most) ++m2[abs(n)];
            }
        }

        // Count occurrences of all variables in these clauses
        priority_queue<pair<int, int>> pq2;
        for(pair<int, int> p : m2) {
            pq2.push(make_pair(p.second, p.first));
        }

        // Take the variables that occur the most
        
        vector<int> nums;
        for(int i = 0; i < count; ++i) {
            if(pq2.empty()) break;
            nums.push_back(pq2.top().second);
            pq2.pop();
        }

        vector<vector<int>> temp = nCombos(nums, 2);
        for(vector<int> t : temp) {
            //if(used.find(t[0]) != used.end() || used.find(t[1]) != used.end()) continue;

            vector<int> t2 = {most, t[0], t[1]};
            combinations.push_back(t2);
            // cout << t2[0] << " " << t2[1] << " " << t2[2] << endl;
        }

    }

    while(combinations.size() > length) combinations.pop_back();

    return combinations;
}



// Runs heuristic in a loop
void runHeurLoop() {
    vector<int> v;
    vector<vector<int>> all;
    int sum = 0;
    for(int i = 1; i <= 100; ++i) {
        v.push_back(runHeuristic(i, 250, true, true, true, 3, all, 1000));
        cout << i << ": " << v.back() << endl; 
        sum += v.back();
    }

    cout << "Sum: " << sum << endl;
}


// Runs heuristic as experiment
void runHeurExp() {
    vector<vector<int>> all = {{9, 18, 20}};
    runHeuristic(1, 20, true, true, false, 3, all, -1);
}


// Runs experiment on variable occurrences
void runExp2(int problem, int vars, bool print, int sampleAmount) {
    vector<int> noConflicts, conflicts;
    vector<vector<int>> all = nCombos(vars, 3);
    all = sample(all, sampleAmount);

    for(vector<int> v : all) {
        vector<vector<int>> temp = {v};
        bool conflictCaused = false;
        int x = runHeuristic(problem, vars, true, true, true, 3, temp, conflictCaused);
        if(x == -1) continue;

        if(conflictCaused) {
            conflicts.push_back(x);
            if(print) cout << "Variable combo: " << v[0] << " " << v[1] << " " << v[2] << " produced a conflict, " << x << " survey value" << endl;
        }
        else {
            noConflicts.push_back(x);
            if(print) cout << "Variable combo: " << v[0] << " " << v[1] << " " << v[2] << " produced no conflict, " << x << " survey value" << endl;
        }
    }

    long long s1 = 0, s2 = 0;
    for(int n : noConflicts) s1 += n;
    for(int n : conflicts) s2 += n;

    float avg1 = s1 * 1.0 / noConflicts.size(), avg2 = s2 * 1.0 / conflicts.size();
    avg1 = ceil(avg1 * 100.0) / 100.0;
    avg2 = ceil(avg2 * 100.0) / 100.0;
    cout << "No Conflict Average: " << avg1 << endl;
    cout << "Conflicts Average: " << avg2 << endl << endl;
}

// Runs experiment on variable occurrences
void runExp2(int problem, int vars, ofstream& myfile, int sampleAmount) {
    vector<int> noConflicts, conflicts;
    vector<vector<int>> all = nCombos(vars, 3);
    all = sample(all, sampleAmount);
    for(vector<int> v : all) {
        vector<vector<int>> temp;
        temp.push_back(v);
        bool conflictCaused = false;
        int x = runHeuristic(problem, vars, true, true, true, 3, temp, conflictCaused, -1);
        if(x == -1) continue;

        if(conflictCaused) {
            conflicts.push_back(x);
        }
        else {
            noConflicts.push_back(x);
        }
    }

    long long s1 = 0, s2 = 0;
    for(int n : noConflicts) s1 += n;
    for(int n : conflicts) s2 += n;

    float avg1 = s1 * 1.0 / noConflicts.size(), avg2 = s2 * 1.0 / conflicts.size();
    avg1 = ceil(avg1 * 100.0) / 100.0;
    avg2 = ceil(avg2 * 100.0) / 100.0;
    myfile << avg1 << " \t" << avg2 << "\n";
    cout << noConflicts.size() << " " << conflicts.size() << endl << endl;
}


// Parse the conflicting variable combinations
vector<vector<int>> parseConflicts() {
    // Save each line
    string line;
    ifstream myfile("uf100-01conflicts.txt");

    vector<vector<int>> result;
    while(getline(myfile, line)) {
        vector<int> v(3);
        stringstream myline(line);
        myline >> v[0] >> v[1] >> v[2];
        result.push_back(v);
    }

    return result;
}


// Analyze conflicts caused by certain combinations
void reverseEngineer(int problem, int vars, int sampleAmount) {
    /*
    vector<vector<int>> conflicts = parseConflicts();
    vector<int> exp;
    for(vector<int> v : conflicts) {
        for(int n : v) cout << n << " ";
        vector<vector<int>> temp = {v};
        bool conflictCaused = false;
        int x = runHeuristic(problem, vars, true, true, true, 3, temp, conflictCaused, -1);
        exp.push_back(x);
        cout << " \t" << x << endl;
    }*/
    /*
    vector<int> exp;
    vector<vector<int>> all = nCombos(vars, 3);

    for(vector<int> v : all) {
        for(int n : v) cout << n << " ";
        cout << endl;
        vector<vector<int>> temp = {v};
        bool conflictCaused = false;
        int x = runHeuristic(problem, vars, true, true, true, 3, temp, conflictCaused, -1);
        exp.push_back(x);
    }

    int sum = 0;
    for(int n : exp) sum += n;
    float avg = ceil(sum * 100.0 / exp.size()) / 100.0;
    cout << "Average: " << avg << endl;*/
}


// Experiment to test heuristics
void runHeurExp3(int problem, int vars, int mode, bool control, int combs) {
    if(control) {
        vector<vector<int>> all = nCombos(vars, 3);
        all = sample(all, combs);
        int conflicts = 0;
        for(vector<int> v : all) {
            vector<vector<int>> temp = {v};
            bool conflictCaused = false;
            int x = runHeuristic(problem, vars, true, true, true, 3, temp, conflictCaused, -1);
            if(conflictCaused) ++conflicts;
        }
        cout << "Control Conflicts: " << conflicts << "\t";
        cout << "Combinations tried: " << all.size() << endl;
    } else {
        string file = "example1.txt";
        if(problem > 0) file = numToFile(problem, vars);
        string filename = "SATexamples/" + file;

        // Parse file to save formula information
        vector<vector<int>> formula;
        int clauses;
        parse(filename, formula, vars, clauses);

        // Determine combinations
        vector<vector<int>> combos;
        if(mode < 3) {
            // Determine most used vars
            vector<int> most = mostOccurred(formula, vars, 40, mode);

            combos = nCombos(most, 3);
        } else if(mode == 3) {
            combos = mostOccurredPairs(formula, vars, combs);
        } else {
            combos = pairHeuristic(formula, vars, combs, 30);
            //for(vector<int> v : combos) cout << v[0] << " " << v[1] << " " << v[2] << endl;
        }

        // Iterate through all combos
        int conflicts = 0;
        for(vector<int> v : combos) {
            vector<vector<int>> temp = {v};
            int e = 0;
            bool conflictCaused = heur(formula, temp, true, false, true, e);

            if(conflictCaused) ++conflicts;
        }
        cout << "Heuristic Conflicts: " << conflicts << "\t";
        cout << "Combinations tried: " << combos.size() << endl;

    }
    cout << endl;
}


// Runs the original heuristic
int runHeur2(int fileNumber, int vs, vector<vector<int>> all, bool hideClauses = false, bool print = false, bool hideAll = false, int len = 3, int samples = -1, int mode = 1, int numAux = 1) {

    string file = "example1.txt";
    if(fileNumber > 0) file = numToFile(fileNumber, vs);
    string filename = "SATexamples/" + file;

    // Parse file to save formula information
    vector<vector<int>> formula;
    int vars, clauses;
    parse(filename, formula, vars, clauses);

    if(print && !hideAll) printFormula(formula, vars, clauses);

    // Generate all combinations of variables
    if(all.size() == 0) {
        all = nCombos(vars, len);
        if(samples > 0) all = sample(all, samples);
    }

    // Run the heuristic
    int e = 0;
    heur2(formula, all, hideClauses, print, hideAll, e, mode, numAux);
    return e;
}

int runHeur2(int fileNumber, int vs, bool hideClauses = false, bool print = false, bool hideAll = false, int len = 3, int samples = -1, int mode = 1, int numAux = 1) {
    vector<vector<int>> all;
    return runHeur2(fileNumber, vs, all, hideClauses, print, hideAll, len, samples, mode, numAux);
}


// Runs experiment comparing heuristics (original vs aux)
void runAuxExp(int vars = 20, int max = 20, int samples = -1) {
    // Take sample if needed
    vector<vector<int>> all;
    if(samples > 0) {
        all = nCombos(vars, 3);
        all = sample(all, samples);
    }

    vector<int> c, a1, a2, a3;
    for(int i = 1; i <= max; ++i) {
        cout << "Problem " << i << endl;
        c.push_back(runHeuristic(i, vars, true, false, true, 3, all, -1));
        a1.push_back(runHeur2(i, vars, all, true, false, true, 3, -1, 1));
        a2.push_back(runHeur2(i, vars, all, true, false, true, 3, -1, 2));
        a3.push_back(runHeur2(i, vars, all, true, false, true, 3, -1, 3));
        cout << "Control Conflicts: " << c.back() << "\tAux1 Conflicts: " << a1.back() << "\tAux2 Conflicts: " << a2.back() << "\tAux3 Conflicts: " << a3.back() << endl << endl;
    }

    int cs = 0, a1s = 0, a2s = 0, a3s = 0;
    for(int n : c) cs += n;
    for(int n : a1) a1s += n;
    for(int n : a2) a2s += n;
    for(int n : a3) a3s += n;

    float cavg = ceil(cs * 100.0 / c.size()) / 100.0;
    float a1avg = ceil(a1s * 100.0 / a1.size()) / 100.0;
    float a2avg = ceil(a2s * 100.0 / a2.size()) / 100.0;
    float a3avg = ceil(a3s * 100.0 / a3.size()) / 100.0;
    cout << endl << endl;
    cout << "Control Average Conflicts " << cavg << endl;
    cout << "Auxiliary 1 Average Conflicts " << a1avg << endl;
    cout << "Auxiliary 2 Average Conflicts " << a2avg << endl;
    cout << "Auxiliary 3 Average Conflicts " << a3avg << endl;
}

// Experiment for all methods of choosing 4 variables
void choose4Exp(int vars, int max, int samples = -1) {

    // Take sample if needed
    vector<vector<int>> all4;
    if(samples > 0) {
        all4 = randomCombos(vars, samples, 4);
    }
    vector<vector<int>> all3;
    if(samples > 0) {
        all3 = randomCombos(vars, samples, 3);
    }
    vector<vector<int>> all2;
    if(samples > 0) {
        all2 = randomCombos(vars, samples, 2);
    }

    vector<int> r4, h4, r3a, h3a, r2a2, h2a2;
    for(int i = 1; i <= max; ++i) {
        cout << "Problem " << i << endl;

        string file = numToFile(i, vars);
        string filename = "SATexamples/" + file;

        // Parse file to save formula information
        vector<vector<int>> formula;
        int clauses;
        parse(filename, formula, vars, clauses);

        // Determine combinations
        vector<vector<int>> combos4, combos3, combos2;
        // Determine most used vars
        vector<int> most = mostOccurred(formula, vars, min(vars, 60), 1);

        combos4 = nCombos(most, 4);
        combos3 = nCombos(most, 3);
        combos2 = nCombos(most, 2);
        while(combos4.size() > samples) combos4.pop_back();
        while(combos3.size() > samples) combos3.pop_back();
        while(combos2.size() > samples) combos2.pop_back();

        
        r4.push_back(runHeuristic(i, vars, true, false, true, 4, all4, -1));
        cout << "R4: " << r4.back() << endl;
        h4.push_back(runHeuristic(i, vars, true, false, true, 4, combos4, samples));
        cout << "H4: " << h4.back() << endl;
        r3a.push_back(runHeur2(i, vars, all3, true, false, true, 3, -1, 3));
        cout << "R3A: " << r3a.back() << endl;
        h3a.push_back(runHeur2(i, vars, combos3, true, false, true, 3, -1, 3));
        cout << "H3A: " << h3a.back() << endl;
        r2a2.push_back(runHeur2(i, vars, all2, true, false, true, 3, -1, 3, 2));
        cout << "R2A2: " << r2a2.back() << endl;
        h2a2.push_back(runHeur2(i, vars, combos2, true, false, true, 3, -1, 3, 2));
        cout << "H2A2: " << h2a2.back() << endl << endl;

    }

    int s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0;
    for(int n : r4) s1 += n;
    for(int n : h4) s2 += n;
    for(int n : r3a) s3 += n;
    for(int n : h3a) s4 += n;
    for(int n : r2a2) s5 += n;
    for(int n : h2a2) s6 += n;

    float r4avg = ceil(s1 * 100.0 / r4.size()) / 100.0;
    float h4avg = ceil(s2 * 100.0 / h4.size()) / 100.0;
    float r3aavg = ceil(s3 * 100.0 / r3a.size()) / 100.0;
    float h3aavg = ceil(s4 * 100.0 / h3a.size()) / 100.0;
    float r2a2avg = ceil(s5 * 100.0 / r2a2.size()) / 100.0;
    float h2a2avg = ceil(s6 * 100.0 / h2a2.size()) / 100.0;
    cout << endl;
    cout << "R4 Average Conflicts " << r4avg << endl;
    cout << "H4 Average Conflicts " << h4avg << endl;
    cout << "R3A Average Conflicts " << r3aavg << endl;
    cout << "H3A Average Conflicts " << h3aavg << endl;
    cout << "R2A2 Average Conflicts " << r2a2avg << endl;
    cout << "H2A2 Average Conflicts " << h2a2avg << endl;

}

// Helper function to find clusters
void findClusters(int root, int height, unordered_map<int, set<int>>& m, vector<vector<int>>& formula, vector<set<int>>& h, int currHeight = 0) {
    if(currHeight >= height) return;

    root = abs(root);

    // Set height
    h[currHeight].insert(root);

    // Find all friends for the root variable
    for(vector<int> c : formula) {
        bool friends = false;
        for(int n : c) {
            if(abs(n) == root) {
                friends = true; 
                break;
            }
        }

        if(friends) {
            for(int n : c) {
                if(abs(n) == root) continue;

                m[root].insert(abs(n));
            }
        }
    }

    // Recursive call on all children
    for(int f : m[root]) {
        findClusters(f, height, m, formula, h, currHeight+1);
    }
}



// Converts a number to corresponding file name
string numToBigFile(int i) {
    string file = "BigFiles/bigfile";
    if(i < 10) file += '0';

    string digits;

    while(i > 0) {
        char digit = '0' + (i % 10);
        i /= 10;
        digits += digit;
    }

    for(int d = digits.size()-1; d >= 0; --d) {
        file += digits[d];
    }
    return file + ".cnf";
}


// Large Files testing
void largeFilesTesting(int start, int end, int mode = 1) {
    vector<int> v, c;

    for(int count = start; count <= end; ++count) {
        string filename = numToBigFile(count);
        int vars = 0, clauses = 0;
        vector<vector<int>> formula;

        parse(filename, formula, vars, clauses);
        v.push_back(vars);
        c.push_back(clauses);

        cout << "File " << count << "\tVars: " << vars << "\tClauses: " << clauses << endl;

        if(mode == 1) {
        
            // Measure clause lengths
            unordered_map<int, int> sizes;
            for(vector<int> c : formula) {
                ++sizes[c.size()];
            }
            for(pair<int, int> p : sizes) {
                cout << "Size " << p.first << ": \t" << p.second << endl;
            }

        }
        else if(mode == 2) {

        
            // Measure variable occurrences
            unordered_map<int, int> occurrences;
            for(vector<int> c : formula) {
                for(int n : c) {
                    ++occurrences[abs(n)];
                }
            }
            // Number for each occurrence
            unordered_map<int, int> occurrences2;
            for(pair<int, int> p : occurrences) {
                ++occurrences2[p.second];
            }
            // Order pairs
            priority_queue<pair<int, int>> pq;
            for(pair<int, int> p : occurrences2) pq.push(make_pair(p.second, p.first));
            cout << "Variable Occurrences \t Number of Such Variables" << endl;
            while(pq.size()) {
                cout << "\t" << pq.top().second << "\t\t\t" << pq.top().first << endl;
                pq.pop();
            }
        
        }
        else if(mode == 3) {
        
            // Measure balance of variables
            unordered_map<int, int> balance;
            for(vector<int> c : formula) {
                for(int n : c) {
                    if(n > 0) ++balance[n];
                    else --balance[abs(n)];
                }
            }
            // Occurrences for each balance value
            unordered_map<int, int> values;
            for(pair<int, int> p : balance) {
                ++values[p.second];
            }
            // Order pairs
            priority_queue<pair<int, int>> pq;
            for(pair<int, int> p : values) pq.push(make_pair(p.second, p.first));
            cout << "Variable Balance Value \t Number of Such Variables" << endl;
            while(pq.size()) {
                cout << "\t" << pq.top().second << "\t\t\t" << pq.top().first << endl;
                pq.pop();
            }
        
        } else if(mode == 4) {
            // Randomly sample variables for balance
            unordered_set<int> used;

            cout << "Variable \t   Positive \t   Negative \t     Balance" << endl;
            for(int j = 0; j < 1000; ++j) {
                // Pick random variable that hasn't been used yet
                int r = rand() % vars + 1;
                while(used.find(r) != used.end()) r = rand() % vars + 1;

                // Count occurrences
                int pos = 0, neg = 0;
                for(vector<int> c : formula) {
                    for(int n : c) {
                        if(n == r) ++pos;
                        else if(n == -1*r) ++neg;
                    }
                }

                // r has been used
                used.insert(r);

                // Print out results
                cout << r << "\t\t\t" << pos << "\t\t" << neg << "\t\t" << (pos-neg) << endl;

            }


        } else if(mode == 5) {
            // Filter out variables based on balance

            // Count occurrences for all variables
            unordered_map<int, int> m;
            for(vector<int> c : formula) {
                for(int n : c) ++m[n];
            }

            ofstream myfile("occurrences2.txt");

            myfile << "File: " << count << "\tVariables: " << vars << "\tClauses: " << clauses << endl << endl;
            myfile << "Variable \t   Positive \t   Negative \t     Balance" << endl;
            for(int r = 1; r <= vars; ++r) {
                
                int pos = m[r];
                int neg = m[-r];
                int bal = pos - neg;

                // Skip if result is common
                // if(bal >= -1 && bal <= 1) continue;

                // Print out results
                myfile << r << "\t\t\t" << pos << "\t\t" << neg << "\t\t" << bal << endl;

            }
        } else if(mode == 6) {
            // Clusters
            
            // Randomly choose a root/start variable
            int root = rand() % vars + 1;

            // Define the height (how many recursive calls to make)
            int height = 3;

            // Save information in a map from each variable to all it's friends
            unordered_map<int, set<int>> m;

            // Associates each variable with its height in the tree
            vector<set<int>> h(height);

            // Recurse
            findClusters(root, height, m, formula, h);

            ofstream myfile("clusters1.txt");

            // Print information in order of height
            myfile << endl << endl;

            for(int i = 0; i < h.size(); ++i) {
                for(int r : h[i]) {
                    myfile << "Var: " << r << "\tHeight: " << i << endl;
                    myfile << "Friends: ";
                    for(int f : m[r]) myfile << f << " ";
                    myfile << endl << endl;
                }
                myfile << endl << endl;
            }

        } else if(mode == 7) {
            // Cluster Cycles

            // Randomly choose a clause, and a variable
            int rootC = rand() % clauses;

            int r = rand() % formula[rootC].size();
            int root = abs(formula[rootC][r]) ;

            // Maintain a list of explored nodes
            unordered_set<int> explored;

            for(int n : formula[rootC]) if(abs(n) != root) explored.insert(abs(n));

            // Maintain the level of exploration
            int level = 1;

            unordered_set<int> done;
            bool allExplored = false;
            // Iterate until root has been explored
            while(explored.find(root) == explored.end()) {
                unordered_set<int> explored2;

                // Explore are neighbors
                while(explored.size()) {
                    int n = *(explored.begin());

                    cout << "Explored: ";
                    for(int e : explored) cout << e << " ";
                    cout << endl;

                    for(int i = 0; i < formula.size(); ++i) {
                        if(i == rootC) continue;
                        vector<int> c = formula[i];
                        bool found = false;
                        for(int n2 : c) {
                            if(abs(n2) == n) {
                                found = true;
                                break;
                            }
                        }

                        if(found) {
                            for(int n2 : c) {
                                if(abs(n2) != n && done.find(abs(n2)) == done.end()) explored2.insert(abs(n2));
                            }
                        }
                    }

                    // Done exploring that neighbor
                    done.insert(n);
                    explored.erase(n);

                    if(done.size() == vars-1) {
                        allExplored = true;
                        break;
                    }
                }
                // Iterate level
                ++level;

                if(allExplored) break;

                // Set explored again
                explored = explored2;
            }

            cout << "Var: " << root << "\tCycle Num: " << level << endl;
            if(allExplored) cout << "allExplored" << endl;

        }
       

       cout << endl;
    }

    cout << endl << endl;
    /*
    float avgVars = 0, avgClauses = 0;
    for(int i = 0; i < v.size(); ++i) {
        avgVars += v[i], avgClauses += c[i];
    }
    avgVars /= v.size(), avgClauses /= c.size();
    cout << "Average Vars: " << avgVars << endl << "Average Clauses: " << avgClauses << endl << endl;*/
}

// Solver experiment
void solverExp(int vars, int fileNum, int trials=10, ostream& myfile = cout) {

    vector<double> v;
    double total = 0;
    for(int i = 1; i <= trials; ++i) {
        // filename of formula
        //string folder = "SATexamples/";
        string folder = "50Vars/";
        string filename = folder + numToFile(fileNum, vars);

        // Use random heuristic
        // Other options: max survey, max appearances, random, random survey, order, jeroslow-wang, mom
        string heuristic = "random";

        int counter = 0;

        // Call run() to solve
        double t = run(filename, heuristic, counter, false, false);
        v.push_back(t);
        total += t;

        if(i == 1) myfile << filename << ": " << endl;
        myfile << "Trial " << i << ": " << fixed << t << setprecision(2) << endl;
    }

    double avg = total/trials;
    double sd = 0;
    for(int n : v) sd += abs(n - avg)*abs(n-avg);
    sd /= (trials-1);
    sd = sqrt(sd);
    
    myfile << endl << "Average Time Taken: " << fixed << ( avg ) << setprecision(2) << endl;
    myfile << endl << "Standard Deviation: " << fixed << sd << setprecision(2) << endl << endl;
}


void clusterExp(int vars, int i, int root = 0) {
    string file = numToFile(i, vars);
    string filename = "SATexamples/" + file;

    // Parse file to save formula information
    vector<vector<int>> formula;
    int clauses;
    parse(filename, formula, vars, clauses);


    // Cluster Cycles

    // Randomly choose a clause
    int rootC = rand() % clauses;

    int r = rand() % formula[rootC].size();
    root = abs(formula[rootC][r]) ;

    cout << "Root: " << root << endl;

    // Maintain a list of explored nodes
    unordered_set<int> explored;

    for(int n : formula[rootC]) if(abs(n) != root) explored.insert(abs(n));

    // Maintain the level of exploration
    int level = 1;

    unordered_set<int> done;
    bool allExplored = false;
    // Iterate until root has been explored
    while(explored.find(root) == explored.end()) {
        unordered_set<int> explored2;

        // Explore are neighbors
        cout << "Level: " << level << endl;
        while(explored.size()) {
            int n = *(explored.begin());
            cout << "n: " << n << "\t";

            cout << "Newly Explored: ";
            for(int i = 0; i < formula.size(); ++i) {
                if(i == rootC) continue;
                vector<int> c = formula[i];
                bool found = false;
                for(int n2 : c) {
                    if(abs(n2) == n) {
                        found = true;
                        break;
                    }
                }

                if(found) {
                    for(int n2 : c) {
                        if(abs(n2) != n && done.find(abs(n2)) == done.end()) {
                            explored2.insert(abs(n2));
                            cout << abs(n2) << " ";
                        }
                    }
                }
            }

            // Done exploring that neighbor
            done.insert(n);
            explored.erase(n);
            cout << endl;

            if(done.size() == vars-1) {
                allExplored = true;
                break;
            }
        }
        // Iterate level
        ++level;
        cout << endl;

        if(allExplored) break;

        // Set explored again
        for(int e : explored2) explored.insert(e);
    }

    cout << "Var: " << root << "\tCycle Num: " << level << endl;
    if(allExplored) cout << "allExplored" << endl;
    
    cout << endl;
}


int main() {
    // Initialize random seed
    srand(time(0));

    // Choose heuristic
    // max survey, max appearances, random, random survey, order, jeroslow-wang, mom
    // runAll(1, 1000, 20, "max survey", false, false);

    // Run heuristic
    // runHeurLoop();

    // Run heuristic once
    // runHeurExp();

    // Run experiment
    /*
    int vars = 250;
    int sample = 5000;
    ofstream myfile("out1.txt");
    myfile << vars << " vars\n";
    for(int i = 1; i <= 20; ++i) {
        cout << "Problem " << i << endl;
        runExp2(i, vars, myfile, sample);
        //runExp2(i, vars, true, sample);
    }
    myfile.close();*/

    // Reverse Engineer the conflicts
    // reverseEngineer(1, 250, -1);

    // Run heuristic exp
    /*
    int vars = 100;
    int mode = 4;
    int combs = 10000;
    for(int i = 1; i <= 10; ++i) {
        cout << "Problem " << i << endl;
        runHeurExp3(i, vars, mode, false, combs);
        runHeurExp3(i, vars, mode, true, combs);
    }*/

    // Runs auxiliary heuristic
    // vector<vector<int>> all = {{1, 2, 3}};
    // cout << runHeur2(1, 50, true, false, true, 3, -1) << endl;

    // Runs experiment for two heuristics (original vs. auxiliary)
    /*
    int vars = 250;
    int sample = 10000;
    int max = 10;
    runAuxExp(vars, max, sample);*/

    // Experiment for all ways of choose 4 variables
    // choose4Exp(250, 10, 10000);

    // Test large files
    //largeFilesTesting();

    //largeFilesTesting(1, 1, 7);


    // Solver
    
    int vars = 50;
    for(int i = 1; i <= 10; ++i) {
        // filename of formula
        //string folder = "SATexamples/";
        string folder = "50Vars/";
        string filename = folder + numToFile(i, vars);

        // Use random heuristic
        // Other options: max survey, max appearances, random, random survey, order, jeroslow-wang, mom
        string heuristic = "random";

        int counter = 0;

        // Call run() to solve
        run(filename, heuristic, counter, false, false);
    }

    // Solver experiment
    /*
    ofstream myfile("50VarInfo.txt");
    for(int i = 9; i <= 33; ++i) {
        //solverExp(50, i, 10, myfile);
    }
    int counter;
    run("SATexamples/uf100-01.cnf", "random", counter, false, false);*/


    // Cluster cycles
    // clusterExp(50, 1);


    return 0;
}
