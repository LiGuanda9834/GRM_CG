#ifndef _STATISTIC_H__
#define _STATISTIC_H__

#include <vector>

enum Status
{
   Status_Ok,                             // Normal status
   Status_Error,                          // Solver error occurred and stopped
   Status_Infeasible,                     // Problem is infeasible
   Status_Optimal,                        // Feasible and solved successfully
   Status_Unbounded,                      // Unbounded
   Status_TimeInterrupt,                  // Time limit exceeded
   Status_NodeInterrupt,                  // Node limit exceeded
   Status_GapInterrupt,                   // Preset gap reached
   Status_MemInterrupt,                   // Memory limit exceeded
   Status_UserInterrupt                   // Interrupted by user
};
typedef enum Status BAPStatus;

enum Stage
{
   Stage_Reading,                          // Reading
   Stage_Readed,                           // Reading finished
   Stage_Presolving,                       // Presolving
   Stage_Presolved,                        // Presolving finished
   Stage_Rooting,                          // Solving root node
   Stage_Rooted,                           // Root node solved
   Stage_Looping,                          // Solving child nodes
   Stage_Looped,                           // Child nodes solved
   Stage_NodeSelecting,                    // Node selecting
   Stage_NodeSelected,                     // Node selected
   Stage_LPing,                            // Solving LP
   Stage_LPed,                             // LP solved
   Stage_Branching,                        // Branching
   Stage_Branched,                         // Branching finished
   Stage_Heuring,                          // Heuristic
   Stage_Heured,                           // Heuristic finished
   Stage_Cutting,                          // Cutting plane
   Stage_Cutted,                           // Cutting plane finished
   Stage_Probing                           // Probing in progress
};
typedef enum Stage BAPStage;

enum Time_stage
{
   Time_Total,                     // Total time
   Time_ReadMps,                   // Time to read MPS file
   Time_ReadCmipSet,               // Time to read CmipSet
   Time_WriteMPS,                  // Time to write MPS file
   Time_WriteLp,                   // Time to write LP file
   Time_Presolve,                  // Time for presolve
   Time_Presolve_Dual,             // Time for solving dual problem in presolve
   Time_Solve,                     // Time for solving
   Time_Root,                      // Time for solving root node
   Time_RootLP,                    // Time for solving LP of root node
   Time_NodeLP,                    // Time for solving node LP
   Time_Cutting,                   // Time for cutting plane
   Time_Heur,                      // Time for heuristic
   Time_Branch,                    // Time for branching
   Time_Nodesel,                   // Time for node selection
   Time_UpdateCmipTreePath,        // Time for updating Cmip tree path
   Time_PrintGAP                   // Time for printing GAP
};
typedef enum Time_stage BAP_Time;

class Statistics
{
public:
    BAPStage current_stage_;            // Current stage
    BAPStatus solution_status_;         // Solution status
    Time_stage time_stage_;             // Time stage (for time statistics)
    int total_nodes_considered_;        // Total number of nodes considered
    int active_nodes_count_;            // Number of currently active nodes
    std::vector<double> time_stages_;   // Time spent in each stage


    int total_branches_;                // Total number of branches
    int total_cuts_;                    // Total number of cutting planes
    int total_heuristics_;              // Total number of heuristics
    int total_nodes_pruned_;            // Total number of pruned nodes

    // Constructor
    Statistics()
        : current_stage_(Stage_Rooting), solution_status_(Status_Ok),
          time_stage_(Time_Total), total_nodes_considered_(0),
          active_nodes_count_(0), time_stages_(static_cast<int>(Time_PrintGAP) + 1, 0),
          total_branches_(0), total_cuts_(0), total_heuristics_(0), total_nodes_pruned_(0)
    {
    }
    // Other member functions can be added as needed
};

#endif