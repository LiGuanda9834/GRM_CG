#ifndef _WTA_H_
#define _WTA_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <time.h>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <cmath>
#include "Scene.h"

using std::vector;
using std::string;
using std::unordered_map;

#define EPS     1e-6
#define BUFSIZE 1000

class WTA
{
public:
    int         weapon_num_m;
    int         target_num_n;
    // P = p[weapon][target]
    double**    probability_matrix;
    
    // V = V[t]
    double*     target_value;
    int         DEBUG_MODE; //

    // Random seed
    int         seed;
public:
    WTA() = default;
    WTA(int n_, int m_, int seed);
    WTA(int n_, int m_);
    ~WTA() = default;


    void Delete();

    // Initialize the WTA by Random Generator or ReadFile
    void Init(int n_, int m_, int seed);
    void Init_sparse(int n_, int m_, int sparsity, int seed);
    void Init_readfile(FILE* fin, int seed_);



    // calculate the x_is, q_js from a vector like [1,0,2,0,0,1,0]
    double cal_linear_coef(int _j, vector<int> _ssl);
    bool is_weapon_in_scene(int _i, vector<int> _ssl);
    
    
    // calculate the q_js from the scene, nnz = 3, nz_index = [0,2,5], nz_value = [1,2,1] 
    double cal_target_value(Scene temp_scene);
    
    
    double set_scene_qjs(Scene& scene);

    // print the prob matrix and value of the WTA
    void print_model();
};


class GRM
{
public:
    int         weapon_num_m;
    int         target_num_n;
    int         radar_num_k; //

    int         ssl_num; //

    int         radar_capacity;
    int         weapon_capacity;

    int         ssl_capacity;       // capatity of one ssl, = min(radar_capacity, weapon_capacity)

    // V = V[t]
    double*     target_value;

    // P = p[ssl][target], t = t[ssl][target]
    double**    probability_matrix;
    double**    time_matrix;
    
    // Other Parameters
    /*
        seed for random seed
        DEBUGE_MODE for how much information has been output
    */
    int         seed;
    int         DEBUG_MODE; //


public:
    GRM();
    
    GRM(int n_, int m_, int k_, int seed_);    // target, weapon, radar
    //GRM(int n_, int m_);
    GRM(FILE* fin);
    ~GRM();




    // Initialize 
    int Init_by_random(int seed);

    //void Init_sparse(int n_, int m_, int sparsity, int seed);
    //void Init_readfile(FILE* fin, int seed_);


    // print the prob matrix and value of the WTA
    int print_model();


    // calculate the q_js from a compress storage
    double cal_linear_coef(int _j, int nnz, vector<int> nz_index, vector<int> nz_value);

    double cal_tjs_coef(int _j, int nnz, vector<int> nz_index, vector<int> nz_value);

    // calculate the q_js from the scene, j = 2, nnz = 3, nz_index = [0,2,5], nz_value = [1,2,1] 
    double set_ssl_qjs(Scene_SSL& scene);
    double set_ssl_tjs(Scene_SSL& scene);

    inline int cal_ssl_capacity() {
        if(weapon_capacity < radar_capacity)
        {
            return weapon_capacity;
        }
        else
        {
            return radar_capacity;
        }

    }


    inline int cal_weapon_index(const int ssl_index) const
    {
        return ssl_index % weapon_num_m;
    } 

    inline int cal_radar_index(const int ssl_index) const
    {
        return ssl_index / weapon_num_m;
    } 
    
      inline int cal_ssl_index(const int radar_index, const int weapon_index) const
      {
         return weapon_num_m * radar_index + weapon_index;
      }

};

#endif