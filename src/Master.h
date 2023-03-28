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

      void Set(const Node &node);
      bool Solve();
      void GetDualValues(std::vector<double> &dual);
      void AddCol(std::vector<Scene_SSL> &scene_ssls);
      void AddSlackToCardinality(){}
      void GetSol(Node &node);

      // Delete a variable ??   
      void DelSlack(){}

      void Initialize_HIGHS();

      // Get a variable value ??
      double GetSlackValue(){return 1;};
      
      // Use to check if the new scenes is repeated
      // @deprecated
      bool Check_is_scenes_new(std::vector<Scene_SSL> &scenes);

      // Writes the active model to the file specified by filename.
      void writeModel(const std::string &name = "master.lp"){}

       int numCol() const {
        return ssl_pool.size();
       }

       double cal_obj_val();

       void print_current_solution();

       bool check_is_integer_solution();

       int convert_sparse_to_compress_int(vector<int> origin_vector, int& nnz, vector<int>& nz_pos, vector<int>& nz_val);

      int convert_sparse_to_compress_double(vector<double> origin_vector, int& nnz, vector<int>& nz_pos, vector<double>& nz_val);

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
