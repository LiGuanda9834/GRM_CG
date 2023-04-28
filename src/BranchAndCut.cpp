/**
 * @file BranchAndCut.cpp
 * @author Zhou Xiao 
 * @brief 
 * @date 2022-11-07
 * @copyright Copyright (c) 2022
 */

#include <cassert>
#include <iostream>
#include "BranchAndCut.h"
#include "print.h"


BranchAndCut::BranchAndCut(
   GRM* _grm,
   const AlgoParameter &_parameter
) : grm(_grm), parameter(_parameter), master(grm, parameter), pricing(grm, parameter), tree(parameter.depthFirst)
{
   globalLb = -parameter.maxValue;
   globalUb = parameter.maxValue;
   nodeCnt = 0;
   cutsCnt = 0;
   timeOnHeur = 0.0;
   timeOnAddCol = 0.0;
   timeOnMaster = 0.0;
   timeOnPricing = 0.0;
   timeOnCG = 0.0;

   int weapon_m = grm->weapon_num_m;
   int target_n = grm->target_num_n;
   int radar_k = grm->radar_num_k;

   if(parameter.objIncludeTime)
   {
      dual = vector<double>(weapon_m + target_n * 2 + radar_k, 0);
   }
   else
   {
      dual = vector<double>(weapon_m + target_n + radar_k, 0);
   }
   
   col = NULL;
   row = NULL;
   incumbent = NULL;

   timer = compute::Timer();
   rootLb = -parameter.maxValue;
   rootUb = parameter.maxValue;
}

// 执行函数
void BranchAndCut::Run() {
   printf(" --- Start Initialize the Root Note ---\n\n");
   /* create and Initialize the root node*/
   Node* root;
   /* the initial status of root is unsolved */
   
   root->Init();

   printf(" --- Start solve the Root Note ---\n");
   // Solve the LP of the root Node
   SolveRootNode(*root);

   tree->leaves->Append1(root);
   return;
   /* solve the leaves of branch and bound tree */
   if( !parameter.rootOnly )
   {
      while( tree->leaves->GetLeafNum() != 0 && !TimeLimit() && !Optimal() )
      {
         Node* temp_node = tree->leaves->GetBest();
         //tree.pop();
         Branch(*temp_node);
         nodeSelect();
         updateTreePath();
         Solve(*temp_node);
         checkLPSol();
      }
      if( tree->leaves->GetLeafNum() == 0 )
      {
         globalLb = globalUb;
      }
   }
   // 输出信息函数
   OutputPerformanceMetrics();
}

/**
 * @brief Solve root node 
 * @param node
 * @return node status
 */
void BranchAndCut::SolveRootNode(Node &node)
{
   info::print(debuginfo, "enter SolveRootNode");
   master.Set(node);
   pricing.Set(node);
   bool feasible = InitialColumns(node); 
   master.Initialize_HIGHS();
   printf("Still right!\n");

   //---master.Solve();
   //infeasible: 
   //---master.scenepool.print_all_scene();

   if( feasible )
   {
      /* if root is feasible, then get the optimal solution of LP relaxtion of root node */
      ColumnGeneration(node);
      return;
      master.GetSol(node);
      //---node.lb = node.lpsol->obj;
   }

   else //infeasible
   {
      /* otherwise, if the root node is infeasible, then ... */
   }
   return;

}

void BranchAndCut::Solve(Node &node)
{
   
}

/**
 * @note Designed for mcfdr. Don't think about how to use it
 * @brief master lp initial columns
 * @param : root node 
 * @return index of columns
 */
bool BranchAndCut::InitialColumns(
   Node          &node
)
{  
   // pricing.Solve(/*no dual variables*/);
   master.AddCol(pricing.valid_scenes_ssl);
   return true;
}


bool BranchAndCut::ColumnGeneration(
   Node          &node
) 
{
   int master_running_counter = 0;

   printf("--- Now start Column Generation --- \n");
   if( !master.Solve() )
   {  
      printf("Fail to solve the master problem\n");
      return false;
   }
   master.ssl_pool.print_all_scene();
   master.GetDualValues(dual);
   print_current_dual();
   pricing.Solve(dual);
   /* add column(s) gradually to master problem,*/ 
   while (pricing.valid_scenes_ssl.size() != 0) /*while scenes generated from sp is empty, stop */
   {
      clock_t start_one_master, after_add_before_master, after_master_before_pricing, end_one_master;
      start_one_master = clock();
   
      printf("\n\n--------------Now Solve a new Master Problem-------------\n");
      /* Add path(s) from the candidate pool */
      //master.AddCol(pricing.valid_scenes_ssl);
      master.add_new_cols(pricing.valid_scenes_ssl);
      master.ssl_pool.print_scene_by_target(grm->target_num_n);
      //return true;
      after_add_before_master = clock();

      master.Solve(); 
      
      after_master_before_pricing = clock();

      printf("\n\n--------------new Master Problem has been solved-------------\n");

      /* Get the value of dual variables of master problem */
      master.GetDualValues(dual);
      pricing.Solve(dual); 
      end_one_master = clock();

      master_running_counter++;

      record_time(start_one_master, after_add_before_master, after_master_before_pricing, end_one_master, 0);
      // if(master_running_counter > 3){
      //    return true;
      // }
   }
   printf("All the Scenes generated from Subproblems are : \n");
   int target_num = grm->target_num_n;
   //master.scenepool.print_all_scene();
   master.ssl_pool.print_scene_by_target(target_num);
   master.cal_obj_val();
   bool is_integer_soluton = master.check_is_integer_solution();

   int final_scenepool_size = master.ssl_pool.size();

   double avg_sub_solve_time = timeOnPricing / double(final_scenepool_size);

   printf("\n--- Show Time and Cols record ---\n");
   printf("All time             : %.3f\n", timeOnCG);
   printf("AddCol  running time : %.2f\n", timeOnAddCol);
   printf("Master  running time : %.2f\n", timeOnMaster);
   printf("Pricing running time : %.2f\n", timeOnPricing);
   printf("pricing running Num  : %d\n", final_scenepool_size);
   printf("avg sub running time : %e\n", avg_sub_solve_time);
   
   printf("======= Column Generation finished ========= \n");
   return true;
}

/**
 * @brief   Big M
 * @note    still WTA version, need modified

bool BranchAndCut::BigM(Node &node)
{
   double pricing_solve_time = 0;
   double master_solve_time = 0;
   master.AddSlackToCardinality();
   bool feasible = master.Solve(master_solve_time);
   if( !feasible )
   {}
   else
   {
      double slackValue = master.GetSlackValue();

      master.GetDualValues(dual);
      pricing.Solve(dual,pricing_solve_time);
      while( !pricing.scenes.empty() && slackValue > parameter.eps )
      {
         master.AddCol(pricing.scenes);
         master.Solve(master_solve_time);
         slackValue = master.GetSlackValue();
         master.GetDualValues(dual);
         pricing.Solve(dual, pricing_solve_time);
      }
   }
   master.DelSlack();
   return feasible;
}
*/

void BranchAndCut::Branch(Node &parent)
{
   if( parameter.enableBranchOnSum )
   {
      double total = parent.lpsol->accumulate();
      if( !compute::integral(total, parameter.eps) )
      {
         int floor = int(std::floor(total));
         // tree.emplace(NULL, ++nodeCnt, true, )
      }
      
   }
}

void BranchAndCut::Print(Node &node, bool diving) const 
{
   string tag = diving ? "dive" : "Node";
   info::print_tab(tag, tree.size(), nodeCnt, node.statusStr(), node.lb, globalLb, globalUb, Gap(),
                     timer.elapsed(), master.time, master.numLp, master.numCol());
}

void BranchAndCut::OutputPerformanceMetrics() {
   // cout << "Porblem Name         : " << mcfdr->probname << endl;
   std::cout << "Parameter Infomation : " << std::endl;
   std::cout << "   Algorithm Name    : " << parameter.algo_name << std::endl;
   std::cout << "   Time Limit        : " << parameter.time_limit << std::endl;
   std::cout << "   Is Root Only      : " << parameter.rootOnly << std::endl;
   std::cout << "   Is Enable Cuts    : " << parameter.enableCuts << std::endl;
   std::cout << "Root Gap(%)          : " << 100 * (rootUb - rootLb) / rootLb << std::endl;
   // cout << "Gap Closed(%)        : " << 100 * (rootLb - FirstLb) / (objValue - FirstLb) << std::endl;
   std::cout << "Global Gap(%)        : " << Gap() << std::endl;
   std::cout << "Time                 : " << timer.elapsed() << std::endl;
   std::cout << "Time On Master       : " << master.time << std::endl;
   std::cout << "Node Count           : " << nodeCnt - tree.size() << std::endl;
   std::cout << "Number of Columns In Master : " << master.numCol() << std::endl;
   std::cout << "Current Date Time    : " << timer.getCurrentDateTime() << std::endl;
}
void BranchAndCut::record_time(clock_t start_time, clock_t addCol_finished_time, clock_t master_finished_time, clock_t pricing_finished_time, int DEBUG_LEVEL){
   double temp_AddCol_sec = double(addCol_finished_time - start_time) / double(CLOCKS_PER_SEC);
   double temp_MasterSolve_sec = double(master_finished_time - addCol_finished_time) / double(CLOCKS_PER_SEC);
   double temp_PricingSolve_sec = double(pricing_finished_time - master_finished_time) / double(CLOCKS_PER_SEC);
   double temp_all_sec = double(pricing_finished_time - start_time) / double(CLOCKS_PER_SEC);
   timeOnCG += temp_all_sec;
   timeOnAddCol += temp_AddCol_sec;
   timeOnMaster += temp_MasterSolve_sec;
   timeOnPricing += temp_PricingSolve_sec;
   if(DEBUG_LEVEL == 1);
   {
      printf("------------- Print BAC Time on master %d----------------\n", master.numLp);
      printf("CG :\t%.2e\n", temp_all_sec);
      printf("AddCol :\t%.2e\n", temp_AddCol_sec);
      printf("Master :\t%.2e\n", temp_MasterSolve_sec);
      printf("Pricing:\t%.2e\n", temp_PricingSolve_sec);

   }
}

void BranchAndCut::print_current_dual(){
   int dual_num = dual.size();
   printf("print dual stored in column generation\n");
   for(int i = 0; i < dual_num; i++){
      printf("%.2f ", dual[i]);
   }
   printf("\n");
}