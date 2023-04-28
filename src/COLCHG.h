#ifndef _COL_CHANGE_H__
#define _COL_CHANGE_H__

#include <unordered_map>
#include <vector>
#include <ilcplex/cplex.h>
#include "Highs.h"

class SOLVERLP{

};

class SOLVEREnvPtr{

};


// The BCHG class represents a bound change, usually occurring in a branch-and-bound algorithm,
// when adding new bounds or changing old bounds for some variables.
class BCHG
{
public:
    int colpos_;      // Column index of the bound change
    bool islb_;       // Indicates whether the lower bound is changed
    double old_bound_;// Bound before the change
    double new_bound_;// Bound after the change

    // Constructor that initializes the bound change parameters
    BCHG(int colpos, bool islb, double old_bound, double new_bound)
        : colpos_(colpos), islb_(islb), old_bound_(old_bound), new_bound_(new_bound)
    {
    }
};


// The COLCHG class represents all bound changes and column changes,
// and records all bound change and column change information for the current node.
class COLCHG
{
public:
    std::unordered_map<int, int> lb_indices_; // key is the modified lb column in LP, value is the index in bchgs
    std::unordered_map<int, int> ub_indices_; // key is the modified ub column in LP, value is the index in bchgs
    std::vector<BCHG> bchgs_;                 // Stores all bound changes
    int old_col_num_;                         // Column count before adding columns
    int new_col_num_;                         // Column count after adding columns
    std::vector<int> old_lp_basis_;           // Column indices for the basis inherited from the parent node, used for warm-start
    std::vector<int> new_lp_basis_;           // Column indices for the basis after solving LP

    // Default constructor
    COLCHG();

    // Constructor that initializes with an LP solver
    COLCHG(SOLVERLP* _lp);

    // Revert the changes
    bool BackAllchg(COLCHG* _col_chg);

    // Add a bound change and apply it to the LP
    bool AddBchg(SOLVERLP* _lp, int _colpos, bool _islb, double _new_bound, bool _is_chg);

    // Add a bound change without applying it to the LP
    bool AddBchg(int _colpos, bool _islb, double _old_bound, double _new_bound_child);

    // Check if bound changes are consistent with the LP
    void CheckBoundChg(SOLVERLP* _lp);

    // Get the number of bound changes
    int GetSizeBchg();

    // Get the information of a bound change
    bool GetBchg(int _i, int& _pos, bool& _islb, double* _old_bound, double* _new_bound);

    // Find the indices of bound changes for a given column index
    void FindIndBchg(int _pos, int& _lbind, int& _ubind);

    // Print the bound change information
    bool PrintBchg();

    // Add a column change
    bool AddColchg(int _old_col_num, int _new_col_num);

    // Get the column indices of the basis from the SOLVERLP* _lp
    void GetLPBasis(SOLVERLP* _lp);

    // Set the column indices of the basis in the SOLVERLP* _lp
    void SetLPBasis(SOLVERLP* _lp);
};

#endif
