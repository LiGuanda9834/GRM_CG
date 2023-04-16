#ifndef _ALGO_PARAMETER_H__
#define _ALGO_PARAMETER_H__

#include <string>
#include <random>
#include <iostream>
#include <limits>

using std::string;

class AlgoParameter
{
public:
// path and name
    std::string problem_name = "GRM";
    std::string algo_name = "Branch and Cut"; 
    std::string test_prefix = "Instance";
    std::string test_extension = ".csv";
    std::string path_data = "";
    std::string path_result_sol= "";
    std::string path_result_csv= "";

// constant number
    double eps = 1.0e-8;
    int time_limit = 7200;
    long maxValue = std::numeric_limits<long>::max();

// Random generation
    int seed = 0;
    std::default_random_engine rnd;

// GRM params
    // model module
    bool objIncludeTime = 1;    // 1 for doesn't consider time, 0 for consider time
    bool useTargetCons = 0;     // 1 for each target has at most 1 SSL 

    // mp module
    int mpInitScene_mode = 1;   // 1 for target i with [radar(i/r_capa), weapon(i/w_capa)]

    // pricing module 
    bool pricingCheckCorrectness = 0;

    // sp module 
    int spInitCut_mode = 0;     // Need consider
    bool spOptOnly = 0;         // 1 for only add opt sp solution
    bool spCutAllowFrac = 0;    

// Branch and Cut param
    bool rootOnly = 1;
    bool enableCuts = 1;
    bool depthFirst = 1;
    bool enableBranchOnSum = 0;
    double optimalityGap = 1.0e-6;


// Debug and Infomation params
    int DEBUG_VERSION = 0;
    bool enableCplexLog = 0;



public:
   AlgoParameter(){}
    AlgoParameter(std::string _prob, std::string _algo, int _tlim, std::string _prefix, std::string _ext,
                    std::string _data,
                    bool _rootOnly, bool _enableCuts, bool _depthFirst,
                    bool _enableBranchOnSum = false, bool _enableCplexLog = false, bool _DEBUG_VERSION = 0)
          : seed(3), eps(1.0e-6),
            problem_name(_prob), algo_name(_algo), time_limit(_tlim),
            test_prefix(_prefix), test_extension(_ext), path_data(_data),
            rootOnly(_rootOnly), enableCuts(_enableCuts), depthFirst(_depthFirst),
            enableBranchOnSum(_enableBranchOnSum),
            enableCplexLog(_enableCplexLog), DEBUG_VERSION(_DEBUG_VERSION)
      {
         std::srand(seed); // ensure the same random sequence for each run
         rnd = std::default_random_engine(seed);
      }

      
      ~AlgoParameter() = default;

      std::string csv_name()
      {
         return "result_" + algo_name + ".csv";
      }
};


#endif