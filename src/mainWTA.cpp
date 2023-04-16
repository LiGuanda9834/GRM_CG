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

   parameter.objIncludeTime = false;


   
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
   //GRM* grm = new GRM(1);
   grm->print_model();

   TestClass test(grm, parameter);
   

   if(MODE == 3){
      printf("Quick Test \n");
      if(parameter.spOptOnly){
         printf("Only add cut when sp achieve sol\n");
      }
      else{
         printf("Allow sp get a general rc < 0 sol\n");
      }
   }


   if(MODE == BAC){
      BranchAndCut test_BAC(grm, parameter);
      test_BAC.Run();
   }

   // Use this to test Sub-Problem
   if(MODE == SP)   
   {
      printf("------ Now test the subproblem -------\n");
      test.sp = new SubProblem;

      test.test_sp_correctness();
      
      delete test.sp;
      printf("------ subproblem test finished -------\n");
   }

   // Use this block to print the parameters and resultes
   finish_time = clock();
   
   test.print_main_time(start_time, finish_time);
   test.print_main_info(MODE);
   // Use This to clear the problem
   printf("---------- FINISHED --------\n");
   delete grm;
   return 0;
}
