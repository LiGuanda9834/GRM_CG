#include "Pricing.h"
#include "Node.h"
#include "Scene.h"
#include "WTA.h"
#include "SubProblem.h"

Pricing::Pricing(GRM* _grm, AlgoParameter _parameter) 
{
   grm = _grm;
   parameter = _parameter;
   sp_num = _grm->target_num_n;
   is_rcLeqZero = new bool[sp_num];
   is_spCorrect = new bool[sp_num];
   for(int i = 0; i < sp_num; i++){
      is_rcLeqZero[i] = false;
      is_spCorrect[i] = true;
   }

   dual_value_num = 0;
   dual_vals = NULL;
   
   dual_block_num = 0;
   splited_dual_block_size = {};
   splited_dual_vals = {};
   all_sp_ssl = {};
   valid_scenes_ssl = {};

   ptrNode = NULL;
   states = {};
   is_consider_time_dual = parameter.objIncludeTime;
   is_check_correctness = parameter.pricingCheckCorrectness;
}


/**
 * @brief Use this to get information from the node and Initialize a feasible solution
 * @todo Use a greedy method or more flexible solution
*/


/**
 * @brief Solve the subproblems of target t
 * @param : dual values, conflict information
 * @return column index 
 */





void Pricing::Solve(vector<double> &dual) 
{
   vector<Scene_SSL> temp_scenes;
   // Set constant and argument
   int _seed = parameter.seed; 
   int _is_frac = parameter.spCutAllowFrac;

   int DEBUG_CORRECT = 0;

   // get dual from mp, divide dual to weapon, radar, target, time(if neccessary)
   get_and_split_dual(dual);
   // generate a scene for each target from subproblem, (May check the correctness of the sp sol)
   find_all_sp_sol(_seed, _is_frac);
   // check all scenes generated from sp is valid
   pick_validate_scenes();
   printf("\nIn this pricing step we gain %d scenes\n", valid_scenes_ssl.size());
}


void Pricing::get_and_split_dual(vector<double>& dual){
   dual_value_num = dual.size();
   dual_vals = new double[dual_value_num];
   for(int i = 0; i < dual_value_num; i++){
      dual_vals[i] = dual[i];
   }

   int m = grm->weapon_num_m;
   int n = grm->target_num_n;
   int k = grm->radar_num_k;
   
   int current_pos = 0;
   double* weapon_dual = new double[m];
   double* radar_dual = new double[k];
   double* target_dual = new double[n];

   printf("weapon dual : ");
   __split_dual(m, weapon_dual, current_pos);

   printf("\nradar dual : ");
   __split_dual(k, radar_dual, current_pos);

   printf("\ntarget dual : ");
   __split_dual(n, target_dual, current_pos);

   if(parameter.objIncludeTime){
      double* time_dual = new double[n];
      printf("\ntime dual : ");
      __split_dual(n, time_dual, current_pos);
   }
}

void Pricing::__split_dual(int current_block_size, double* current_dual_block, int& dual_check_pos){
   for(int i = 0; i < current_block_size; i++){
      current_dual_block[i] = dual_vals[dual_check_pos];
      dual_check_pos++;
   }
   dual_block_num++;
   splited_dual_block_size.push_back(current_block_size);
   splited_dual_vals.push_back(current_dual_block);

   bool print_vals = true;
   if(print_vals){
      for(int i = 0; i < current_block_size; i++){
         printf("%.2f ", current_dual_block[i]);
      }
   }
}

void Pricing::find_all_sp_sol(int _seed, int _is_frac){
   for(int t = 0; t < sp_num ; t++)
   {  
      printf("\nNow calculate the scene %d\n", t);
      SubProblem sub_prob;
      double* weapon_dual_ = splited_dual_vals[0];
      double* radar_dual_ = splited_dual_vals[1];
      double target_dual_ = splited_dual_vals[2][t];
      double time_dual_ = 0;

      if(is_consider_time_dual){
         double time_dual_ = splited_dual_vals[3][t];
      }
      
      sub_prob.Init(grm, t, weapon_dual_, radar_dual_, time_dual_, target_dual_, _seed, _is_frac, &parameter);


      Scene_SSL temp_scene;
      vector<int> solve_by_OA = sub_prob.cal_optimal_scene(temp_scene);
      all_sp_ssl.push_back(temp_scene);

      double real_redcost = sub_prob.cal_reduced_cost(temp_scene);
      printf("opt_val : %e, real_redcost : %e\n", sub_prob.sub_optval, real_redcost);
      if(real_redcost < -1e-8){
         printf("---- This target doesn't satisfied -----\n");
         is_rcLeqZero[t] = true; 
      }

      if(is_check_correctness){
         is_spCorrect[t] = sub_prob.check_correctness_by_enum();
      }
      sub_prob.Delete();
   }
}


void Pricing::pick_validate_scenes(){
   
   for(int i = 0; i < sp_num; i++){
      if(is_rcLeqZero[i]){
         valid_scenes_ssl.push_back(all_sp_ssl[i]);
      }
   }
}


void Pricing::Set(Node &node) {
   ptrNode = &node;
    int weapon_num = grm->weapon_num_m;
    int target_num = grm->target_num_n;
    int radar_num = grm->radar_num_k;
    int radar_cap = grm->radar_capacity;
    int ssl_num = grm->ssl_num;
   // @deprecated
   // method, just random assign a feasible solution
   /*
   bool simple_method = 1;
   if(target_num != weapon_num || radar_num * 5 != weapon_num){
      printf("target : radar : weapon != 5:1:5\n");
      printf("Master Problem Initilized should be rewirte\n");
   }
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
   */
}

Pricing::~Pricing(){
   if(is_rcLeqZero){
      delete[] is_rcLeqZero;
      is_rcLeqZero = NULL;
   }
   if(is_spCorrect){
      delete[] is_spCorrect;
      is_spCorrect = NULL;
   }
   if(dual_vals){
      delete[] dual_vals;
      dual_vals = NULL;
   }
   for(int i = 0; i < splited_dual_vals.size(); i++){
      if(splited_dual_vals[i]){
         delete[] splited_dual_vals[i];
         splited_dual_vals[i] = NULL;
      }
   }
}