#include "Pricing.h"
#include "Node.h"
#include "Scene.h"
#include "WTA.h"
#include "SubProblem.h"

Pricing::Pricing(GRM* _grm, AlgoParameter _parameter) 
{
   grm = _grm;
   ptrNode = NULL;
   
   scenes_ssl = {};
   states = {};
   int n = grm->weapon_num_m;
   int m = grm->weapon_num_m;
   int k = grm->radar_num_k;

   int dual_num = 2 * n + m + k;
   dual_value = new double[dual_num];
   parameter = _parameter;
}


/**
 * @brief Use this to get information from the node and Initialize a feasible solution
 * @todo Use a greedy method or more flexible solution
*/

void Pricing::Set(Node &node) {
   ptrNode = &node;
    int weapon_num = grm->weapon_num_m;
    int target_num = grm->target_num_n;
    int radar_num = grm->radar_num_k;
    int radar_cap = grm->radar_capacity;

    int ssl_num = grm->ssl_num;

   bool simple_method = 1;
   if(target_num != weapon_num || radar_num * 5 != weapon_num){
      printf("target : radar : weapon != 5:1:5\n");
      printf("Master Problem Initilized should be rewirte\n");
   }
   // current method, just random assign a feasible solution
   if(simple_method){
      for(int j = 0; j < target_num; j++){
         int radar_index_ = j / 5;
         int weapon_index_ = j;
         int ssl_index = grm->cal_ssl_index(radar_index_, weapon_index_);
         vector<int> temp_scene_ssl(grm->ssl_num, 0);
         temp_scene_ssl[ssl_index] = 1;
         Scene_SSL scene_j(j, temp_scene_ssl, weapon_num, radar_num);
         scenes_ssl.push_back(scene_j);
      }
      for(int j = 0; j < target_num; j++){
         scenes_ssl[j].PrintScene();
      }
   }
    //find one scene for each target by greedy
   else{
   }
}
/**
 * @brief Solve the subproblems of target t
 * @param : dual values, conflict information
 * @return column index 
 */
void Pricing::Solve(vector<double> &dual) 
{
   //printf("\nThe name of the Problem is %s \n", parameter.problem_name);
   scenes_ssl = {};
   vector<Scene_SSL> temp_scenes = {};
   bool all_constraint_satisfied = true;

   int m = grm->weapon_num_m;
   int n = grm->target_num_n;
   int k = grm->radar_num_k;


   bool* each_constraint_satisfied = new bool[n];
   for(int i = 0; i < n; i++){
      each_constraint_satisfied[i] = true;
   }
   
   // get dual from master problem
   for(int i = 0; i < dual.size(); i++){
      dual_value[i] = dual[i];
   }

   double* weapon_dual = new double[m];
   double* radar_dual = new double[k];
   double* time_dual = new double[n];
   double* target_dual = new double[n];

   printf("weapon dual : ");
   for(int i = 0; i < m; i++){
      weapon_dual[i] = dual_value[i];
      printf("%.2f ", weapon_dual[i]);
   }
   printf("\nradar dual : ");
   for(int i = 0; i < k; i++){
      radar_dual[i] = dual_value[i + m];
      printf("%.2f ", radar_dual[i]);
   }
   printf("\ntime dual : ");
   for(int i = 0; i < n; i++){
      time_dual[i] = dual_value[i + m + k];
      printf("%.2f ", time_dual[i]);
   }
   printf("\ntarget dual : ");
   for(int i = 0; i < n; i++){
      target_dual[i] = dual_value[i + m + k + n];
      printf("%.2f ", target_dual[i]);
   }

   // Use this to test subproblem by enumerate
   vector<int> Is_correct(n, 1);
   // if This Flag == 1, test correct by enumeration
   int DEBUG_CORRECT = 0;


   int _seed = 0; 
   int _is_frac = 1;
   AlgoParameter temp_parameter = parameter;

   // generate a scene for each target from subproblem
   


   for(int t = 0; t < n ; t++)
   {  
      printf("\nNow calculate the scene %d\n", t);
      SubProblem sub_prob;
      sub_prob.Init(grm, t, weapon_dual, radar_dual, time_dual[t], target_dual[t], _seed, _is_frac, &parameter);
      Scene_SSL temp_scene;
      vector<int> solve_by_OA = sub_prob.cal_optimal_scene(temp_scene);
      if(DEBUG_CORRECT){
         //vector<int> solve_by_Enum = sub_prob.cal_optimal_scene_by_enum();
         printf("ssl_num : %d\n", sub_prob.ssl_num); 
         for(int i = 0; i < sub_prob.ssl_num; i++){
            //if(solve_by_OA[i] != solve_by_Enum[i]){
            //   Is_correct[i] = 0;
            //}
         }
      }
      temp_scenes.push_back(temp_scene);
      // @todo decide weather do not push scene when one target optval > 0 or all  
      // 子问题最优解大于0 说明此时对偶约束已经满足 即不需要添加该场景
      
      //bool target_j_satisfied = sub_prob.is_optval_geq_zero();

      double real_redcost = sub_prob.cal_reduced_cost(temp_scene);
      //temp_scene.PrintScene();
      printf("opt_val : %e, real_redcost : %e\n", sub_prob.sub_optval, real_redcost);
      if(!(real_redcost > -1e-8)){
         printf("---- This target doesn't satisfied -----\n");
         all_constraint_satisfied = false;
         each_constraint_satisfied[t] = false; 
      }
      sub_prob.Delete();
   }


   if(DEBUG_CORRECT){
      for(int i = 0; i < n; i++){
         if(Is_correct[i]){
            printf("target %d is Correct!\n", i);
         }
      }
   }

   // 0 for add all if all unfinished
   // 1 for add one if one unfinished
   int add_mode = 1;
   if(add_mode == 0)
   {
      if(!all_constraint_satisfied){
         scenes_ssl = temp_scenes;
      }
   }
   if(add_mode == 1)
   {
      for(int t = 0; t < n; t++){
         if(!each_constraint_satisfied[t]){
            scenes_ssl.push_back(temp_scenes[t]);
         }
      }
   }
   
   printf("\nIn this pricing step we gain %d scenes\n", scenes_ssl.size());
   delete[] weapon_dual;
   delete[] target_dual;
   delete[] time_dual;
   delete[] radar_dual;
   delete[] each_constraint_satisfied;
}

Pricing::~Pricing(){
   if(dual_value){
      delete[] dual_value;
      dual_value = NULL;
   }
}