// LP relaxation of subproblem at each node in the branch-and-bound tree

#ifndef _MASTER_H_
#define _MASTER_H_

// @todo Need import HIGHS instead of cplex
#include "WTA.h"
#include "AlgorithmParameter.h"
#include "Node.h"
#include "Highs.h"

#define WTA_EPS (1e-6)

// LP abstract class, discard
/**
 * @brief This class save the information of the LP
 * @todo It will be repalced by the LP Solver like HIGHS

   class LP_ALL_IN_ONE{
   public:
      int               N_rows;
      int               N_cols;

      vector<double>*   A;
      double*           rhs;
      vector<double>    cost;
      vector<double>    lb;
      vector<double>    ub;

      vector<double>    opt_sol;
      double*           opt_dual_sol; // m + n, two parts
      double            opt_value;

      int               LP_STATE;

      LP_ALL_IN_ONE();
      LP_ALL_IN_ONE(int _N_rows, int _N_cols);

      void INITIALIZE_LP_BY_MASTER(WTA* wta, ScenePool scene_pool);
      void SOLVE_LP();
      // Update LP Info after add a new column
      void ADD_NEW_COLUMN(Scene& temp_scene, int temp_scene_id){printf("scene %d has been added \n", temp_scene_id);}

      void PRINT_LP_INFO();


      void Delete();
      double* GET_DUAL_VALUE();

      ~LP_ALL_IN_ONE();
   };
*/



class Master {
   public:
      Master() = default;
      Master(GRM* _grm, const AlgoParameter &param);
      ~Master();



      // Branch and Bound
      void Set(const Node &node);
      void DelSlack(){}
      void AddSlackToCardinality(){}
      void GetSol(Node &node);
      double GetSlackValue(){return 1;};


      // This block is used to Initilze and solve master with HIGHS
      void Initialize_HIGHS();
      void AddCol(std::vector<Scene_SSL> &scene_ssls);
      bool Solve();

      // auxiliary methods to help MP
      void __add_similar_rows(int& n_rows_, vector<double>& row_lowers_, vector<double>& row_uppers_, int new_rows_nums, double temp_lower, double temp_upper);

      int __find_init_scene(vector<Scene_SSL>& init_scenes, int& num_cols, const int target_num, const int radar_capa, const int weapon_capa);
      int __cal_onessl_matrixAndCost(Scene_SSL temp_SSL, int& target_nnz, vector<int>& target_nz_rows, vector<double>& target_nz_vals, double& col_cost);
      int __add_cols_matrixAndCost(vector<Scene_SSL> all_scenes, vector<int>& add_nnz, vector<int>& add_nz_rows, vector<double>& add_nz_vals, vector<double>& col_costs);
      int __init_col_LbUb(const int init_col_num, vector<double>& col_lbs, vector<double>& col_ubs, double lb, double ub);



      // with Pricing
      void GetDualValues(std::vector<double> &dual);

      //print and write
      void print_current_solution();
      //    Writes the active model to the file specified by filename.
      void writeModel(const std::string &name = "master.lp"){}

      // Use to check if the new scenes is repeated
      // @deprecated
      bool Check_is_scenes_new(std::vector<Scene_SSL> &scenes);

      bool check_is_integer_solution();

       int numCol() const {
         return ssl_pool.size();
       }

       double cal_obj_val();


      // other auxiliary methods
       int convert_sparse_to_compress_int(vector<int> origin_vector, int& nnz, vector<int>& nz_pos, vector<int>& nz_val);

      int convert_sparse_to_compress_double(vector<double> origin_vector, int& nnz, vector<int>& nz_pos, vector<double>& nz_val);

      int __add_conpress_double(int& odd_nz_num, vector<int>& old_nz_indices, vector<double>& old_nz_vals, int add_nz_num, vector<int> add_nz_indices, vector<double> add_nz_vals, int add_base_num);

      int __add_conpress_int(int& odd_nz_num, vector<int>& old_nz_indices, vector<double>& old_nz_vals,int add_nz_num, vector<int> add_nz_indices, vector<int> add_nz_vals, int add_base_num);

      bool is_double_integer(double temp_double){
         int rounded = int(round(temp_double));
         double diff = std::abs(temp_double - static_cast<double>(rounded));
         //printf("val : %.2e, diff : %.2e\n",temp_double, diff);
         if (diff < WTA_EPS)
         {
            return true;
         }
         return false;
      }

   public:
      GRM*                 grm;        // @brief The wta model
      const AlgoParameter  &param;     // @brief the Param of the algorithm
      SSL_Pool             ssl_pool;  // @brief Use this to takedown all the weapon scene
      long                 numLp;      // @brief Use this to calculate the number of LP
      double               time;       // @brief take down the time of the master problem

   // the indices of opt cols, use this to get SSL from SSL_pool
      vector<int>          opt_scene_indices; 

      
   public:
   /**
    * @brief Use this member varialbs to solve LP
    * @todo Replace the following member varible to HIGHS
   */

      // The member variales about HIGHS

      
      HighsModel           model;
      Highs                highs;
      HighsStatus          return_status;
      

  /*
      IloEnv env;
      IloModel model;
      IloCplex cplex;
      IloObjective obj;
      IloRangeArray rng;
      IloRange *ptrNumVehicle;
      IloNumVarArray y;
      IloNumVar slack;
  */
};


#endif //_MASTER_H_
