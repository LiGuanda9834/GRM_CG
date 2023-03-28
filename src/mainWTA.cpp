#include "BranchAndCut.h"
#include "WTA.h"
#include "print.h"
#include "Pricing.h"
#include "Scene.h"
#include "SubProblem.h"
#include <string>
#include <ctime>



#define BAC 1  // Branch and Cut
#define SP 2   // Sub-Problem
#define QT 3   // Quick Test
#define BAB 4  // Branch and Bound

/*
enum class problem_type : int {
   BAC = 1,
   SP = 2,
   QT = 3,
   BAB = 4,
};
*/



void temp_print_solution(int val){
      printf("scene %d : ", val);
}

void getCombinations(vector<vector<int>>& All_num, int col_num, int radar_capacity, int weapon_start, vector<int> weapon_capacity, int& counter_scene, int row_num, int current_radar, double& opt_val, vector<int>& opt_sol );
using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
   printf("Start main function\n");

   // This block is used to get 
   string str_test_size = "5";
   int _temp_testsize = 0;
   int arg_1 = 0;
   int arg_2 = 0;
   int arg_3 = 0;
   int arg_4 = 0;
   int arg_5 = 0;


   if(argc > 1){
      arg_1 = atoi(argv[1]);
      str_test_size = argv[1];
   }
   if(argc > 2){
      arg_2 = atoi(argv[2]);
   }
   if(argc > 3){
      arg_3 = atoi(argv[3]);
   }
   if(argc > 4){
      arg_4 = atoi(argv[4]);
   }
   if(argc > 5){
      arg_5 = atoi(argv[5]);
   }

   clock_t start_time, finish_time;

   int MODE = 3;

   AlgoParameter parameter(
      "WTA",              // problem name
      "BranchAndPrice",   // algorithm name
      3600,               // time limit = 3600 (seconds)
      "AA", "",           // teston_prefix, teston_extension
      "./src/mcfdr/data", // path_data
      true,               // rootOnly
      false,              // enableCuts
      false,              // depthFirst if true, otherwise it is primalBoundFirst
      true,               // branchOnSum
      false,              // enableCplexLog
      true                // debug
   );



   
   printf("\n ----- start test WTA by CG ----- \n");

  int grm_target_num = 5;
  int grm_weapon_num = 5;
  int grm_radar_num = 1;
  int temp_seed = 0;


   if(argc > 1){
      MODE = arg_1;
   }
   if(argc > 2){
      grm_target_num = arg_2;
   }
   if(argc > 3){
      grm_weapon_num = arg_3;
   }
   if(argc > 4){
      grm_radar_num = arg_4;
   }
   if(argc > 5){
      temp_seed = arg_5;
   }
   if(argc > 6 || argc < 1)
   {
      printf("Need 1 to 5 arguments. There are %d arguments", argc);
      return 0;
   }

   start_time = clock();
   GRM* grm = new GRM(grm_target_num, grm_weapon_num, grm_radar_num, 1);
   grm->Init_by_random(temp_seed);
   grm->print_model();
   

   if(MODE == 3){
      printf("Quick Test \n");
      // Test Scene
      /*
         vector<int> new_SSL_1 = {1,0,0,1,0,0,1,0};
         vector<int> new_SSL_2 = {0,2,0,1,0,0,0,0};
         vector<int> new_SSL_3 = {0,0,0,0,0,3,0,0};      
         Scene_SSL temp_ssl_1(5, new_SSL_1, 4, 2);
         Scene_SSL temp_ssl_2(3, new_SSL_2, 4, 2);
         Scene_SSL temp_ssl_3(5, new_SSL_3, 4, 2);
         SSL_Pool temp_ssl_pool;
         temp_ssl_pool.add_scene(temp_ssl_1);
         temp_ssl_pool.add_scene(temp_ssl_2);
         temp_ssl_pool.add_scene(temp_ssl_3);
      */
      /*
         temp_ssl_pool.print_all_scene();
         temp_ssl_pool.print_scene_by_target(6);



      */
     /*
      for(int i = 0; i < temp_ssl_pool.size(); i++){
         double temp_qjs = grm->set_ssl_qjs(temp_ssl_pool[i]);
         //printf("Now calculate ssl %d \n", i);
         //temp_ssl_pool[i].PrintScene();

      }
     */
      int temp_col_num = grm->weapon_num_m;
      int temp_row_num = grm->radar_num_k;

      int temp_radar_capa = 8;
      vector<vector<int>> test_enum(temp_row_num, vector<int>(temp_col_num, 0));
      vector<int> w_capacity(temp_col_num, 4);

      int counter = 0;
      double best = 100000000;

      vector<int> opt_sol(temp_col_num * temp_row_num, 0);
      
      
      getCombinations(test_enum, temp_col_num, temp_radar_capa, 0, w_capacity, counter, temp_row_num, 0, best,  opt_sol);

      printf("All : %d\n", counter);
      printf("Best_opt_val : %.2f\n", best);
   }
   // Use This block to Initialize the different classes
   /*
      Master test_master(wta, parameter);
      Pricing test_pricing(wta, parameter);
   */ 

   if(MODE == BAC){
      BranchAndCut test_BAC(grm, parameter);
      test_BAC.Run();

   // Use this to show the Branch and cut search param
      bool PRINT_SEARCH_PARAMS = false;
      if(PRINT_SEARCH_PARAMS){
         int _size1 = test_BAC.globalLb;
         int isPrimal = test_BAC.tree.searchStrategy;
         printf("%s \t: %d\n ", "Is Primal?", isPrimal);
         printf("seed : %d, seed_in_BAC : %d\n", parameter.seed, test_BAC.parameter.seed);
      }
   }


   // Use this to test Sub-Problem
   if(MODE == SP)   
   {
      printf("------ Now test the subproblem -------\n");
      Scene_SSL temp_ssl;
      SubProblem temp_sub;
      int _weapon_num = grm->weapon_num_m;
      int _radar_num = grm->radar_num_k;
      int _target_num = grm->target_num_n;

      double* weapon_dual = new double[_weapon_num];
      double* radar_dual = new double[_radar_num];
      double* time_dual = new double[_target_num];
      double* target_dual = new double[_target_num];

      int target_index = 1;

      int _seed = 0;
      int _is_frac = 1;
      for(int i = 0; i < 5; i++){
         weapon_dual[i] = 0.0;
         time_dual[i] = 0.0;
      }
      radar_dual[0] = -7.6;
      time_dual[1] = -1.0;
      target_dual[0] = 17.99;
      target_dual[1] = 16.21;
      target_dual[2] = 18.04;
      target_dual[3] = 17.24;
      target_dual[4] = 18.23;

      temp_sub.Init(grm, target_index, weapon_dual, radar_dual, time_dual[target_index], target_dual[target_index], _seed, _is_frac, &parameter);

      // This block is used to check the correctness of subproblem
      vector<int> temp_enum_opt_sol(grm->ssl_num, 0);
      double temp_enum_opt_val = 0;

      //temp_sub.print_debug();

      temp_sub.cal_optimal_scene(temp_ssl);
      // This block is used to test find the solution by enumeration

      temp_sub.cal_optimal_scene_by_enum(temp_enum_opt_sol, temp_enum_opt_val);
      temp_sub.Delete();
      
      delete[] weapon_dual;
      delete[] radar_dual;
      delete[] time_dual;
      delete[] target_dual;

   //temp_sub.test_subproblem(wta, 1);
   //temp_sub.print_debug();


   //This block is used to test cal_constraint_by_x(int weapon_set) and find_init_cut() function
   /* 
      Test is finished

   vector<vector<int>> temp_init_cut;
   temp_sub.find_init_cut(temp_init_cut, 1, 1);
   printf("size: %d \n", temp_init_cut[0].size());

   temp_sub.print_LP_info();

   vector<int> temp_weapon_set = {0,3,4};
   printf("weapon set: \n");
   for(int i = 0; i < temp_weapon_set.size(); i++){
      printf("%d ", temp_weapon_set[i]);
   }
   printf("\n");
   
   temp_sub.cal_constraint_by_x(temp_weapon_set);
   temp_sub.print_LP_info();
   */
   printf("------ subproblem test finished -------\n");
   }
   
   // Use this block to print the parameters and resultes
   finish_time = clock();
   double sec = double(finish_time - start_time) / double(CLOCKS_PER_SEC);
   printf("Time \t:");
   std::cout << finish_time - start_time   << "/" << CLOCKS_PER_SEC << " = " << sec << " (s) "<< std::endl;

   printf("Mode \t:");
   switch (MODE)
   {
   case BAC:
      printf("Branch and Cut\n");
      break;
   
   case SP:
      printf("Sub Problem\n");

   case QT:
      printf("Quick Test \n");

   case BAB:
      printf("Branch and Bound\n");

   default:
      break;
   }
   printf("Size\t:target(%d), weapon(%d), radar(%d)\n", grm_target_num, grm_weapon_num, grm_radar_num );
   printf("Seed\t:%d\n", temp_seed);

   // Use This to clear the problem
   printf("---------- FINISHED --------\n");
   delete grm;
   return 0;

   // Use this to test different classes
/*
   std::vector<int> _scene = {1,2,3,4};
   Scene test_scene(4, _scene);
   ScenePool test_scene_pool;

   Node test_node;

   test_pricing.Set(test_node);
   int num_scene = test_pricing.scenes.size();

   LP_ALL_IN_ONE lp(5,5);  
   lp.PRINT_LP_INFO();
*/

}

void getCombinations(vector<vector<int>>& All_num, int col_num, int radar_capacity, int weapon_start, vector<int> weapon_capacity, int& counter_scene, int row_num, int current_radar, double& opt_val,vector<int>& opt_sol) {
    //printf("R(%d), W(%d) \n", current_radar, weapon_start);
    if(current_radar == (row_num-1) && weapon_start == (col_num-1)){
      if(radar_capacity > weapon_capacity[weapon_start]){
         return;
      }
      All_num[current_radar][col_num - 1] = radar_capacity;
      /*
      */
      printf("Scene %d : \n", counter_scene);
      for(int j = 0; j < row_num; j++){
         printf("R%d : ", j);
         for(int i = 0; i < col_num; i++){
            printf("%d ", All_num[j][i]);
         }
         printf("\n");
      }

     /*
     vector<int> temp_ssl(col_num * row_num);
     for(int i = 0; i < row_num; i++){
         for(int j = 0; j < col_num; j++){
            temp_ssl[i * col_num + j] = All_num[i][j];
         }
     }

     print_func(counter_scene);
     for(int i = 0; i < temp_ssl.size(); i++){
      printf("%d ", temp_ssl[i]);
     }
     printf("\n");
     */
     
      counter_scene++;
      return;
    }


    if(weapon_start == (col_num - 1)){
      if(radar_capacity > weapon_capacity[weapon_start]){
         return;
      }
      // This block is used to generate a best solution
      All_num[current_radar][col_num - 1] = radar_capacity;
      vector<int> new_weapon_capacity = weapon_capacity;
      for(int i = 0; i < col_num; i++){
         new_weapon_capacity[i] -= All_num[current_radar][i];
      }
      /*
      printf("capacity : ");
      for(int i = 0; i < col_num; i++){
         printf("%d ", new_weapon_capacity[i]);
      }
      printf("\n");
      */
      getCombinations(All_num, col_num, 8, 0, new_weapon_capacity, counter_scene, row_num,current_radar + 1, opt_val, opt_sol);
      return;
    }

    if(radar_capacity == 0){
      for(int i = weapon_start; i < col_num; i++){
         All_num[current_radar][i] = 0;
         getCombinations(All_num, col_num, 0, col_num -1, weapon_capacity, counter_scene, row_num, current_radar, opt_val, opt_sol);
         return;
      }
    }

    int temp_capacity = (radar_capacity < weapon_capacity[weapon_start]) ? radar_capacity : weapon_capacity[weapon_start];

    //printf("temp_capacity : %d\n", temp_capacity);

    for(int j = 0; j <= temp_capacity; j++){
      All_num[current_radar][weapon_start] = j;
      getCombinations(All_num, col_num, radar_capacity-j, weapon_start + 1, weapon_capacity, counter_scene, row_num, current_radar, opt_val, opt_sol);
    }
}