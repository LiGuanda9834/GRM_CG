#include "Master.h"

#include "Highs.h"
#include <cassert>
#include <ctime>

using std::cout;
using std::endl;
using std::vector;


Master::Master(
   GRM* _grm, 
   const AlgoParameter &_param
): grm(_grm), param(_param)
{
   ssl_pool = SSL_Pool();
   numLp = 0;
   time = 0;
}

// Use this to get information from the node
void Master::Set(const Node &node) 
{
   /**
    * @brief Set the upper bound and the lower bound of all the scenes
    * @todo Is this step neccessary
   */
}

void Master::Initialize_HIGHS(){
  printf("\n----- Now Initialize master's HIGHS --- \n");


  int DEBUG = 0;

  // Although the first constraint could be expressed as an upper
  // bound on x_1, it serves to illustrate a non-trivial packed
  // column-wise matrix.
  //

   int m = grm->weapon_num_m;
   int n = grm->target_num_n;
   int k = grm->radar_num_k;
   int N_scenes = ssl_pool.size();

   int r_capacity = grm->radar_capacity;
   int w_capacity = grm->weapon_capacity;

   printf("Now the Scene Pool has : %d\n", N_scenes);
   // time eta
   int N_cols = N_scenes + 1;
   int N_rows = m + 2*n + k;

   int base_weapon_cq = 0;
   int base_radar_cq = m;
   int base_time_cq = m + k;
   int base_target_cq = m + k + n;

  model.lp_.num_col_ = N_cols;
  model.lp_.num_row_ = N_rows;
  model.lp_.sense_ = ObjSense::kMinimize;
  model.lp_.offset_ = 0;
  
   vector<double> cost(N_cols, 0);

   // Init cost, include scenes and time
   // [scene cost]
   for(int i = 0; i < N_scenes; i++){
      Scene_SSL temp_ssl = ssl_pool[i];
      cost[i] = grm->set_ssl_qjs(temp_ssl);
   }
   cost[N_scenes] = 1;
   // [time cost]

model.lp_.col_cost_ = cost;


// Init lb and ub of LP
  model.lp_.col_lower_ = vector<double>(N_cols, 0.0);
  model.lp_.col_upper_ = vector<double>(N_cols, 1.0);
  model.lp_.row_lower_ = vector<double>(N_rows, 0.0);
  model.lp_.row_upper_ = vector<double>(N_rows, 1.0);

for(int i = 0; i < m; i++){
   model.lp_.row_upper_[i] = static_cast<double>(w_capacity);
}

for(int i = base_radar_cq; i < base_radar_cq + k; i++){
   model.lp_.row_upper_[i] = static_cast<double>(r_capacity);
}

for(int i = base_time_cq; i < base_time_cq + n; i++){
      model.lp_.row_upper_[i] = 0;
      model.lp_.row_lower_[i] = -1e+10;
}

for(int i = base_target_cq; i < base_target_cq + n; i++){
   model.lp_.row_lower_[i] = 1.0;
}

model.lp_.col_upper_[N_cols - 1] = 1e+10;



// Init A, pass each cols to column_compress data structure
// Include scenes and eta
vector<int>       start_index;
vector<int>       nz_rows;
vector<double>    nz_vals;

int               nz_counter = 0;

// Init All the scenes
for(int i = 0; i < N_scenes; i++){
   start_index.push_back(nz_counter);
   Scene_SSL temp_ssl_scene = ssl_pool[i];
   // Init A, weapon_row coef
   int weapon_nnz = 0;
   vector<int> weapon_nz_indices;
   vector<int> weapon_nz_values;

   
   convert_sparse_to_compress_int(temp_ssl_scene.activated_weapons_num, weapon_nnz, weapon_nz_indices, weapon_nz_values);

   for(int j = 0; j < weapon_nnz; j++){
      nz_rows.push_back(weapon_nz_indices[j]);
      nz_vals.push_back(static_cast<double>(weapon_nz_values[j]));
   }
   nz_counter += weapon_nnz;

   // Init A, radar row coef
   int radar_nnz = 0;
   vector<int> radar_nz_indices;
   vector<int> radar_nz_values;

   convert_sparse_to_compress_int(temp_ssl_scene.activated_radars_num, radar_nnz, radar_nz_indices, radar_nz_values);

   for(int j = 0; j < radar_nnz; j++){
      nz_rows.push_back(radar_nz_indices[j] + base_radar_cq);
      nz_vals.push_back(static_cast<double>(radar_nz_values[j]));
   }
   nz_counter += radar_nnz;

   // Init A, time row coef
   for(int j = 0; j < n; j++){
      int temp_scene_target_j = temp_ssl_scene.target_index;
      if(j == temp_scene_target_j)
      {
         nz_rows.push_back(j + base_time_cq);
         nz_vals.push_back(grm->set_ssl_tjs(temp_ssl_scene));
         nz_counter++;
      }
   }

   // Init A, target row coef
   int target_index_ = temp_ssl_scene.target_index;
   int target_row_index = target_index_ + base_target_cq;
   nz_rows.push_back(target_row_index);
   nz_vals.push_back(1.0);
   nz_counter++;
}
start_index.push_back(nz_counter);
//Init eta
for(int i = 0; i < n; i++){
   nz_rows.push_back(base_time_cq + i);
   nz_vals.push_back(-1);
}
nz_counter += n;
start_index.push_back(nz_counter);

  // Here the orientation of the matrix is column-wise
  model.lp_.a_matrix_.format_ = MatrixFormat::kColwise;
  // a_start_ has num_col_1 entries, and the last entry is the number
  // of nonzeros in A, allowing the number of nonzeros in the last
  // column to be defined
  model.lp_.a_matrix_.start_ = start_index;
  model.lp_.a_matrix_.index_ = nz_rows;
  model.lp_.a_matrix_.value_ = nz_vals;

   printf("N_rows : %d", N_rows);
   printf("N_cols : %d",  N_cols);
   int real_nz_num = nz_vals.size();
   if(real_nz_num == nz_counter){
      printf("column index : ");
      for(int i = 0; i < start_index.size(); i++){
         printf(" %d ", start_index[i]);
      }
      printf("\nnz index : ");
      for(int i = 0; i < nz_rows.size(); i++){
         printf("\t%d", nz_rows[i]);
      }
      printf("\nnz val : ");
      for(int i = 0; i < nz_vals.size(); i++){
         printf("\t%.2f", nz_vals[i]);
      }
      printf("\n");
   }
   else{
      printf("!!!!! counter has problem\n");
   }
   return_status = highs.passModel(model);
   assert(return_status==HighsStatus::kOk);


// Write Model to Init_LP.lp

   printf("\n---- Highs of Master Problem has been Initialized ---- \n");
}



Master::~Master(){}

/**
 * @brief Use this to solve the LP by solver like HIGHS
 * @todo need to update the result
*/
bool Master::Solve() 
{

   opt_scene_indices = {};
   numLp++;
   printf("Now solve Mater Problem\n");
   printf("scene pool size : %d\n", ssl_pool.size());
   //lp->INITIALIZE_LP_BY_MASTER(this->wta, this->scenepool);
   //lp->SOLVE_LP();
   //lp->PRINT_LP_INFO();
   bool result = true;
   printf("\n\n------------- Start Highs ---------------------\n");
   /*
  HighsModel model;
  model.lp_.num_col_ = 2;
  model.lp_.num_row_ = 3;
  model.lp_.sense_ = ObjSense::kMinimize;
  model.lp_.offset_ = 3;
  model.lp_.col_cost_ = {1.0, 1.0};
  model.lp_.col_lower_ = {0.0, 1.0};
  model.lp_.col_upper_ = {4.0, 1.0e30};
  model.lp_.row_lower_ = {-1.0e30, 5.0, 6.0};
  model.lp_.row_upper_ = {7.0, 15.0, 1.0e30};
  //
  // Here the orientation of the matrix is column-wise
  model.lp_.a_matrix_.format_ = MatrixFormat::kColwise;
  // a_start_ has num_col_1 entries, and the last entry is the number
  // of nonzeros in A, allowing the number of nonzeros in the last
  // column to be defined
  model.lp_.a_matrix_.start_ = {0, 2, 5};
  model.lp_.a_matrix_.index_ = {1, 2, 0, 1, 2};
  model.lp_.a_matrix_.value_ = {1.0, 3.0, 1.0, 2.0, 2.0};
  //

  // Create a Highs instance
  Highs highs;
  HighsStatus return_status;
  //
  // Pass the model to HiGHS
  */

   // Use this to take down how much time HIGHS takes


   //int timer_id = 97;
   //HighsTimer my_timer;
   //my_timer.start(97);

   // This flag is used to check weather the LP Solution is integer, if not, throw an alert 
   bool is_integer = 1;

   int DEBUG_WRITE_LP = 0;
   if(DEBUG_WRITE_LP) {
      printf("\n --- now write the incumbent LP  --- \n");
      return_status = highs.writeModel("wta.lp");
   }

  //
  // Get a const reference to the LP data in HiGHS
  const HighsLp& lp = highs.getLp();
  //
  // Solve the model
  return_status = highs.run();
  assert(return_status==HighsStatus::kOk);

  //
  // Get the model status
  const HighsModelStatus& model_status = highs.getModelStatus();
  assert(model_status==HighsModelStatus::kOptimal);
  cout << "Model status: " << highs.modelStatusToString(model_status) << endl;
  //
  // Get the solution information
  const HighsInfo& info = highs.getInfo();
  cout << "Simplex iteration count: " << info.simplex_iteration_count << endl;
  cout << "Objective function value: " << info.objective_function_value << endl;
  cout << "Primal  solution status: " << highs.solutionStatusToString(info.primal_solution_status) << endl;
  cout << "Dual    solution status: " << highs.solutionStatusToString(info.dual_solution_status) << endl;
  cout << "Basis: " << highs.basisValidityToString(info.basis_validity) << endl;
  const bool has_values = info.primal_solution_status;
  const bool has_duals = info.dual_solution_status;
  const bool has_basis = info.basis_validity;
  //
  // Get the solution values and basis
  const HighsSolution& solution = highs.getSolution();
  const HighsBasis& basis = highs.getBasis();
  //
  // Report the primal and solution values and basis
  int DEBUG_PRINT_ALL_COLUMN = 0;


   if(!DEBUG_PRINT_ALL_COLUMN){
      printf("Only Print nonzero solution!\n");
   }
  for (int col=0; col < lp.num_col_; col++) {
      if(!DEBUG_PRINT_ALL_COLUMN){
         if(solution.col_value[col] == 0){
            continue;
         }
      }
      cout << "Column " << col;
      if (has_values) cout << "; value = " << solution.col_value[col];
      if((col != (grm->weapon_num_m)) && !is_double_integer(solution.col_value[col]))
      {
         printf("Alert!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! We find a non-integer LP solution in Master Problem !!!!!!!\n");
      }
      if( solution.col_value[col] > 1 - 1e-6){
         if(col < grm->target_num_n + 1){
            opt_scene_indices.push_back(col);
         }
         else{
            opt_scene_indices.push_back(col - 1); 
         }

      }
      if (has_duals) cout << "; dual = " << solution.col_dual[col];
      if (has_basis) cout << "; status: " << highs.basisStatusToString(basis.col_status[col]);
      cout << endl;
  }
   int DEBUG_PRINT_ROW = 1;
   if(DEBUG_PRINT_ROW){
      for (int row=0; row < lp.num_row_; row++) {
         cout << "Row    " << row;
         if (has_values) cout << "; value = " << solution.row_value[row];
         if (has_duals) cout << "; dual = " << solution.row_dual[row];
         if (has_basis) cout << "; status: " << highs.basisStatusToString(basis.row_status[row]);
         cout << endl;
      }
   }

   printf("Pool Size : %d, col Num : %d\n", ssl_pool.size(), lp.num_col_);



   printf("------------- End Highs ---------------------\n");
   
   return result;

// 
/**
 * @brief calculate how much time HIGHS takes
 * @deprecated
 * 
      highs_running_time = highs.getRunTime();
      printf(" HIGHS use %e seconds -------------------------------------------------------------- \n ", highs_running_time);
*/



   /*
---------- This Blocl change the LP to MIP, but I think this is not Neccessary and make dual value to 0,
           So I Note this block 

  // Now indicate that all the variables must take integer values
  model.lp_.integrality_.resize(lp.num_col_);
  for (int col=0; col < lp.num_col_; col++)
    model.lp_.integrality_[col] = HighsVarType::kInteger;

  highs.passModel(model);
  // Solve the model
  return_status = highs.run();
  assert(return_status==HighsStatus::kOk);
  // Report the primal solution values
  for (int col=0; col < lp.num_col_; col++) {
    cout << "Column " << col;
    if (info.primal_solution_status) cout << "; value = " << solution.col_value[col];
    cout << endl;
  }
  for (int row=0; row < lp.num_row_; row++) {
    cout << "Row    " << row;
    if (info.primal_solution_status) cout << "; value = " << solution.row_value[row];
    cout << endl;
  }
   const HighsLp& lp_ = highs.getLp();
   const HighsSolution& solution_ = highs.getSolution();
   int N_Dual_sol = lp_.num_row_;
   printf("Print Dual after MIP\n");
   for(int i = 0; i < N_Dual_sol; i++){
      printf("dual %d : %.2f\n", i, solution.row_dual[i]);
   }
   */

}

// get all the Dual_Values from the lp, m + n
void Master::GetDualValues(vector<double> &dual) 
{
   printf("Now get dual sols from lp to master \n");
   const HighsLp& lp = highs.getLp();
   const HighsSolution& solution = highs.getSolution();

   int N_Dual_sol =  lp.num_row_;
   printf("size : %d\n", solution.row_dual.size());
   printf("dual size : %d\n", dual.size());
   dual = vector<double>(N_Dual_sol, 0);
   for(int i = 0; i < N_Dual_sol; i++){
      printf("dual %d : %.2f\n", i, solution.row_dual[i]);
      dual[i] = solution.row_dual[i];
   }
   return;
}

void Master::print_current_solution(){
   const HighsLp& lp = highs.getLp();
   const HighsSolution& solution = highs.getSolution();
   const HighsBasis& basis = highs.getBasis();
   const HighsInfo& info = highs.getInfo();
   const bool has_values = info.primal_solution_status;
   
   const bool has_duals = info.dual_solution_status;
   const bool has_basis = info.basis_validity;

   printf("Print Master LP solution!\n");
   for (int col=0; col < lp.num_col_; col++) {
      cout << "Column " << col;
      if (has_values) cout << "; value = " << solution.col_value[col];
      /*
      if(solution.col_value[col] > 1e-6 && solution.col_value[col] < 1 - 1e-6){
         printf("Alert!!!!!! We find a non-integer LP solution in Master Problem !!!!!!!\n");
      }
      if( solution.col_value[col] > 1 - 1e-6){
         opt_scene_indices.push_back(col);
      }
      */
      if (has_duals) cout << "; dual = " << solution.col_dual[col];
      if (has_basis) cout << "; status: " << highs.basisStatusToString(basis.col_status[col]);
      cout << endl;
  }
   for (int row=0; row < lp.num_row_; row++) {
      cout << "Row    " << row;
      if (has_values) cout << "; value = " << solution.row_value[row];
      if (has_duals) cout << "; dual = " << solution.row_dual[row];
      if (has_basis) cout << "; status: " << highs.basisStatusToString(basis.row_status[row]);
      cout << endl;
   }
}

bool Master::check_is_integer_solution(){
   bool all_variable_is_integer = true;
   const HighsLp& lp = highs.getLp();
   const HighsSolution& solution = highs.getSolution();
   for (int col=0; col < lp.num_col_; col++) {
      if((col != grm->weapon_num_m) && !is_double_integer(solution.col_value[col])){
         all_variable_is_integer = false;
      }
   }   
   if(all_variable_is_integer){
      printf("All variable in opt solution of Master is Integer value\n");
   }
   else{
      printf("fractional value exist in the opt solution\n");
   }
   return all_variable_is_integer;
}

/**
 * @brief Use this to add scenes to the current master RP's ScenePool
 * @note  Do not delete scenes from the scenes Pool
*/
void Master::AddCol(vector<Scene_SSL> &scene_ssls) 
{

   return_status = highs.writeModel("Before_add_Master_LP.lp");
   if(return_status != HighsStatus::kOk){
      printf("Write Model failed! \n");
      exit(1);
   }
   int num_new_col_ = scene_ssls.size();
   double* cost_ = new double[num_new_col_];
   double* lower_ = new double[num_new_col_];
   double* upper_ = new double[num_new_col_];

   int* nz_starts_ = new int[num_new_col_+1];
   int num_new_nz_ = 0;

   int old_pool_size = ssl_pool.size();

   int m = grm->weapon_num_m;
   int n = grm->target_num_n;
   int k = grm->radar_num_k;

   int base_weapon_cq = 0;
   int base_radar_cq = m;
   int base_time_cq = m + k;
   int base_target_cq = m + k + n;
//------------------------- 0320 ----------------------------------
// First make a vector version, then change it to int* and double*

// Make a vector version 
int nz_counter = 0;
vector<int> start_index;
vector<int> nz_rows;
vector<double> nz_vals;

for(int i = 0; i < num_new_col_; i++){
   start_index.push_back(nz_counter);
   Scene_SSL temp_ssl_scene = scene_ssls[i];
   // Init A, weapon_row coef
   
   int weapon_nnz = 0;
   vector<int> weapon_nz_indices;
   vector<int> weapon_nz_values;

   convert_sparse_to_compress_int(temp_ssl_scene.activated_weapons_num, weapon_nnz, weapon_nz_indices, weapon_nz_values);

   for(int j = 0; j < weapon_nnz; j++){
      nz_rows.push_back(weapon_nz_indices[j]);
      nz_vals.push_back(static_cast<double>(weapon_nz_values[j]));
   }
   nz_counter += weapon_nnz;

   // Add radar row coef
   int radar_nnz = 0;
   vector<int> radar_nz_indices;
   vector<int> radar_nz_values;

   convert_sparse_to_compress_int(temp_ssl_scene.activated_radars_num, radar_nnz, radar_nz_indices, radar_nz_values);

   for(int j = 0; j < radar_nnz; j++){
      nz_rows.push_back(radar_nz_indices[j] + base_radar_cq);
      nz_vals.push_back(static_cast<double>(radar_nz_values[j]));
   }
   nz_counter += radar_nnz;


   int target_index_ = temp_ssl_scene.target_index;

   // Add time row coef
   int time_row_index = target_index_ + base_time_cq;
   nz_rows.push_back(time_row_index);
   nz_vals.push_back(grm->set_ssl_tjs(temp_ssl_scene));
   nz_counter++;
   printf("T row : row%d x_%d(%.2f) \n", time_row_index, old_pool_size + i,  grm->set_ssl_tjs(temp_ssl_scene));
   // Add target row coef
   int target_row_index = target_index_ + base_target_cq;
   nz_rows.push_back(target_row_index);
   nz_vals.push_back(1.0);
   nz_counter++;
}
start_index.push_back(nz_counter);

num_new_nz_ = nz_counter;

// set cost = q_js, lower 0, upper = ssl_capacity, add scene to scene_pool
   for(int i = 0; i < num_new_col_ ; i++){
      Scene_SSL temp_scene_ssl = scene_ssls[i];
      lower_[i] = 0.0;
      upper_[i] = grm->ssl_capacity;
      cost_[i]  = grm->set_ssl_qjs(temp_scene_ssl);
      nz_starts_[i] = start_index[i];
      ssl_pool.add_scene(temp_scene_ssl);
   }
   nz_starts_[num_new_col_] = start_index[num_new_col_];
   int* indices_ = new int[num_new_nz_];
   double* values_ = new double[num_new_nz_];
   int count_nz_num = 0;
   for(int i = 0; i < num_new_nz_; i++){
      indices_[i] = nz_rows[i];
      values_[i] = nz_vals[i];
   }
   printf("=================== ADD COLUMNS ================\n");
   printf("nz_start : ");
   for(int i = 0; i < num_new_col_; i++){
      printf("%d ", nz_starts_[i]);
   }
   printf("nz : \n");
   for(int i = 0; i < num_new_col_; i++){
      int start_index = nz_starts_[i];
      int end_index = nz_starts_[i+1];
      printf("col %d \n ind : \t", i);
      for(int j = start_index; j < end_index; j++){
         printf("%d\t", indices_[j]);
      }
      printf("\n val :\t");
      for(int j = start_index; j < end_index; j++){
         printf("%.2f\t", values_[j]);
      }
   printf("\n");
   }
   highs.addCols(num_new_col_, cost_, lower_, upper_, num_new_nz_, nz_starts_, indices_, values_);
   //return_status = highs.writeModel("New_Master.lp");

   return_status = highs.writeModel("After_add_Master_LP.lp");
   if(return_status != HighsStatus::kOk){
      printf("Write Model failed! \n");
      exit(1);
   }
   delete[] cost_;
   delete[] upper_;
   delete[] lower_;
   delete[] nz_starts_;
   delete[] indices_;
   delete[] values_;

   // This unit is used to print all the scenes in the scenepool after add them.
   bool DEBUG_PRINT_ALL_SCENES = 1;
   if(DEBUG_PRINT_ALL_SCENES){
      printf("Now Print all the scenes in the scene pool \n");
      for(int line = 0; line <1; line++){
         printf("\n");
      }
      ssl_pool.print_scene_by_target(n);
      for(int line = 0; line <1; line++){
         printf("\n");
      }
   }
}

/**
 * This function is deprecated. The math insure the new scene should not be repeated
 * @deprecated
*/
bool Master::Check_is_scenes_new(std::vector<Scene_SSL> &scenes){
   if(scenes.size() == 1){
      printf("No New Scenes \n");
      return false;
   }
   bool scenes_is_new = false;
   int target_num = scenes.size();
   int last_scene_index = ssl_pool.size();
   int first_scene_index = last_scene_index - target_num;
   for(int i = 0; i < target_num; i++){
      Scene_SSL old_scene = ssl_pool[first_scene_index + i];
      Scene_SSL new_scene = scenes[i];
      bool is_same_i = old_scene.is_same_w_r_t(new_scene);
      if(!is_same_i){
         scenes_is_new = true;
      }
   }
   if(!scenes_is_new){
      printf("We don't find new scene\n");
   }
   return scenes_is_new;
}
/**
 * @brief Use this function to pass LP solution to the Node
 * @todo callback the objective value from the solver
 * @todo callback weather the ith Scene has been used in this solution, if so pass the value to val
 * ！！！ 
*/
void Master::GetSol(Node &node) 
{
   node.lpsol = new LpSol();
   node.lpsol->obj;
   for( int i = 0; i < ssl_pool.size(); i++ )
   {
      double val = 0; // @todo callback the ??
      if( val > param.eps )
      {
         //node.lpsol->push_back(std::make_pair(&ssl_pool[i], val));
      }
   }
}


double Master::cal_obj_val(){
   int all_ssl_num_int = 0;
   double obj_value = 0;
   printf("Pool Size : %d\n", ssl_pool.size());
   /*
   printf("Now printf best cols \n");
   for(int i = 0; i < opt_scene_indices.size(); i++){
      printf("Num %d : %d\n", i , opt_scene_indices[i]);
   }
   */
  int m = grm->weapon_num_m;
  int k = grm->radar_num_k;

   // ssl_pool[opt_scene_indices[i]].target_index
   vector<int> FSC_num(m, 0);
   vector<int> LD_num(k, 0);
   printf("the optimal sol is : \n");
   for(int i = 0; i < opt_scene_indices.size(); i++){
      int temp_target_index = ssl_pool[opt_scene_indices[i]].target_index;
      double target_value = ssl_pool[opt_scene_indices[i]].obj_qjs;
      printf("value of target %d : %.2f\n", temp_target_index, target_value);
      obj_value += target_value;
      ssl_pool[opt_scene_indices[i]].PrintScene();
      for(int w = 0; w < m; w++){
         FSC_num[w] += ssl_pool[opt_scene_indices[i]].activated_weapons_num[w];
      }
      for(int r = 0; r < k; r++){
         LD_num[r] += ssl_pool[opt_scene_indices[i]].activated_radars_num[r];
      }
   }
   printf("optimal value is %.4f\n", obj_value);
   printf("Weapon Use : ");
   for(int i = 0; i < m; i++){
      printf("%d ", FSC_num[i]);
   }
   printf("\n Radar Use : ");
   for(int i = 0; i < k; i++){
      printf("%d ", LD_num[i]);
   }
   printf("\n");
   return obj_value;
}

int Master::convert_sparse_to_compress_int(vector<int> origin_vector, int& nnz, vector<int>& nz_pos, vector<int>& nz_val){
   int return_value = 1;
   nnz = 0;
   for(int i = 0; i < origin_vector.size(); i++){
      if(origin_vector[i] != 0){
         nz_pos.push_back(i);
         nz_val.push_back(origin_vector[i]);
         nnz++;
      }
   }
   return return_value;
}

int Master::convert_sparse_to_compress_double(vector<double> origin_vector, int& nnz, vector<int>& nz_pos, vector<double>& nz_val){
   int return_value = 1;
   nnz = 0;
   for(int i = 0; i < origin_vector.size(); i++){
      if(origin_vector[i] > EPS || origin_vector[i] < -EPS){
         nz_pos.push_back(i);
         nz_val.push_back(origin_vector[i]);
         nnz++;
      }
   }
   return return_value;
}


/*
void LP_ALL_IN_ONE::INITIALIZE_LP_BY_MASTER(WTA* wta, ScenePool scene_pool){
   
   printf("Now Initlized the LP my master problem\n");
   int m = wta->weapon_num_m;
   int n = wta->target_num_n;
   N_rows = wta->weapon_num_m + wta->target_num_n;
   N_cols = scene_pool.size();

   if(rhs){
      delete[] rhs;
      rhs = NULL;
   }
   if(A){
      delete[] A;
      A = NULL;
   }
   if(opt_dual_sol){
      delete[] opt_dual_sol;
      opt_dual_sol = NULL;
   }

   A = new vector<double>[N_rows];
   rhs = new double[N_rows];
   opt_dual_sol = new double[N_rows];
   opt_sol = vector<double>(N_cols, 0);
   

   cost = vector<double>(N_cols, 0);

   // Init cost
   for(int i = 0; i < N_cols; i++){
      Scene temp_scene = scene_pool[i];
      cost[i] = wta->set_scene_qjs(temp_scene);
   }

   // Init A, weapon row coef, change from column to row
   for(int i = 0; i < N_cols; i++){
      Scene temp_scene = scene_pool[i];
      vector<int> temp_weapon_indices = temp_scene.weapon_indices;
      int nz_col_num = temp_weapon_indices.size();
      for(int j = 0; j < nz_col_num; j++){
         int temp_weapon_index = temp_weapon_indices[j];
         A[temp_weapon_index].push_back(i);
      }
      // Init A, target row coef
      int target_index = temp_scene.target_index;
      int row_index = target_index + m;
      A[row_index].push_back(i);
   }

   

   lb = vector<double>(N_cols, 0);
   ub = vector<double>(N_cols, 0);

   for(int i = 0; i < N_rows; i++){
      rhs[i] = 1;
      opt_dual_sol[i] = 0;
   }
   opt_value = 0;
   LP_STATE = 0;
}


void LP_ALL_IN_ONE::PRINT_LP_INFO(){
   printf("Now print the LP info\n");
   printf("LP has %d rows and %d columns\n", N_rows, N_cols);
   printf("c:\n");
   for(int i = 0; i < N_cols; i++){
      printf("%.2f\t", cost[i]);
   }
   // print Ax \leq b
   printf("\n A: \n");
   for(int i = 0; i < N_rows; i++){
      int nz_num = A[i].size();
      printf("r%d:\t", i);
      if(nz_num == 0){
         for(int j = 0; j < N_cols; j++){
               printf("0\t");
         } 
      }
      else{
         for(int j = 0; j < N_cols; j++){
            int temp_nz_id = 0;
            int temp_nz_weapon = A[i][temp_nz_id];
            if(temp_nz_weapon == j){
               printf("1\t");
               temp_nz_id++;
            }
            else{
               printf("0\t");
            }
         }
      }
      
      printf("<=\t %.2f\n", rhs[i]);
   }

   // print lb and ub
   printf("lb & ub\n");
   printf("size: %d - %d\n", lb.size(), ub.size());

   for(int i = 0; i < N_cols; i++){
      printf("%d'th scene:\t%.2f - %.2f\n", i, lb[i], ub[i]);

   }

   printf("Opt Sol:");
   printf("size: %d\n", opt_sol.size());
   for(int i = 0; i < N_cols; i++){
      printf(" %.2f", opt_sol[i]);
   }
   printf("\n");
   printf("dual variable\n");
   for(int i = 0; i < N_rows; i++){
      printf("%.2f\n", opt_dual_sol[i]);
   }
   printf("\n");
}

void LP_ALL_IN_ONE::Delete(){
   if(A){
      delete[] A;
      A = NULL;
   }
   if(rhs){
      delete[] rhs;
      rhs = NULL;
   }
   if(opt_dual_sol){
      delete[] opt_dual_sol;
      opt_dual_sol = NULL;
   }
}


LP_ALL_IN_ONE::LP_ALL_IN_ONE(int _N_rows, int _N_cols){
   N_rows = _N_rows;
   N_cols = _N_cols;
   A = new vector<double>[N_rows];
   rhs = new double[N_rows];
   opt_dual_sol = new double[N_rows + N_cols];


   cost = vector<double>(N_cols, 0);
   lb = vector<double>(N_cols, 0);
   ub = vector<double>(N_cols, 0);

   for(int i = 0; i < N_rows; i++){
      rhs[i] = 0;
      opt_dual_sol[i] = 0;
   }
   for(int i = 0; i < N_cols; i++){
      opt_dual_sol[N_rows + i] = 0;
   }
   opt_value = 0;
   LP_STATE = 0;
}

LP_ALL_IN_ONE::~LP_ALL_IN_ONE(){
   if(rhs){
      delete[] rhs;
      rhs = NULL;
   }
   if(A){
      delete[] A;
      A = NULL;
   }
   if(opt_dual_sol){
      delete[] opt_dual_sol;
      opt_dual_sol = NULL;
   }
}

void LP_ALL_IN_ONE::SOLVE_LP(){
   printf("Now solve LP \n");
   
   //simplest 
   int is_simplest = 1;
   if(is_simplest == 1){
      printf("use this to calculate the simplest version\n");
      printf("Alert!!! Only correct when w_num = t_num\n");
      int m = N_rows / 2;
      int n = m;
      for(int i = 0; i < m; i++){
         opt_dual_sol[i] = 0;
      }
      for(int i = 0; i < n; i++){
         opt_dual_sol[i + m] = cost[i];
      }

   }
}

double* LP_ALL_IN_ONE::GET_DUAL_VALUE(){
   if(opt_dual_sol){
      return opt_dual_sol;
   }
   else{
      printf("LP haven't been solved \n");
      return NULL;
   }
}
*/