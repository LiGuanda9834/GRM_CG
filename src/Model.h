#ifndef _MODEL_H__
#define _MODEL_H__


#include "Master.h"
#include "Pricing.h"
#include "Solution.h"
#include "Tree.h"
#include "WTA.h"
#include "AlgorithmParameter.h"
#include "Tree.h"
#include "Node.h"
#include "Statistics.h"


class BranchAndCut {
public:
    // Constructor
    BranchAndCut(const AlgoParameter &parameter);
    // Destructor
    virtual ~BranchAndCut() = default;
    // Run the entire algorithm
    void Run();

protected:
    // Solve the root node
    virtual void SolveRootNode(Node &node);
    // Solve the LP at the specified node
    virtual void Solve(Node &node);
    // Initialize the columns
    virtual bool InitialColumns(Node &node);
    // Column generation method
    virtual bool ColumnGeneration(Node &node);
    // Solve the LP
    virtual void SolveLP();

    // Branch at the specified parent node
    void Branch(Node &parent);

    // Heuristic method to find a feasible solution
    void IncumbentHeuristic(Node &node);
    // Record the time
    void record_time(clock_t start_time, clock_t AddCol_finished_time, clock_t master_finished_time, clock_t subProb_finished_time, int DEBUG_LEVEL);
    // Print the current dual solution
    void print_current_dual();

    // Determine if the time limit has been reached
    inline bool TimeLimit() const;
    // Determine if an optimal solution has been found
    inline bool Optimal() const;
    // Calculate the gap between the current optimal solution and lower bound
    inline double Gap() const;

    // Print the information of the specified node
    void Print(Node &node, bool diving = false) const;

    // Output performance metrics
    void OutputPerformanceMetrics();

    // Unused member functions
    void Branch() {}
    void nodeSelect() {}
    void updateTreePath() {}
    void checkLPSol() {}

public:
    // Algorithm parameters
    const AlgoParameter &parameter;
    // Branch and bound tree
    Tree* tree;

    // Current dual solution (used for passing from master to pricing problem)
    vector<double> dual;

    // Column and row types
    Col *col;
    Row *row;

    // Global upper and lower bounds
    double globalUb;
    double globalLb;

    // Node and cut counters
    long nodeCnt;
    int cutsCnt;

    // Time spent on various parts of the algorithm
    double timeOnHeur;
    double timeOnAddCol;
    double timeOnMaster;
    double timeOnPricing;
    double timeOnCG;


    // Statistics object
    Statistics stat_;

    // Timer
    compute::Timer timer;
    // Root node upper and lower bounds
    double rootUb;
    double rootLb;

    // Flush-related member functions
    void Flush();
    void FlushChgCols();
    void FlushAddCols();

    // Pruning-related member functions
    void infPruning();    // Prune due to infeasibility
    void errorPruning();  // Prune due to error
    void fPruning();      // Prune due to integer solution
    void Pruning();       // Prune due to not good enough
    void pathPruning();   // Prune due to parent
};
#endif