#include "SubProblem.h"

#include <ilcplex/cplex.h>


void SubProblem::Delete(){
    if(ssl_num > 0){
      delete[]    weapon_dual_ui;
      delete[]    radar_dual_muk;
      delete[]    j_prob_vector;
      delete[]    x;
      delete[]    cutind;
      delete[]    cutval;
    }
}

bool SubProblem::is_optval_geq_zero(){
   int DEBUG_PRINT_OBJ = 0;
   if(DEBUG_PRINT_OBJ){
      printf("obj val : %.2f\n", sub_optval);
   }
   bool return_value = false;
   if(sub_optval > -1e-8){
      sub_status = true;
      return_value = true;
   }
   return return_value;
}

void SubProblem::Init(GRM* _grm, int t_index, double* w_dual_ui, double* r_dual, double time_dual_wj_, double t_dual_vj, int seed_, int is_fractional, AlgoParameter* param){

   

   sub_optval = -INFINITY; // Is that right
   sub_status = 0;
   grm = _grm;

   ssl_num = grm->ssl_num;
   target_index = t_index;
   int j = t_index;
    

   int target_num = grm->target_num_n;
   weapon_num_m_ = grm->weapon_num_m;
   radar_num_k_ = grm->radar_num_k;
   //int n = target_num;
   


   weapon_dual_ui = new double[weapon_num_m_];
   radar_dual_muk = new double[radar_num_k_];

    for(int i = 0; i < weapon_num_m_; i++){
      weapon_dual_ui[i] = w_dual_ui[i];
    }
    for(int i = 0; i < radar_num_k_; i++){
      radar_dual_muk[i] = r_dual[i];
    }

    target_dual_vj = t_dual_vj;
    time_dual_wj = time_dual_wj_;

    target_value = grm->target_value[target_index];
    j_prob_vector = new double[ssl_num];
    for(int i = 0; i < ssl_num; i++){
        j_prob_vector[i] = grm->probability_matrix[i][j];
    }

    x = new double[ssl_num + 1];
    cutind = new int[ssl_num + 1];
    cutval = new double[ssl_num + 1];
    for(int i = 0; i < ssl_num+1; i++){
      x[i] = 0;
      cutind[i] = 0;
      cutval[i] = 0;
    }
   cutval[ssl_num] = -1;
   
   rhs = 0;

   seed = seed_;
   separate_fractional_solutions = 1;
   parameter = param;
}

double SubProblem::destroy_prob(){
    double prob = 1;
    for(int i = 0; i < ssl_num; i++){
        if(x[i] > WTA_EPS){
            prob = prob * pow(1 - j_prob_vector[i], x[i]);
        }
    }
    return prob;
}

double SubProblem::cal_fx(){
   int DEBUG = 0;
   if(DEBUG){
      print_debug();
   }
    double destroy_p = destroy_prob();
    double fx = target_value * destroy_p;
    for(int i = 0; i < ssl_num; i++){
        if(x[i] > WTA_EPS){
            int temp_weapon_index = cal_weapon_index(i);
            int temp_radar_index = cal_radar_index(i);
            fx = fx - (weapon_dual_ui[temp_weapon_index] + radar_dual_muk[temp_radar_index]) * x[i];
        }
    }
    fx = fx - target_dual_vj - time_dual_wj;
    return fx;
}

double SubProblem::__cal_fx_from_ssl_set(vector<int> test_scene){
   int DEBUG_PRINT = 0;
   if(DEBUG_PRINT){
      for(int i = 0; i < test_scene.size(); i++){
         printf("%d ", test_scene[i]);
      }
   }
   double destroy_p = 1;
   int temp_ssl_num = test_scene.size();
    for(int i = 0; i < temp_ssl_num; i++){
        if(test_scene[i] > WTA_EPS){
            destroy_p = destroy_p * pow(1 - j_prob_vector[i], test_scene[i]);
        }
    }
    double fx = target_value * destroy_p;
    if(DEBUG_PRINT){
      printf("destroy_prob : %.3e\n", destroy_p);
    }
    for(int i = 0; i < temp_ssl_num; i++){
        if(test_scene[i] > WTA_EPS){
            int temp_weapon_index = cal_weapon_index(i);
            int temp_radar_index = cal_radar_index(i);
            fx = fx - (weapon_dual_ui[temp_weapon_index] + radar_dual_muk[temp_radar_index]) * test_scene[i];
        }
    }
    fx = fx - target_dual_vj - time_dual_wj;
    return fx;
}

double SubProblem::cal_cutval(int i){
    double destroy_p = destroy_prob();
    double cut_val = target_value * destroy_p;
    cut_val = cut_val * log(1 - j_prob_vector[i]);
    int temp_weapon_index = cal_weapon_index(i);
    int temp_radar_index = cal_radar_index(i);
    cut_val = cut_val - weapon_dual_ui[temp_weapon_index] - radar_dual_muk[temp_radar_index];
    return cut_val;
}

double SubProblem::cal_rhs(){
    double temp_rhs = -cal_fx();
    int m = ssl_num;
    for(int i = 0; i < m; i++){
        if(x[i] > WTA_EPS){
            temp_rhs = temp_rhs + cal_cutval(i) * x[i];
        }
    }
    return temp_rhs;
}

double SubProblem::cal_eta_lower_bound(){
   // This method is more precise but costly
   // 
   /**
    * @deprecated
      double prob_part = target_value;
      int m = weapon_num;
      for(int i = 0; i < m; i++){
         prob_part = prob_part * (1 - j_prob_vector[i]);
      }
      double lower_bound = prob_part - target_dual_vj;
      return lower_bound;
   */
   return - target_dual_vj - time_dual_wj;
}

double SubProblem::cal_eta_upper_bound(){
   double dual_part = target_value;
   for(int i = 0; i < ssl_num; i++){
      int temp_weapon_index = cal_weapon_index(i);
      int temp_radar_index = cal_radar_index(i);
      dual_part = dual_part - weapon_dual_ui[temp_weapon_index] - radar_dual_muk[temp_radar_index];
   }
   double upper_bound = dual_part - target_dual_vj - time_dual_wj;
   return upper_bound;
}


void SubProblem::cal_constraint_by_x(vector<int> ssl_sets){

   // create x by weapon sets
   for(int i = 0; i < ssl_num+1; i++){
      x[i] = 0;
   }
   for(int i = 0; i < ssl_num; i++){
      //printf("weapon index: %d\n", weapon_index);
      x[i] = ssl_sets[i];
      cutind[i] = i;
      cutval[i] = cal_cutval(i);
   }
   // calculate coef by x
   rhs = cal_rhs();

   cutind[ssl_num] = ssl_num;
   cutval[ssl_num] = -1;
   return;
}

void SubProblem::print_LP_info(){
   // m is the col num without OA param eta
   int m = ssl_num;
   printf("x: \t");
   for(int i = 0; i < m+1; i++){
      printf("%.2f ", x[i]);
   }
   printf("\n");
   printf("ind:\t");
   for(int i = 0; i < m+1; i++){
      printf("%d ", cutind[i]);
   }
   printf("\n");
   printf("val: \t");
   for(int i = 0; i < m+1; i++){
      printf("%.2f ", cutval[i]);
   }
   printf("\nrhs:\t%.2f\n", rhs);
   return;
}

void SubProblem::print_model(){
   printf(" --- Print sub-Problem information ---\n");
   printf("target :\t%d \n", target_index);
   printf("tatget_dual :\t%.2f \n", target_dual_vj);
   printf("time_dual :\t%.2f \n", time_dual_wj);
   printf("weapon_dual :\t");
   for(int i = 0; i < weapon_num_m_; i++){
      printf("%.2f, ", weapon_dual_ui[i]);
   }
   printf("\n");
   printf("radar_dual :\t");
   for(int i = 0; i < radar_num_k_; i++){
      printf("%.2f, ", radar_dual_muk[i]);
   }
   printf("\n");
   printf("Target value :\t%.2f \n", target_value);
   printf("Obj:\nMin %.2f", target_value);
   for(int i = 0; i < ssl_num; i++){
      printf("(1-%.2f x_%d)", j_prob_vector[i], i);
   }

   for(int i = 0; i < ssl_num; i++){
         int temp_weapon_index = cal_weapon_index(i);
         int temp_radar_index = cal_radar_index(i);
      printf(" - (%.2f + %.2f)x_%d", weapon_dual_ui[temp_weapon_index], radar_dual_muk[temp_radar_index],i);
   }
   return;
   printf("- %.2f - %.2f \n", time_dual_wj, target_dual_vj);
   printf("x_i in [0,1] \n");
   return;
}

void SubProblem::print_debug(){
   print_model();
   return;
   for(int i = 0; i < ssl_num; i++){
      printf("x_%d : %.2f", i, x[i]);
   }
   printf("\n");

}

vector<int> SubProblem::cal_optimal_scene(Scene_SSL& scene_ssl){
    int DEBUG_MODE = parameter->DEBUG_VERSION;
    int DEBUG_PRINT_SUBPROB_SOLUTION = 0;
    int status = 0;
    int solstat;

    double objval;    
    int num_x_cols;

    double *x = NULL; 
    int i, j;

    int m = grm->weapon_num_m;
    int n = grm->target_num_n;
    int k = grm->radar_num_k;
    int _seed = 1;

   /* Cplex environment and master ILP */

   CPXENVptr env = NULL;
   CPXLPptr  lp = NULL;

   double cplex_start_time;
   double cplex_end_time;
   double running_time;
   // This is the return value, the best weapon set that solve this subproblem
   vector<int> best_ssl_set(ssl_num, 0);

   //print_model();


   /* Decide when Benders' cuts are going to be separated:
0: only when a integer solution if found
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_FEAS )
1: even to cut-off fractional solutions, 
at the end of the cplex cut-loop
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_LAST || 
wherefrom == CPX_CALLBACK_MIP_CUT_FEAS ) */

   int separate_fractional_solutions; 

   /* Cut callback data structure */

   // Set separate_fractional_solutions
   
   // wta.separate_fractional_solutions = separate_fractional_solutions;
   printf("Target:\t%d\n", target_index);
   int DEBUG_PRINT_BASIC_INFO = 0;
   if(DEBUG_PRINT_BASIC_INFO){
         printf("proble size: m = %d , n = %d , seed = %d \n", m, n, _seed);
         printf ("Benders' cuts separated to cut off: ");
         if ( separate_fractional_solutions ) {
            printf ("Integer and fractional infeasible solutions.\n");
         }
         else {
            printf ("Only integer infeasible solutions.\n");
      }
   }
   fflush (stdout);

   /* Init the CPLEX environment */
   env = CPXopenCPLEX (&status);
   if ( env == NULL ) {
      fprintf (stderr, "Failure in CPXopenCPLEX, status = %d.\n", status);
      goto TERMINATE;
   }

   /* Turn on output to the screen */

   status = CPXsetintparam (env, CPXPARAM_ScreenOutput, CPX_ON); 
   if ( status ) {
      fprintf (stderr, "Failed to turn on screen indicator, status = %d.\n",
               status);
      goto TERMINATE;
   }

   // This three commands are used to shrink the output info
   status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
      if ( status ) {
      fprintf (stderr, "Failed to turn off  CPX_PARAM_SCRIND, status = %d.\n",
               status);
      goto TERMINATE;
   }
   status = CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 0);
   if ( status ) {
      fprintf (stderr, "Failed to turn off CPX_PARAM_MIPDISPLAY, status = %d.\n",
               status);
      goto TERMINATE;
   }
   status = CPXsetintparam(env,  CPX_PARAM_MIPINTERVAL, -1);
   if ( status ) {
      fprintf (stderr, "Failed to turn off CPX_PARAM_MIPDISPLAY, status = %d.\n",
               status);
      goto TERMINATE;
   }


   /* Create the master ILP */
   status = CPXgettime(env, &cplex_start_time);
   if ( status ) {
      fprintf (stderr,
               "Failed to get start time.\n");
      goto TERMINATE;
   }


   lp = CPXcreateprob (env, &status, "master_ILP.lp");
   if ( lp == NULL ) {
      fprintf (stderr, "Failure in CPXcreateprob, status = %d.\n", status);
      goto TERMINATE;
   }


   status = create_master_ILP (env, lp, this);
   if ( status ) {
      fprintf (stderr,
               "Failed to create the master ILP.\n");
      goto TERMINATE;
   }

// Write Model
   status = CPXwriteprob(env, lp, "After_Init_cut.lp", NULL);
   if( status ){
      fprintf (stderr, "Error in Write Problem, status = %d.\n", status);
         goto TERMINATE;
   }


   /* Set up environment parameters to use the function benders_callback 
      as cut callback function */
   status = set_benders_callback (env, this);
   if ( status ) {
      fprintf (stderr,
               "Failure in function set_benders_callback: status = %d.\n",
               status);
      goto TERMINATE;
   }

   // /* Optimize the problem and obtain solution status */

   status = CPXmipopt (env, lp);
   if ( status ) {
      fprintf (stderr, "Failed to optimize MIP, status = %d.\n", status);
      goto TERMINATE;
   }
   
   solstat = CPXgetstat (env, lp);
   printf ("\nSolution status: %d\n", solstat);

   /* Write out the objective value */ 
   status = CPXgettime(env, &cplex_end_time);
   if ( status ) {
      fprintf (stderr,
               "Failed to get end time.\n");
      goto TERMINATE;
   }


   if ( CPXgetobjval (env, lp, &objval) ) {
      printf ("Failed to obtain objective value.\n");
   }
   else {
      printf ("Objective value: %17.10e\n", objval);
   }

   if ( solstat == CPXMIP_OPTIMAL ) {
      /* Write out the optimal tour */ 
      printf ("Solution status is CPX_STAT_OPTIMAL\n");
      num_x_cols = CPXgetnumcols (env, lp);
      x = (double*)malloc (num_x_cols * sizeof(*x));
      if ( x == NULL ) {
         fprintf (stderr, "No memory for x array.\n");
         status = -1;
         goto TERMINATE;
      }
      status = CPXgetx (env, lp, x, 0, num_x_cols-1);
      if ( status ) {
         fprintf (stderr, "Failed to obtain solution, status = %d.\n", status);
         goto TERMINATE;
      }

      printf("Node :\t%d\n", CPXgetnodecnt(env, lp));
   } 
   else {
      printf ("Solution status is not CPX_STAT_OPTIMAL\n");
      printf("Node :\t%d\n", CPXgetnodecnt(env, lp));
   }

   running_time = cplex_end_time - cplex_start_time;
   printf("Time :\t%e\n",target_index ,running_time);


   for(int i = 0; i < ssl_num; i++){
      best_ssl_set[i] = round(x[i]);
   }

   // This block is used to show the x from cpx
   DEBUG_PRINT_SUBPROB_SOLUTION = 0;

   if(DEBUG_PRINT_SUBPROB_SOLUTION){
      printf("Solution:\n");
      for(int i = 0; i < num_x_cols; i++){
         printf("x%d\t", i);
      }
      printf("\n");
      for(int i = 0; i < num_x_cols; i++){
         printf("%.2f\t", x[i]);
      }
      printf("\n");
      }
   sub_optval =  x[num_x_cols - 1];

TERMINATE:

   /* Free the allocated memory if necessary */
   free_and_null ((char **) &x);

   if ( lp != NULL ) {
      int local_status = CPXfreeprob (env, &lp);
      if ( local_status ) {
         fprintf (stderr, "CPXfreeprob failed, error code %d.\n",
                  local_status);
         status = local_status;
      }
   }

   /* Free the CPLEX environment, if necessary */

   if ( env != NULL ) {
      int local_status = CPXcloseCPLEX (&env);
      if ( local_status ) {
         fprintf (stderr, 
                  "Could not close CPLEX environment, status = %d.\n", 
                  local_status);
         status = local_status;
      }
   }
   scene_ssl.Set_Scene(target_index, best_ssl_set, m, k);

   int DEBUG_SCENE = 1;
   if(DEBUG_SCENE){
      printf("Now pass the scene to the Scene pool : \n");
      scene_ssl.PrintScene();
   }
   return best_ssl_set;
   //return status;
}


/* This routine initializes the data structure for 
   the user callback function benders_callback.
   In particular, it creates the worker LP that will be used by 
   the function benders_callback to separate Benders' cuts.

   Worker LP model (dual of flow constraints and 
   capacity constraints of the flow MILP)

   Modeling variables:
   forall k in V0, i in V:
      u(k,i) = dual variable associated with flow constraint (k,i)
             
   forall k in V0, forall (i,j) in A: 
      v(k,i,j) = dual variable associated with capacity constraint (k,i,j)
   
   Objective:
   minimize sum(k in V0) sum((i,j) in A) x(i,j) * v(k,i,j) 
            - sum(k in V0) u(k,0) + sum(k in V0) u(k,k)

   Constraints:
   forall k in V0, forall (i,j) in A: u(k,i) - u(k,j) <= v(k,i,j)

   Nonnegativity on variables v(k,i,j)
   forall k in V0, forall (i,j) in A: v(k,i,j) >= 0 */

static int
init_user_cbhandle  (USER_CBHANDLE *user_cbhandle, int num_nodes, 
                     int separate_fractional_solutions)
{
   int i, j, k;
   int status = 0;
   
   /* Data structures to create columns and add rows */

   int num_rows, nzcnt;
   char *sense = NULL;
   double *rhs = NULL;
   int *rmatbeg = NULL;
   int *rmatind = NULL;
   double *rmatval = NULL;
   char *colname = NULL;

   /* Init user_cbhandle */

   user_cbhandle->separate_fractional_solutions = separate_fractional_solutions;
   user_cbhandle->num_nodes  = num_nodes;
   user_cbhandle->num_x_cols = num_nodes * num_nodes;
   user_cbhandle->num_v_cols = (num_nodes - 1) * user_cbhandle->num_x_cols;
   user_cbhandle->num_u_cols = (num_nodes - 1) * num_nodes;
   user_cbhandle->env        = NULL;
   user_cbhandle->lp         = NULL;
   user_cbhandle->x          = NULL;
   user_cbhandle->indices    = NULL;
   user_cbhandle->ray        = NULL;
   user_cbhandle->cutval     = NULL;
   user_cbhandle->cutind     = NULL;
   
   user_cbhandle->x = (double*)malloc (user_cbhandle->num_x_cols *
                              sizeof(*user_cbhandle->x));
   if ( user_cbhandle->x == NULL ) {
      fprintf (stderr, "No memory for x array.\n");
      goto TERMINATE;
   }
   user_cbhandle->indices = (int* )malloc (user_cbhandle->num_x_cols *
                                    sizeof(*user_cbhandle->indices));
   if ( user_cbhandle->indices == NULL ) {
      fprintf (stderr, "No memory for indices array.\n");
      goto TERMINATE;
   }
   user_cbhandle->ray = (double*)malloc ((user_cbhandle->num_v_cols +
                                 user_cbhandle->num_u_cols) *
                                sizeof(*user_cbhandle->ray));
   if ( user_cbhandle->ray == NULL ) {
      fprintf (stderr, "No memory for ray array.\n");
      goto TERMINATE;
   }
   user_cbhandle->cutval = (double*)malloc (user_cbhandle->num_x_cols *
                                   sizeof(*user_cbhandle->cutval));
   if ( user_cbhandle->cutval == NULL ) {
      fprintf (stderr, "No memory for cutval array.\n");
      goto TERMINATE;
   }
   user_cbhandle->cutind = (int*)malloc (user_cbhandle->num_x_cols *
                                   sizeof(*user_cbhandle->cutind));
   if ( user_cbhandle->cutind == NULL ) {
      fprintf (stderr, "No memory for cutind array.\n");
      goto TERMINATE;
   }
   
   /* Create the environment for the worker LP */

   user_cbhandle->env = CPXopenCPLEX (&status);
   if ( user_cbhandle->env == NULL ) {
      fprintf (stderr, 
         "Could not open CPLEX environment for the worker LP: status = %d.\n", 
         status); 
      goto TERMINATE;
   }

   /* Turn off the presolve reductions */

   status = CPXsetintparam (user_cbhandle->env, CPXPARAM_Preprocessing_Reduce, 0);
   if ( status ) {
      fprintf(stderr, 
         "Failed to set CPXPARAM_Preprocessing_Reduce, status = %d.\n", status);
      goto TERMINATE;
   }

   /* Create the worker LP */

   user_cbhandle->lp = CPXcreateprob (user_cbhandle->env, &status, "atsp_worker.lp");
   if ( user_cbhandle->lp == NULL ) {
      fprintf (stderr, "Failed to create the worker LP: status = %d\n", status);
      goto TERMINATE;
   }

   /* Allocate memory for column names */

   colname = (char*)malloc (100 * sizeof(*colname));
   if ( colname == NULL ) {
      fprintf (stderr, "No memory for colname array.\n");
      status = -1;
      goto TERMINATE;
   }
  
   /* Create variables v(k,i,j), one per time 
      For simplicity, also dummy variables v(k,i,i) are created.
      Those variables are fixed to 0 and do not partecipate to 
      the constraints */
   
   for (k = 1; k < num_nodes; ++k) {
      for (i = 0; i < num_nodes; ++i) {
         for (j = 0; j < num_nodes; ++j) {
            double ub = ( i == j ? 0. : CPX_INFBOUND );
            sprintf (colname, "v.%d.%d.%d", k, i, j);
            status = CPXnewcols (user_cbhandle->env, user_cbhandle->lp, 1, 
                                 NULL, NULL, &ub, NULL, &colname);
            if ( status ) {
               fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
               goto TERMINATE;
            }
         }
      }
   }

   /* Create variables u(k,i), one per time */
   
   for (k = 1; k < num_nodes; ++k) {
      for (i = 0; i < num_nodes; ++i) {
         double obj = 0.;
         double lb = -CPX_INFBOUND;
         double ub = CPX_INFBOUND;
         sprintf (colname, "u.%d.%d", k, i);
         if ( i == 0 )
            obj = -1.;
         else if ( i == k )
            obj = 1.;
         status = CPXnewcols (user_cbhandle->env, user_cbhandle->lp, 1, 
                              &obj, &lb, &ub, NULL, &colname); 
         if ( status ) {
            fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
            goto TERMINATE;
         }
      }
   }

   /* Init data structures for CPXaddrows */
   
   num_rows = user_cbhandle->num_x_cols * (num_nodes - 1);

   rhs = (double*)malloc (num_rows * sizeof (*rhs));
   if ( rhs == NULL ) {
      fprintf (stderr, "No memory for rhs array.\n");
      status = -1;
      goto TERMINATE;
   }
   sense = (char*)malloc (num_rows * sizeof (*sense));
   if ( sense == NULL ) {
      fprintf (stderr, "No memory for sense array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatbeg = (int*)malloc ( (num_rows + 1) * sizeof (*rmatbeg));
   if ( rmatbeg == NULL ) {
      fprintf (stderr, "No memory for rmatbeg array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatind = (int*)malloc (3 * num_rows * sizeof (*rmatind));
   if ( rmatind == NULL ) {
      fprintf (stderr, "No memory for rmatind array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatval = (double*)malloc (3 * num_rows * sizeof (*rmatval));
   if ( rmatval == NULL ) {
      fprintf (stderr, "No memory for rmatval array.\n");
      status = -1;
      goto TERMINATE;
   }

   /* Populate data structures for CPXaddrows and add all the constraints:
      forall k in V0, forall (i,j) in A: u(k,i) - u(k,j) <= v(k,i,j) */
   
   num_rows = 0;
   nzcnt = 0;
   for (k = 1; k < num_nodes; ++k) {
      for (i = 0; i < num_nodes; ++i) {
         for (j = 0; j < num_nodes; ++j) {
            if ( i != j ) {
               rhs[num_rows]     =  0.;
               sense[num_rows]   =  'L';
               rmatbeg[num_rows] =  nzcnt;
               rmatind[nzcnt]    =  (k-1) * user_cbhandle->num_x_cols +
                                    i * num_nodes + j;
               rmatval[nzcnt++]  = -1.;
               rmatind[nzcnt]    =  user_cbhandle->num_v_cols + 
                                    (k-1) * num_nodes + i; 
               rmatval[nzcnt++]  =  1.;
               rmatind[nzcnt]    =  user_cbhandle->num_v_cols + 
                                    (k-1) * num_nodes + j; 
               rmatval[nzcnt++]  = -1.;
               ++num_rows;
            }
         }
      }
   }
   rmatbeg[num_rows] = nzcnt; 

   status = CPXaddrows (user_cbhandle->env, user_cbhandle->lp, 
                        0, num_rows, nzcnt, rhs, sense, 
                        rmatbeg, rmatind, rmatval, NULL, NULL);
   if ( status ) {
      fprintf (stderr, "Error in CPXaddrows: status = %d\n", status);
      goto TERMINATE;
   }
   
TERMINATE:

   free_and_null ((char **) &colname);
   free_and_null ((char **) &sense);
   free_and_null ((char **) &rhs);
   free_and_null ((char **) &rmatbeg);
   free_and_null ((char **) &rmatind);
   free_and_null ((char **) &rmatval);

   return status;

} /* END init_user_cbhandle */
   

/* 
This routine frees up the data structure for the user cutcallback
created by init_user_cbhandle
*/

static int
free_user_cbhandle  (USER_CBHANDLE *user_cbhandle)
{
   int status = 0; 

   if (user_cbhandle == NULL) goto TERMINATE;
   
   free_and_null ((char **) &user_cbhandle->x);
   free_and_null ((char **) &user_cbhandle->indices);
   free_and_null ((char **) &user_cbhandle->ray);
   free_and_null ((char **) &user_cbhandle->cutval);
   free_and_null ((char **) &user_cbhandle->cutind);
   
   if ( user_cbhandle->lp != NULL ) {
      int local_status = CPXfreeprob (user_cbhandle->env, &(user_cbhandle->lp) );
      if ( local_status ) {
         fprintf (stderr, "CPXfreeprob failed, error code %d.\n", status);
         status = local_status;
      }
      else
         user_cbhandle->lp = NULL;
   }

   if ( user_cbhandle->env != NULL ) {
      int local_status = CPXcloseCPLEX ( &(user_cbhandle->env) );
      if ( local_status ) {
         fprintf (stderr, "CPXcloseCPLEX failed, error code %d.\n", status);
         status = local_status;
      }
      else
         user_cbhandle->env = NULL;
   }

TERMINATE:

   return status; 

} /* END free_user_cbhandle */


/* This routine sets up the environment parameters to use the function
   benders_callback to separate and add Bender's cut during the branch-and-cut.
   benders_callback is always set to generate Benders' cuts 
   as lazy constraints, with CPXsetlazyconstraintcallbackfunc.
   Depending on the parameter user_cbhandle->separate_fractional_solutions,
   benders_callback may also be set to generate Benders' cut 
   as user cutting planes, with CPXsetusercutcallbackfunc. */

static int 
set_benders_callback  (CPXENVptr env, SubProblem* sub_prob)
{
   int status = 0;

   /* Let MIP callbacks work on the original model */

   int temp_seed = sub_prob->seed;
   
   //printf("seed is %d\n", temp_seed);

   status = CPXsetintparam (env, CPXPARAM_MIP_Strategy_CallbackReducedLP,
                            CPX_OFF);
   if ( status )  {
      fprintf (stderr,
        "Failed to set CPXPARAM_MIP_Strategy_CallbackReducedLP, status = %d.\n",
        status);
      goto TERMINATE;
   }

   /* Set the maximum number of threads to 1. 
      This instruction is redundant: If MIP control callbacks are registered, 
      then by default CPLEX uses 1 (one) thread only.
      Note that the current example may not work properly if more than 1 threads 
      are used, because the callback functions modify shared global data.
      We refer the user to the documentation to see how to deal with multi-thread 
      runs in presence of MIP control callbacks. */
   /*
   */
   status = CPXsetintparam (env, CPXPARAM_Threads, 1);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPXPARAM_Threads, status = %d.\n", status);
      goto TERMINATE;
   }
   
   status = CPXsetdblparam (env, CPX_PARAM_EPAGAP, 0.0);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPX_PARAM_EPAGAP, status = %d.\n", status);
      goto TERMINATE;
   }
   
   status = CPXsetdblparam (env, CPX_PARAM_EPGAP, 0.0);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPX_PARAM_EPGAP, status = %d.\n", status);
      goto TERMINATE;
   }
  
   /* Turn on traditional search for use with control callbacks */

   status = CPXsetintparam (env, CPXPARAM_MIP_Strategy_Search,
                            CPX_MIPSEARCH_TRADITIONAL);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPXPARAM_MIP_Strategy_Search, status = %d.\n",
             status);
      goto TERMINATE;
   }
   
   status = CPXsetdblparam (env, CPX_PARAM_TILIM, 3600);
   
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPX_PARAM_TILIM, status = %d.\n",
             status);
      goto TERMINATE;
   }



   status = CPXsetintparam(env, CPX_PARAM_RANDOMSEED, temp_seed);

   /* Set up to use the function benders_callback to generate lazy constraints */

   status = CPXsetlazyconstraintcallbackfunc (env, benders_callback, 
                                                sub_prob);
   if ( status )  {
      fprintf (stderr,
         "Error in CPXsetlazyconstraintcallbackfunc, status = %d.\n",
         status);
      goto TERMINATE;
   }

   /* Set up to use the function benders_callback also to generate user cuts */
   
   if ( sub_prob->separate_fractional_solutions ) {
      status = CPXsetusercutcallbackfunc (env, benders_callback, 
                                          sub_prob);
      if ( status )  {
          fprintf (stderr,
             "Error in CPXsetusercutcallbackfunc, status = %d.\n",
             status);
         goto TERMINATE;
      }
   }

TERMINATE:

   return status;

} /* END set_benders_callback */

double SubProblem::cal_reduced_cost(Scene_SSL& temp_ssl_scene){

   double red_cost = grm->set_ssl_qjs(temp_ssl_scene);
   red_cost -= target_dual_vj;
   red_cost -= time_dual_wj;
   for(int i = 0; i < weapon_num_m_; i++){
      red_cost -= weapon_dual_ui[i] * temp_ssl_scene.activated_weapons_num[i];
   }
   for(int i = 0; i < radar_num_k_; i++){
      red_cost -= radar_dual_muk[i] * temp_ssl_scene.activated_radars_num[i];
   }
   temp_ssl_scene.redcost = red_cost;
   return red_cost;
}

/* This function separate Benders' cuts violated by the current solution and
   add them to the current relaxation through CPXcutcallbackadd.
   If benders_callback is called as lazy constraints callback, then 
   wherefrom can be CPX_CALLBACK_MIP_CUT_FEAS or CPX_CALLBACK_MIP_CUT_UNBD.
   If benders_callback is called as user cut callback, then 
   wherefrom can be CPX_CALLBACK_MIP_CUT_LOOP or CPX_CALLBACK_MIP_CUT_LAST.
   Note that the case CPX_CALLBACK_MIP_CUT_UNBD cannot occur in our context, 
   the current LP relaxation is bounded by construnction. */

static int CPXPUBLIC 
benders_callback  (CPXCENVptr env, void *cbdata, int wherefrom, 
                   void *cbhandle, int *useraction_p) 
{
   // Those variables are used to decide how to add cut 
   const int ONLY_OPTIMAL = 1;
   const int ALLOW_FEASIBLE = 0;
   int ADD_CUT_MODE = 1;
   bool is_add_cut = false;

   int DEBUG_LP = 0;
   
   int status = 0;
   int do_separate = 0;

   SubProblem* sub_prob = (SubProblem*) cbhandle;
   int x_num_ = sub_prob->ssl_num;
   int purgeable;
   double sum = 0;

   /* Data structures to add the Benders' cut */

   *useraction_p = CPX_CALLBACK_DEFAULT;
   
   
   /* Decide if we want to separate cuts, depending on 
      the parameter wherefrom */
   
   switch (wherefrom) {
      case CPX_CALLBACK_MIP_CUT_FEAS: 
         do_separate = 1; 
         break;
      case CPX_CALLBACK_MIP_CUT_LAST: 
         do_separate = 1;
         break;
      case CPX_CALLBACK_MIP_CUT_LOOP: 
         do_separate = 0;
         break;
      default:
         fprintf (stderr, "Unexpected value of wherefrom: %d\n", wherefrom);
         do_separate = 0;
   }

   if( !do_separate ) goto TERMINATE; 

   /* Get the current x solution */
   
   status = CPXgetcallbacknodex (env, cbdata, wherefrom, sub_prob->x, 0, x_num_);
   if ( status ) {
      fprintf (stderr, "Error in CPXgetcallbacknodex: status = %d\n", status);
      goto TERMINATE;
   }




sub_prob->rhs = sub_prob->cal_rhs();
   for (int i = 0; i < x_num_; ++i) 
   {
      sub_prob->cutind[i] = i;
      sub_prob->cutval[i] = sub_prob->cal_cutval(i);
   }
   sub_prob->cutind[x_num_] = x_num_;
   sub_prob->cutval[x_num_] = -1;   

 
   if(DEBUG_LP){
   printf(" Now print LP info \n");
   sub_prob->print_LP_info();
   }


   purgeable = CPX_USECUT_FORCE;

   /* With this choice of the purgeable parameter,
      the cut is added to the current relaxation and
      it cannot be purged.
      Note that the value CPX_USECUT_FILTER is not allowed if
      Benders' cuts are added as lazy constraints (i.e., if 
      wherefrom is CPX_CALLBACK_MIP_CUT_LOOP or 
      CPX_CALLBACK_MIP_CUT_LAST). 
      Possible values and meaning of the purgeable parameter 
      are illustrated in the documentation of CPXcutcallbackadd */

   /* Add the cut to the master ILP */
   // @date 0206 Use this to control weather add a frac cut
   if(DEBUG_LP){

   }
   
   //printf("Check if need cut\n");
   //printf("eta : %.4f , f_x : %.4f\n", sub_prob->x[m], sub_prob->cal_fx());

   //printf("diff : %.4e\n", sub_prob->cal_fx() - sub_prob->x[m] );
   //assert((sub_prob->x[m] + WTA_EPS < sub_prob->cal_fx()) == (sub_prob->cal_fx() < -WTA_EPS));
   ADD_CUT_MODE = ALLOW_FEASIBLE;
   if(ADD_CUT_MODE == ONLY_OPTIMAL){
      is_add_cut = (sub_prob->x[x_num_] + WTA_EPS < sub_prob->cal_fx());
   }
   if(ADD_CUT_MODE == ALLOW_FEASIBLE){
      //printf("--------Allow Feasible--------\n");
      is_add_cut = (sub_prob->cal_fx() > -WTA_EPS);
   }
   
   if(is_add_cut)
   {
      //printf("------ the cut is from %ld ---------\n", wherefrom);
      bool is_integer = sub_prob->print_solution();
      //assert(is_integer);
      if(!is_integer)
      {
         //printf("add a filter cut.\n" );
         purgeable = CPX_USECUT_FILTER;
         if(wherefrom != CPX_CALLBACK_MIP_CUT_LAST)
         {
            printf("CPX_CALLBACK_MIP_CUT_FEAS = %d\n", wherefrom == CPX_CALLBACK_MIP_CUT_FEAS);
         }
      }
      /*
      else
      {
         printf("add a force cut.\n" );
         if(wherefrom != CPX_CALLBACK_MIP_CUT_FEAS)
            printf("CPX_CALLBACK_MIP_CUT_LAST = %d\n", wherefrom == CPX_CALLBACK_MIP_CUT_LAST);
      }
      double ss = 0;
      double ff = 1;
      for(int ii = 0; ii < nzcnt - 1; ii++)
      {
         int ind = wta->cutind[ii];
         ss += wta->cutval[ii] * (wta->real_sol[ind] - wta->x[ind]);
         ff = ff * (1 - wta->p[ii][j] * wta->x[ind]);
      }
      if(ss + ff > wta->real_sol[m*n+j] + 1e-8)
      {
         for(int ii = 0; ii < nzcnt - 1; ii++)
         {
            int ind = wta->cutind[ii];
            //if(wta->real_sol[ind] > 1e-8)
               printf("%.5f %.5f(%.0f,%.0f) ", wta->p[ii][j], wta->cutval[ii], wta->x[ind], wta->real_sol[ind]);
         }
         int ind = wta->cutind[nzcnt-1];
         //if(wta->real_sol[ind] > 1e-8)
         printf("%.5f(%.8f,%.8f) ", wta->cutval[nzcnt-1],  wta->x[m*n + j],  wta->real_sol[m*n + j]);
         printf(" --- %d\n", j);
      }
      assert(ss + ff <= wta->real_sol[m*n+j] + 1e-8);

      printf("rhs = %.2f, naznt = %d \n", wta->rhs, nzcnt);
      */

     // this block is used to check the new cut generated from the current point
     int DEBUG_CUT = 0;
     if(DEBUG_CUT){
         printf("linear coef of new cut : \n");
         for(int i = 0; i < x_num_; i++)
         {
            printf("x_%d : %.4f ",sub_prob->cutind[i], sub_prob->cutval[i]);
         }
         printf("theta : %.4f\n", sub_prob->cutval[x_num_]);
     }

      status = CPXcutcallbackadd (env, cbdata, wherefrom, x_num_+1, sub_prob->rhs, 'L', 
         sub_prob->cutind, sub_prob->cutval,
         purgeable);
   if ( status ) {
      fprintf (stderr, "Error in CPXcutcallbackadd: status = %d\n", status);
      goto TERMINATE;
   }
   }
      /* Tell CPLEX that cuts have been created */ 

   *useraction_p = CPX_CALLBACK_SET; 

TERMINATE:

      /* If an error has been encountered, we fail */

   if ( status ) *useraction_p = CPX_CALLBACK_FAIL; 

   return status;

} /* END benders_callback */


/* This routine creates the master ILP (arc variables x and degree constraints).

   Modeling variables:
   forall (i,j) in A: 
      x(i,j) = 1, if arc (i,j) is selected
             = 0, otherwise

   Objective:
   minimize sum((i,j) in A) c(i,j) * x(i,j)

   Degree constraints:
   forall i in V: sum((i,j) in delta+(i)) x(i,j) = 1
   forall i in V: sum((j,i) in delta-(i)) x(j,i) = 1

   Binary constraints on arc variables:
   forall (i,j) in A: x(i,j) in {0, 1} */

static int
create_master_ILP   (CPXENVptr env, CPXLPptr lp, SubProblem* subproblem)
{
   int DEBUG = 0;
   int i, j;
   int status = 0;
   //int n;
   int ssl_num = subproblem->ssl_num;
   int weapon_num = subproblem->weapon_num_m_;
   int radar_num = subproblem->radar_num_k_;

   int ssl_capacity_ = subproblem->grm->ssl_capacity;

   double w = subproblem->target_value;
   double* p = subproblem->j_prob_vector;
  
   char sense, *colname = NULL;
   int nzcnt, rmatbeg, *rmatind = NULL;
   double rhs, *rmatval = NULL;

   // Add how many init cuts
   int Init_cut_num = 1;
   int wt_ratio = 1;
   vector<vector<int>> Init_weapon_sets(Init_cut_num, vector<int>(ssl_num, 0));

   subproblem->find_init_cut(Init_weapon_sets, Init_cut_num, 1);



   /* Change problem type */
   
   status = CPXchgprobtype (env, lp, CPXPROB_MILP);
   if ( status ) {
      fprintf (stderr, "Error in CPXchgprobtype, status = %d.\n", status);
      goto TERMINATE;
   }

   /* Create arc variables x(i,j), one per time 
      For simplicity, also dummy variables x(i,i) are created.
      Those variables are fixed to 0 and do not partecipate to 
      the constraints */
   
   colname = (char*)malloc ( 100* sizeof(*colname));
   if ( colname == NULL ) {
      fprintf (stderr, "No memory for colname array.\n");
      status = -1;
      goto TERMINATE;
   }

   // Initialize the x_i in Model



   for (i = 0; i < ssl_num; ++i) 
   {
      double cost = 0;
      double lb   = 0;
      double ub   = ssl_capacity_;
      char type   = 'I';
      sprintf (colname, "x.%d", i);
      status = CPXnewcols (env, lp, 1, &cost, &lb, &ub, &type, &colname);
      if ( status ) {
         fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
         goto TERMINATE;
      }
   }



   // Init theta
   {
      double cost = 1;
      double lb   = 0;
      double ub   = 0;
      lb = subproblem->cal_eta_lower_bound();
      ub = subproblem->cal_eta_upper_bound();
      char type   = 'C';
      sprintf (colname, "theta.%d", j);
      status = CPXnewcols (env, lp, 1, &cost, &lb, &ub, &type, &colname);
      if ( status ) {
         fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
         goto TERMINATE;
      }
   }

   //printf("------ Check Memory ------ \n");
   //goto TERMINATE;   // ----------------------------------------------------------

   /* Init data structures to add capacity constraints */
   // @todo understand what is the capacity constraints, do I need this?
   rhs = 1.;
   sense = 'L';
   rmatbeg = 0;
   rmatind = (int*)malloc ( ssl_num * sizeof (*rmatind));
   if ( rmatind == NULL ) {
      fprintf (stderr, "No memory for rmatind array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatval = (double*)malloc ( ssl_num * sizeof (*rmatval));
   if ( rmatval == NULL ) {
      fprintf (stderr, "No memory for rmatval array.\n");
      status = -1;
      goto TERMINATE;
   }


   /* Add the out degree constraints, one at a time:
      forall i in V: sum((i,j) in delta+(i)) x(i,j) = 1 */
   
   // @todo I think this step is not necessary
/*

   for (i = 0; i < m; ++i) {
      nzcnt = 0;
      for (j = 0; j < n; ++j) {
         rmatind[nzcnt]   = i * n + j;
         rmatval[nzcnt++] = 1.;
      }
      status = CPXaddrows (env, lp, 0, 1, n, &rhs, &sense,
                           &rmatbeg, rmatind, rmatval, NULL, NULL);
      if ( status ) {
         fprintf (stderr, "Error in CPXaddrows, status = %d.\n", status);
         goto TERMINATE;
      }
   }
*/
   // Add weapon_capacity_constraint
   for(int i = 0; i < weapon_num; i++){
      int nnz = radar_num;
      double rhs = subproblem->grm->weapon_capacity;
      int* nz_index = new int[nnz];
      double* nz_val = new double[nnz];
      for(int z = 0; z < radar_num; z++){
         nz_index[z] = subproblem->cal_ssl_index(z, i);
         nz_val[z] = 1;
      }
      status = CPXaddrows (env, lp, 0, 1, nnz, &rhs, &sense,
                            &rmatbeg, nz_index, nz_val, NULL, NULL);
      delete[] nz_index;
      delete[] nz_val;
   }
   // Add radar_capacity_constraint
   for(int i = 0; i < radar_num; i++){
      int nnz = weapon_num;
      double rhs = subproblem->grm->radar_capacity;
      int* nz_index = new int[nnz];
      double* nz_val = new double[nnz];
      for(int z = 0; z < weapon_num; z++){
         nz_index[z] = subproblem->cal_ssl_index(i, z);
         nz_val[z] = 1;
      }
      status = CPXaddrows (env, lp, 0, 1, nnz, &rhs, &sense,
                            &rmatbeg, nz_index, nz_val, NULL, NULL);
      delete[] nz_index;
      delete[] nz_val;
   }

   // Add Init_cut
   for(int i = 0; i < Init_cut_num; i++){

      vector<int> weapon_set(1, 0);
      weapon_set = Init_weapon_sets[i];

      if(DEBUG){
         printf("Now add Init cut %d \n", i);
         printf("Weapon set size: %d \n", weapon_set.size());
         printf("weapon set is : ");
      
         for(int j = 0; j < weapon_set.size(); j++){
            printf("%d ", weapon_set[j]);
         }
         printf("\n");
      }
      // 2023 0223 1342 debug to note
      subproblem->cal_constraint_by_x(weapon_set);
      rhs = subproblem->rhs;
   
       status = CPXaddrows (env, lp, 0, 1, ssl_num + 1, &rhs, &sense,
                            &rmatbeg, subproblem->cutind, subproblem->cutval, NULL, NULL);
      // if ( status ) {
      //    fprintf (stderr, "Error in CPXaddrows, status = %d.\n", status);
      //    goto TERMINATE;
      // }
   }

   
   
   // Add Init Cut in this step
   

TERMINATE:

   free_and_null ((char **) &colname);
   free_and_null ((char **) &rmatind);
   free_and_null ((char **) &rmatval);
   
   return status;

} /* END create_master_ILP */


/* This routine read an array of doubles from an input file  */


//------------------------------------------------------------------------
static int
read_file (FILE *in, SubProblem& sub_prob)
{
   int  status = 0;
   char ch;
      
   int read = fscanf (in, "%c", &ch);
   
#if 0
   int  max, num;

   num = 0;
   max = 10;

   for (;;) {
      int read = fscanf (in, "%c", &ch);
      if ( read == 0 ) {
         status = -1;
         goto TERMINATE;
      }
      if ( ch == '\t' ||
           ch == '\r' ||
           ch == ' '  ||
           ch == '\n'   ) continue;
      if ( ch == '[' ) break;
      status = -1;
      goto TERMINATE;
   }

   for(;;) {
      int read = fscanf (in, "%lg", (*data_p)+num);
      if ( read == 0 ) {
         status = -1;
         goto TERMINATE;
      }
      num++;
      if ( num >= max ) {
         max *= 2;
         *data_p = (double*)realloc(*data_p, max * sizeof(double));
         if ( *data_p == NULL ) {
            status = CPXERR_NO_MEMORY;
            goto TERMINATE;
         }
      }
      do {
         read = fscanf (in, "%c", &ch);
         if ( read == 0 ) {
            status = -1;
            goto TERMINATE;
         }
      } while (ch == ' ' || ch == '\n' || ch == '\t'  || ch == '\r');
      if ( ch == ']' ) break;
      else if ( ch != ',' ) {
         status = -1;
         goto TERMINATE;
      }
   }

   *num_p = num;
#endif
TERMINATE:

   return status;

} /* END read_file */


/* This routine frees up the pointer *ptr, and sets *ptr to 
   NULL */

static void
free_and_null (char **ptr)
{
   if ( *ptr != NULL ) {
      free (*ptr);
      *ptr = NULL;
   }
} /* END free_and_null */ 


void SubProblem::find_init_cut(vector<vector<int>>& cut_point, int cut_num = 1, int wt_ratio = 1){
   int DEBUG_PRINT_START = 0;
   if(DEBUG_PRINT_START){
      printf("Now find the Init_cut of subproblem\n");
   }

   cut_point = vector<vector<int>>(cut_num, vector<int>(ssl_num, 0));

   int DEBUG = 0;
   // Use "weapon value" to calculte witch weapon should be added to Init_cut
   vector<double> ssl_value(ssl_num, 0);

   // use 1 - p_ij and u_i to choose the best SSL to this target, 
   // those two variable may need scaling
   bool is_scaling = 0;
   if(!is_scaling){
      for(int i = 0; i < ssl_num; i++){
         int temp_weapon_index = cal_weapon_index(i);
         int temp_radar_index = cal_radar_index(i);
         ssl_value[i] = 1 - target_value * j_prob_vector[i] - weapon_dual_ui[temp_weapon_index] - radar_dual_muk[temp_radar_index];

         if(DEBUG){
            printf("prob : %.2f, dual : %.2f, t_value : %.2f\n", j_prob_vector[i], weapon_dual_ui[i], target_value);
            printf("value : %.2f\n", ssl_value[i]);
         }
      }
   }
   // @todo Add scaling
   else{
   }

   for(int i = 0; i < cut_num; i++){
      int weapon_id = 0;
      // only find one SSL for the target with 4 missiles with lowest weapon value
      int min_index = 0;
      double min_value = ssl_value[0];

      for(int j = 0; j < ssl_num; j++){
          double temp_value = ssl_value[j];
          if(DEBUG){
            printf("new value : %.2f, old value : %.2f\n", temp_value, min_value);
          }

           if(temp_value + WTA_EPS < min_value){
                min_index = j;
                min_value = temp_value;
           }
      }
      cut_point[i][min_index] = 1;
   }


   if(DEBUG){
      printf("Now show all the weapon scenes to generalize the Init cut \n");
      printf("Scene Num: %d\n", cut_point.size());
      for(int i = 0; i < cut_point.size(); i++){
         printf("Scene size: %d \n", cut_point[i].size());
         printf("Scene %d : ", i);
         for(int j = 0; j < cut_point[i].size(); j++){
            printf("%d ", cut_point[i][j]);
         }
         printf("\n");
      }
      printf("print finished \n");
   }
   return;
}

static void
usage (char *progname)
{
   fprintf (stderr,
      "Usage:     %s {0|1} [form] [sparsity] [m] [n] [seed] \n", progname);
   fprintf (stderr,
      " 0:        Benders' cuts only used as lazy constraints,\n");
   fprintf (stderr,
      "           to separate integer infeasible solutions.\n");
   fprintf (stderr,
      " 1:        Benders' cuts also used as user cuts,\n");
   fprintf (stderr,
      "           to separate fractional infeasible solutions.\n");
   fprintf (stderr, 
      " form:     0 for exponential form, 1 for polynominal form.\n");
   fprintf (stderr,
      " m:        number of weapens.\n");
   fprintf (stderr, 
      " n:        number of targets.\n");
   fprintf (stderr, 
      " sparsity: the rate of zeros in the probobility matrix.\n");
   fprintf (stderr, 
      " seed:     random seed.\n");
} /* END usage */

bool SubProblem::print_solution()
   {
      int integer_num = 0;
      int x_num = grm->ssl_num;
      //printf("m = %d \n", m);
      //printf("the frac variable on target %d is :", j);
      for (int i = 0; i < x_num; i++){
         double i_value = x[i];
         int rounded = int(round(i_value));
         double diff = std::abs(i_value - static_cast<double>(rounded));
         if (diff < WTA_EPS)
         {
            integer_num++;
         }
         //printf("%4f ", i_value);
      }
      //printf("\n the number of integer variable is %d \n", integer_num);
      return (integer_num == x_num);
   }

void SubProblem::__enumerate_all_scene(vector<vector<int>>& All_num, int col_num, int radar_capacity, int weapon_start, vector<int> weapon_capacity, int& counter_scene, int row_num, int current_radar, double& opt_val, vector<int>& opt_sol)  {
    if(current_radar == (row_num-1) && weapon_start == (col_num-1)){
      if(radar_capacity > weapon_capacity[weapon_start]){
         return;
      }
      All_num[current_radar][col_num - 1] = radar_capacity;
      /*
      printf("Scene %d : \n", counter_scene);
      for(int j = 0; j < row_num; j++){
         printf("R%d : ", j);
         for(int i = 0; i < col_num; i++){
            printf("%d ", All_num[j][i]);
         }
         printf("\n");
      }
      */

     /*
     */
     vector<int> temp_ssl(col_num * row_num);
     for(int i = 0; i < row_num; i++){
         for(int j = 0; j < col_num; j++){
            temp_ssl[i * col_num + j] = All_num[i][j];
         }
     }

     double temp_val = __cal_fx_from_ssl_set(temp_ssl);

     if(temp_val < opt_val - 1e-10){
      opt_val = temp_val;
      for(int i = 0; i < temp_ssl.size(); i++){
         opt_sol[i] = temp_ssl[i];
      }
     }
     int DEBUG_PRINT = 0;
     if(DEBUG_PRINT)
     { 
      printf("SSL %d : ", counter_scene);
      printf(" with val %.4f", temp_val);
      printf("\n");
     }
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
      __enumerate_all_scene(All_num, col_num, 8, 0, new_weapon_capacity, counter_scene, row_num,current_radar + 1, opt_val, opt_sol);
      return;
    }

    if(radar_capacity == 0){
      for(int i = weapon_start; i < col_num; i++){
         All_num[current_radar][i] = 0;
         __enumerate_all_scene(All_num, col_num, 0, col_num -1, weapon_capacity, counter_scene, row_num, current_radar, opt_val, opt_sol);
         return;
      }
    }

    int temp_capacity = (radar_capacity < weapon_capacity[weapon_start]) ? radar_capacity : weapon_capacity[weapon_start];

    //printf("temp_capacity : %d\n", temp_capacity);

    for(int j = 0; j <= temp_capacity; j++){
      All_num[current_radar][weapon_start] = j;
      __enumerate_all_scene(All_num, col_num, radar_capacity-j, weapon_start + 1, weapon_capacity, counter_scene, row_num, current_radar, opt_val, opt_sol);
    }
}

    void SubProblem::cal_optimal_scene_by_enum(vector<int>& opt_sols, double& opt_val){

      printf("--------------- Start Enumerate ------------------\n");
      int temp_col_num = grm->weapon_num_m;
      int temp_row_num = grm->radar_num_k;

      int temp_radar_capa = grm->radar_capacity;

      vector<vector<int>> test_enum(temp_row_num, vector<int>(temp_col_num, 0));
      vector<int> w_capacity(temp_col_num, grm->weapon_capacity);

      int counter = 0;
      opt_val = 100000000;
      vector<int> opt_sol(temp_col_num * temp_row_num, 0);

      __enumerate_all_scene(test_enum, temp_col_num, temp_radar_capa, 0, w_capacity, counter, temp_row_num, 0, opt_val,  opt_sol);
      printf("All : %d\n", counter);
      printf("opt val : %.2f\n", opt_val);
      printf("opt sol : ");
      for(int i = 0 ; i < temp_col_num * temp_row_num; i++){
         printf("%d ", opt_sol[i]);
      }
      printf("\n");
      printf("--------------- Enumerate end------------------\n");
    }