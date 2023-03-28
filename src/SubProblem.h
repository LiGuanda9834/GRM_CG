#ifndef _SUBPROBLEM_H__
#define _SUBPROBLEM_H__

#include "WTA.h"
#include "Scene.h"
#include "AlgorithmParameter.h"
#include <vector>

#include <ilcplex/cplex.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <algorithm>

#define WTA_EPS (1e-6)

class SubProblem{
public:
   /**
    * 1 for solved and optval >= 0
    * 0 for solved and optval < 0
   */
    double          sub_optval;
    int             sub_status;
    GRM*            grm;

    // variables to describe WTA model
    int             ssl_num;
    int             target_index;

    int             weapon_num_m_;
    int             radar_num_k_;

    double*         weapon_dual_ui;
    double*         radar_dual_muk;
    double          time_dual_wj;
    double          target_dual_vj;




   // target value = V_j
    double          target_value;
    double*         j_prob_vector;

    // varible used in LP solve, cutind, cutval and rhs is the OA linear constraint created by x
    double*         x;
    int*            cutind;
    double*         cutval;
    double          rhs;




    /**
     * @brief   to set algorithm param
     * @todo    Add these params to the parameter of total algorithm
    */ 
    int             seed;
    int             separate_fractional_solutions;
    /**
     * @brief   Is any bug here
     * 0 for no bug
    */

    AlgoParameter*  parameter;

public:
    SubProblem() = default;
    ~SubProblem() = default;

    void Init(GRM* grm_, int t_index, double* w_dual_ui, double* r_dual, double time_dual, double t_dual_vj, int seed, int is_fractional, AlgoParameter* param);

    double destroy_prob();

    // cal fx and cutval is depend on the member variable x. Only the x is set, this two function could be correct. 
    double cal_fx();
    double cal_cutval(int i);
    double cal_rhs();

    double cal_reduced_cost(Scene_SSL& temp_scene);
    
   // This function is used to find the solution by enumeration.
   // test_scene is the integer vector to show which ssl has been activated like [0,1,3,0,0,1,0,0]
   // unfinished
    double __cal_fx_from_ssl_set(vector<int> test_scene);

    // Give a x, calculate cutval, cutind and rhs for add linear constraint
    void  cal_constraint_by_x(vector<int> weapon_sets);

    double cal_eta_lower_bound();
    double cal_eta_upper_bound();

   // 希望用这个函数里面的东西简单测试下子问题，还没debug, xian paoq
   // int test_subproblem(WTA* temp_WTA, int test_target_num) ;

   // 之后将该返回值改为return status 确保能够得到问题的求解状态
   vector<int> cal_optimal_scene(Scene_SSL& scene_ssl);
   
   // Use this function to varify the correctness of the Subproblem, test by enumeration
    void cal_optimal_scene_by_enum(vector<int>& opt_sols, double& opt_val);

   // A bad name, need change
    bool is_optval_geq_zero();

   // Use this to check weather a solution is frac or integer
    bool print_solution();

   // use this to print current x, cut_ind, cut_val, rhs
    void print_LP_info(); 

    // Use this to print the target index and f(x)
    void print_model();

    // Use this to print the model and temp x
    void print_debug();
   
   /**
    * @brief
    *    This function is used to find a Initial cut in SubProblem
    * @version
    *    Use 1 - p[s][j] * v[j] - u[i] - mu[k]  to find one best ssl for each target
    * @param 
    *    cut_point:  the output, cut_point[i] is the ith's cut with form [0,4,0,0,1,0,0]
    *    cut_num:    Init how many cut
    *    wt_ratio:   In one cut how many nonzero weapon
    * @return 
    *    cut_point:  take down the ssl in cut_point[i]
   */
    void find_init_cut(vector<vector<int>>& cut_point, int cut_num, int wt_ratio);


   void __enumerate_all_scene(vector<vector<int>>& All_num, int col_num, int radar_capacity, int weapon_start, vector<int> weapon_capacity, int& counter_scene, int row_num, int current_radar, double& opt_val, vector<int>& opt_sol);

    void Delete();

   inline int cal_weapon_index(const int ssl_index) const
   {
      return ssl_index % weapon_num_m_;
   } 

   inline int cal_radar_index(const int ssl_index) const
   {
      return ssl_index / weapon_num_m_;
   } 

   inline int cal_ssl_index(const int radar_index, const int weapon_index) const
   {
      return weapon_num_m_ * radar_index + weapon_index;
   }

   // print the lower bound and the upper bound of eta, Used to check cal_lb and cal_ub
   inline void print_eta_bound()
   {
      double temp_lb = cal_eta_lower_bound();
      double temp_ub = cal_eta_upper_bound();
      printf("lb : %.2f\n", temp_lb);
      printf("ub : %.2f\n", temp_ub);
      return;
   }

   

   // Use this function to rounded all the double
};

/* Declaration of the data structure for the function benders_callback */

typedef struct {
   /* Parameter to decide when Benders' cuts are going to be separated:
0: only when an integer solution if found
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_FEAS )
1: even to cut-off fractional solutions, 
at the end of the cplex cut-loop
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_LAST || 
wherefrom == CPX_CALLBACK_MIP_CUT_FEAS ) */
   int separate_fractional_solutions; 
   /* Environment for the worker LP used to generate Benders' cuts */
   CPXENVptr env; 
   /* Worker LP used to generate Benders' cuts */
   CPXLPptr  lp;  
   /* number of nodes in the ATSP instance */
   int num_nodes; 
   /* Number of columns in the master ILP */
   int num_x_cols;
   /* Number of columns in the worker LP */
   int num_v_cols, num_u_cols;
   /* Data structure to 
      -- read the solution from the master ILP 
      -- update the objective function of the worker LP */
   double *x;
   int *indices;
   /* Data structure to read an unbounded direction (ray) from the worker LP */
   double *ray;
   /* Data structure to add a Benders' cut to the master ILP */
   double *cutval;
   int *cutind;


   

} USER_CBHANDLE;


/* Declarations for functions in this program */

static int CPXPUBLIC 
benders_callback  (CPXCENVptr env, void *cbdata, int wherefrom, 
      void *cbhandle, int *useraction_p);

static int
set_benders_callback  (CPXENVptr env, SubProblem* sub_prob),
create_master_ILP     (CPXENVptr env, CPXLPptr lp, SubProblem* sub_prob),
init_user_cbhandle    (USER_CBHANDLE *user_cbhandle, int num_nodes, int separate_fractional_solutions),
free_user_cbhandle    (USER_CBHANDLE *user_cbhandle);


static void
free_and_null  (char **ptr),
usage          (char *progname);

#endif