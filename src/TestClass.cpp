#include "TestClass.h"

TestClass::TestClass(GRM* grm_, AlgoParameter& param_){
    grm = grm_;
    master = NULL;
    pricing = NULL;
    sub = NULL;
    param = param_;
}


void TestClass::test_scene_addSceneToPool(){
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

    temp_ssl_pool.print_all_scene();
    temp_ssl_pool.print_scene_by_target(6);

    for(int i = 0; i < temp_ssl_pool.size(); i++){
        double temp_qjs = grm->set_ssl_qjs(temp_ssl_pool[i]);
        printf("Now calculate ssl %d \n", i);
        temp_ssl_pool[i].PrintScene();
    }
}

void TestClass::getCombinations(vector<vector<int>>& All_num, int col_num, int radar_capacity, int weapon_start, vector<int> weapon_capacity, int& counter_scene, int row_num, int current_radar, double& opt_val,vector<int>& opt_sol) {
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


void TestClass::test_Enumerate(int radar_capacity, int weapon_capacity){
    printf("------ TEST-----\n");
    printf("test enumerate all the ssl \n");
      // Test Scene
      int temp_col_num = grm->weapon_num_m;
      int temp_row_num = grm->radar_num_k;

      int temp_radar_capa = 8;
      temp_radar_capa = radar_capacity;
      int temp_weapon_capa = 4;
      temp_weapon_capa = weapon_capacity;
      vector<vector<int>> test_enum(temp_row_num, vector<int>(temp_col_num, 0));
      vector<int> w_capacity(temp_col_num, temp_weapon_capa);

      int counter = 0;
      double best = 100000000;

      vector<int> opt_sol(temp_col_num * temp_row_num, 0);
      
      
      getCombinations(test_enum, temp_col_num, temp_radar_capa, 0, w_capacity, counter, temp_row_num, 0, best, opt_sol);

      printf("All : %d\n", counter);
      printf("Best_opt_val : %.2f\n", best);
}
