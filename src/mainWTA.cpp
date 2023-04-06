#include "BranchAndCut.h"
#include "WTA.h"
#include "print.h"
#include "Pricing.h"
#include "Scene.h"
#include "SubProblem.h"
#include "TestClass.h"
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

   TestClass test(grm, parameter);
   

   if(MODE == 3){
      printf("Quick Test \n");
      string Path = "/share/home/liguanda/WTA-Problem/Code/GRM_CG/data/CSV/";
      string Name = "instance" + std::to_string(1) + ".csv";
      string file_name_str = Path + Name;
      char* test_file_char = (char*)file_name_str.c_str();
      std::cout << test_file_char << endl;
      FILE* fin = NULL;
      fin = fopen(test_file_char, "r");
      if(fin == NULL){
         printf("Open file Failed! \n");
      }
      else{
         int T_num = 0, W_num = 0, R_num = 0;
         int W_capacity, R_capacity = 0;
         fscanf (fin, "%d,%d,%d,", &T_num, &W_num, &R_num);
         fscanf (fin, "%d, %d,", &W_capacity, &R_capacity);

         printf("T = %d, W = %d, R = %d\n", T_num, W_num, R_num);
         printf("Weapon capacity : %d, Radar capacity : %d\n", W_capacity, R_capacity);
         for(int i = 0; i < T_num; i++){
            double temp_value;
            fscanf (fin, "%lf,", &temp_value);
            printf("%d value : %lf\n",i ,temp_value);
         }
         int ssl_num = W_num * R_num;
         
         for(int i = 0; i < T_num; i++){
            for(int j = 0; j < ssl_num; j++){
               double temp_value;
               fscanf (fin, "%lf,", &temp_value);
               printf("%lf ",temp_value);
            }
            printf("\n");
         }

      }
   }


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

      int test_size = 1;
      vector<bool> is_correct(test_size, true);
      for(int i = 0; i < test_size; i++){
         temp_sub.Init_test(grm, 0, &parameter, i);
         temp_sub.print_model();
         is_correct[i] = temp_sub.check_correctness_by_enum();
      }
      for(int i = 0; i < test_size; i++){
         if(is_correct[i] = true){
            printf("instance %d is correct\n", i);
         }
      }
      temp_sub.Delete();
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