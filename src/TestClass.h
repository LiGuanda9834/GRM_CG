#ifndef TEST_CLASS_H
#define TEST_CLASS_H

#include <iostream>
#include "BranchAndCut.h"
#include "WTA.h"
#include "print.h"
#include "Pricing.h"
#include "Scene.h"
#include "SubProblem.h"
#include "AlgorithmParameter.h"

#include <string>
#include <ctime>
#include <vector>


class TestClass{
public:
// Constructor
    TestClass(GRM* grm_, AlgoParameter& param_);



    void test_TestClass(){
        printf("Now Test Test Class \n");
    }

// 


// Test Scene and ScenePool
    void test_scene_addSceneToPool();


// Other 
    // Enumerate all the ssl
    void getCombinations(vector<vector<int>>& All_num, int col_num, int radar_capacity, int weapon_start, vector<int> weapon_capacity, int& counter_scene, int row_num, int current_radar, double& opt_val, vector<int>& opt_sol );

    void test_Enumerate(int radar_capacity, int weapon_capacity);



public:
    GRM* grm;
    Master* master;
    Pricing* pricing;
    SubProblem* sub;
    AlgoParameter param;
};

#endif