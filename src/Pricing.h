#ifndef _PRICING_H__
#define _PRICING_H__

#include "WTA.h"
#include "Node.h"
#include "AlgorithmParameter.h"


/**
 * @brief use this class to takedown the state of the Pricing 
*/
class Label {
public:
    Label() = default;
    Label(const Label & _parent, int j);
    ~Label() = default;
public:
    Label *parent;
    int cur;
    int serviceStartTime;
    int load;
    double cost;
    std::vector<int> onboardOrders;
    std::vector<bool> unreachableOrders; // inst.n+1

    bool pruned; // prune by bound or is dominated
};

// auto CompareLabelAscending = [](const Label *a, const Label *b) 
// {
//     return a->cost > b->cost;
// };

class Pricing {
public:
   Pricing();
   Pricing(GRM* _grm, AlgoParameter _parameter);
   ~Pricing();

   
   void Set(Node &node);
   void Solve(vector<double> &dual);

   void get_and_split_dual(vector<double>& dual);
   void find_all_sp_sol(int _seed, int _is_frac);
   void pick_validate_scenes();

   void __split_dual(int current_block_size, double* current_dual_block, int& dual_check_pos);
 
   // Use this to print the splited dual vals
   void print_splitted_dual();

   // Use this function to clear all the vec data, because all the SubProb only has one instance
   void clear_vec_member(); 
   // void extend(const Label &parent);
   // bool dominance(int j);

public:
   //class memeber for Branch and Pricing
   GRM* grm;
   AlgoParameter parameter;

   int sp_num;
   bool* is_rcLeqZero;
   bool* is_spCorrect;

   int dual_value_num;
   double* dual_vals;
   
   //class member used to solve GRM problem
   int dual_block_num;
   vector<int> splited_dual_block_size;
   vector<double*> splited_dual_vals;
   
   vector<Scene_SSL> all_sp_ssl;
   vector<Scene_SSL> valid_scenes_ssl;

   bool is_consider_time_dual;
   bool is_check_correctness;

/**
 * @category 1 : initalize, find a feasible solution
 * @category 2 : in CG
 * genereted scenes with negative reduced cost, 
 * if empty, means the Subproblem has reach the optimal 
*/
   // class member for branch and bound
   Node *ptrNode;
   std::vector<std::vector<Label>> states; // @todo What is this?
};


#endif //ORA_PRICING_H
