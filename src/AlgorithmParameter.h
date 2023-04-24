#ifndef _ALGO_PARAMETER_H__
#define _ALGO_PARAMETER_H__

#include <string>
#include <random>
#include <iostream>
#include <limits>
#include <type_traits>
#include <iomanip>

using std::string;
using std::cout;
using std::endl;

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

// param print
    bool printRandomParam = 0;
    bool printDataSource = 0;
    bool printModelParam = 0;
    bool printBranchAndCutParam = 1;
    bool printMpParam = 0;
    bool printPricingParam = 0;
    bool PrintSpParam = 1;


// Random generation
    int seed = 0;
    std::default_random_engine rnd;

// Data Source
    int dataSource = 1;         // 1 for randomly generated, 0 for file reading. 

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
    int spPrintMode = 0; 

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
     
     void show_sp_print_mode(){
        /*
        [...|3|2|1]
        i use to control if show OA cut when add a cut
        */ 
        int current_param_num = 1;
        vector<bool> print_args(current_param_num, false);
        for(int i = 0; i < current_param_num; i++){
            print_args[i] = (spPrintMode & (1 << i)) != 0;
        }
        if(print_args[0]){
            printf("show OA cut in the sp OA process\n");
        }
     }
     
     void set_sp_print_mode(vector<bool> sp_print_args){
        for(int i = 0; i < sp_print_args.size(); i++){
            spPrintMode |= (1 << i);
        }
     }

    void printAllParams(){
        printf("========= Now print All the algorithm parameters  ==========\n");
        if(printDataSource){
            printf("=== Now print the Data Information ===\n");
            printOneParamName("Data source");
            if(dataSource == 1){
                printf("randomly generating\n");
            }
            else{
                printf("reading files from BeiLiGong\n");
            }
        }
        if(printModelParam){
            printf("=== Now print the Model parameters ===\n");
            printOneParam("obj consider time?", objIncludeTime);
            printOneParam("limit targetSSL?", useTargetCons);
        }
        if(printMpParam){
            printf("=== Now print the MP parameters ===\n");
            printOneParamName("mp Init mode");
            if(mpInitScene_mode == 1){
                printf("target i with [radar(i/r_capa), weapon(i/w_capa)]\n");
            }
        }
        if(PrintSpParam){
            printf("=== Now print the SP parameters ===\n");
            printOneParam("only add opt sp", spOptOnly);
            printOneParam("OA allow frac cut", spCutAllowFrac);
        }
        if(printBranchAndCutParam){
            printf("=== Now print the Branch and Bound parameters ===\n");
            printOneParam("only cal root Node", rootOnly);
        }
        printf("========= All the algorithm parameters print finish ========\n");
    }

    template<typename T>
    void printOneParam(const string& param_name_, const T& param_value_) {
        int output_length = 20;
        string truncated_name = param_name_.substr(0, output_length);
        string formatted_name = truncated_name + string(output_length - truncated_name.size(), ' ');
        cout << formatted_name << ": ";

        if constexpr (std::is_integral<T>::value){
            cout << param_value_ << endl;
        }
        else if constexpr(std::is_floating_point<T>::value){
            cout << std::fixed << std::setprecision(3) << param_value_ << endl;
        }
        else if constexpr(std::is_same<T, bool>::value){
            cout << (param_value_ ? "true" : "false") << endl;
        }
        else if constexpr(std::is_same<T, string>::value){
            cout << param_value_ << endl;
        }
        return;
    }

    void printOneParamName(const string& param_name_){
        int output_length = 15;
        string truncated_name = param_name_.substr(0, output_length);
        string formatted_name = truncated_name + string(output_length - truncated_name.size(), ' ');
        cout << formatted_name << ": ";
        return;
    }
};


#endif