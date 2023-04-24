#include "WTA.h"

/**
 * @brief Generate a instance by random
 * @details P[i][j] in [0.6, 0.9], Value of targets W in [25, 100]
*/
WTA::WTA(int n_, int m_, int seed)
{
    printf("Create WTA\n");
    target_num_n = n_;
    weapon_num_m = m_;
    target_value = new double[target_num_n];
    //P[i][j]   i : weapon, j : target
    probability_matrix = new double*[weapon_num_m]; 
    srand(seed);
    for(int i = 0; i < target_num_n; i++)
    {
    target_value[i] = rand()%75 + 25;
    }
    DEBUG_MODE = 0;
    
    
    for(int i = 0; i < weapon_num_m; i++)
    {  
        probability_matrix[i] = new double[target_num_n];
        for(int j = 0; j < target_num_n; j++){
            probability_matrix[i][j] = double(rand())/double(RAND_MAX) * 0.3 + 0.6;
            if(DEBUG_MODE == 5){
                printf("%d -> %d : %.4f \n", i, j, probability_matrix[i][j]);
            }
        }
    }
}

// n for target num, m for weapon num
WTA::WTA(int n_, int m_){
    printf("Create empty WTA with size W: %d, T:%d \n Need Initialized\n", m_, n_);
    target_num_n = n_;
    weapon_num_m = m_;
    target_value = new double[target_num_n];
    //P[i][j]   i : weapon, j : target
    probability_matrix = new double*[weapon_num_m]; 
    for(int i = 0; i < target_num_n; i++)
    {
        target_value[i] = 0;
    }
    for(int i = 0; i < weapon_num_m; i++)
    {  
        probability_matrix[i] = new double[target_num_n];
        for(int j = 0; j < target_num_n; j++){
            probability_matrix[i][j] = 0;
        }
    }
    DEBUG_MODE = 0;
}


void WTA::Init(int n_, int m_, int seed = 0)
{
    printf("Initialize by random, seed = %d\n", seed);
    target_num_n = n_;
    weapon_num_m = m_;
    target_value = new double[target_num_n];
    //P[i][j]   i : weapon, j : target
    probability_matrix = new double*[weapon_num_m]; 
    srand(seed);
    for(int i = 0; i < target_num_n; i++)
    {
        target_value[i] = rand()%75 + 25;
    }
    for(int i = 0; i < weapon_num_m; i++)
    {  
        probability_matrix[i] = new double[target_num_n];
        for(int j = 0; j < target_num_n; j++){
            probability_matrix[i][j] = double(rand())/double(RAND_MAX) * 0.3 + 0.6;
            //printf("%d -> %d : %.4f \n", i, j, probability_matrix[i][j]);
        }
    }
}

void WTA::Init_sparse(int n_, int m_, int sparsity_prob_, int seed = 0)
{
    target_num_n = n_;
    weapon_num_m = m_;

    int sparsity_prob = sparsity_prob_;

    int nZero_counter = 0;

    for(int i = 0; i < target_num_n; i++)
    {
        target_value[i] = rand()%75 + 25;
    }
    for(int i = 0; i < weapon_num_m; i++)
    {  
        for(int j = 0; j < target_num_n; j++){
            if (rand()%100 > sparsity_prob){
                probability_matrix[i][j] = double(rand())/double(RAND_MAX) * 0.3 + 0.6;
                nZero_counter++;
            } 
            else{
                probability_matrix[i][j] = 0;
            }
        }
    }
    int probMatrix_size = weapon_num_m * target_num_n;
    double real_sparsity = 0;
    real_sparsity = (double)(probMatrix_size - nZero_counter) / (double)probMatrix_size;
}


void WTA::Init_readfile(FILE* fin, int seed_)
{
    printf("Initialzed by read file \n");
    seed = seed_;
    int status = 0;
    int f = fscanf (fin, "%d", &target_num_n);
    printf("num : %d\n", weapon_num_m);
    if(!target_value){
        printf("new Again !!!!!!!!!!!!!!!!\n");
        target_value = new double[target_num_n];
    }
    for(int i = 0; i < target_num_n; i++)
    {
        fscanf (fin, "%lf", &target_value[i]);
        printf("%d:%.0f\n", i, target_value[i]);
    }
    for(int i = 0; i < target_num_n; i++)
    {
        if(!probability_matrix[i]){
            probability_matrix[i] = new double[target_num_n];
        }
        for(int j = 0; j < weapon_num_m; j++)
        {
            fscanf(fin, "%lf", &(probability_matrix[i][j]));
            //printf("%.2f\n", p[i][j]);
        }
    }
}

void WTA::Delete(){
    if(this){
        if(target_value){
            delete[] target_value;
        }
        if(probability_matrix){
            for(int t = 0; t < weapon_num_m; t++){
                if(probability_matrix[t]){
                    delete[] probability_matrix[t];
                }
            }
            delete[] probability_matrix;
        }
    }
}

void WTA::print_model(){
    printf("->\t");
    for(int i = 0; i < weapon_num_m; i++){
        printf("W%d \t", i);
    }
    printf("value \n");
    for(int j = 0; j < target_num_n; j++){
        printf("T%d\t", j);
        for(int i = 0; i < weapon_num_m; i++){
            printf("%.2f \t", probability_matrix[i][j]);
        }
        printf("%.2f \n", target_value[j]);
    }
}



double WTA::cal_target_value(Scene temp_scene){
    int t_index = temp_scene.target_index;
    double return_value = target_value[t_index];
    int activated_weapon_num = temp_scene.weapon_indices.size();
    for(int i = 0; i < activated_weapon_num; i++){
        return_value = return_value * (1 - probability_matrix[temp_scene.weapon_indices[i]][t_index]);
    }
    return return_value;
}

double WTA::cal_linear_coef(int _j, vector<int> _scene){
    double q_js = target_value[_j];
    int activated_weapon_num = _scene.size();
    for(int i = 0; i < activated_weapon_num; i++){
        int _weapon = _scene[i];
        double q_js_temp = q_js;
        q_js = q_js * (1 - probability_matrix[_weapon][_j]);
        int DEBUG_qjs = 0;
        if(DEBUG_qjs){
            printf("%s : %.2f * %.2f -> %.2f \n", "q_js", q_js_temp, 1 - probability_matrix[_weapon][_j], q_js);
        }
    }
    return q_js;
}


bool WTA::is_weapon_in_scene(int _i, vector<int> _scene){
    if (std::find(_scene.begin(), _scene.end(), _i) != _scene.end()){
        return 1;
    }
    else{
        return 0;
    }
}
double WTA::set_scene_qjs(Scene& scene){
    double q_js = cal_linear_coef(scene.target_index, scene.weapon_indices);
    scene.obj_qjs = q_js;
    return q_js;
}

GRM::GRM(){
    weapon_num_m = 0;
    target_num_n = 0;
    radar_num_k = 0;
    ssl_num = 0;

    probability_matrix = NULL;
    time_matrix = NULL;
    target_value = NULL;
    DEBUG_MODE = 0;
    seed = 0;
}

GRM::GRM(int n_, int m_, int k_, int seed_){
    printf("Generate empty GRM with size n:%d, m:%d, l:%d\n", n_, m_, k_);
    printf("Set weapon capacity to 4, radar capacity to 8. May Need change\n");

    target_num_n = n_;
    weapon_num_m = m_;
    radar_num_k = k_;
    ssl_num = m_ * k_;

    radar_capacity = 8;
    weapon_capacity = 4;

    ssl_capacity = cal_ssl_capacity();

    target_value = new double[target_num_n];
    for(int i = 0; i < target_num_n; i++)
    {
        target_value[i] = 0;
    }

    // Init Probability Matrix, 
    /*
        p[i][j]   i : ssl, j : target
        random form 0.3 - 0.9, double
    */
    probability_matrix = new double*[ssl_num]; 
    for(int i = 0; i < ssl_num; i++)
    { 
        probability_matrix[i] = new double[target_num_n];
        for(int j = 0; j < target_num_n; j++){
            probability_matrix[i][j] = 0;
        }
    }
    // Init Probability Matrix, 
    /*
        t[i][j]   i : ssl, j : target
        random form 0.1 - 1.0, double
    */
    time_matrix = new double*[ssl_num]; 
    for(int i = 0; i < ssl_num; i++)
    {
        time_matrix[i] = new double[target_num_n];
        for(int j = 0; j < target_num_n; j++){
            time_matrix[i][j] = 0;
        }
    }
    DEBUG_MODE = 0;
    seed = seed_;
}

GRM::GRM(int instance_id){
    printf("Initialzed GRM by read file \n");
    string Path = "/share/home/liguanda/WTA-Problem/Code/GRM_CG/data/CSV/";
    string Name = "instance" + std::to_string(instance_id) + ".csv";
    string file_name_str = Path + Name;
    char* test_file_char = (char*)file_name_str.c_str();
    std::cout << test_file_char << std::endl;
    FILE* fin = NULL;
    fin = fopen(test_file_char, "r");
    if(fin == NULL){
        printf("Open file Failed! Check the file name\n");
    }
    else{
        fscanf (fin, "%d,%d,%d,", &target_num_n, &weapon_num_m, &radar_num_k);
        ssl_num = weapon_num_m * radar_num_k;

        fscanf (fin, "%d, %d,", &weapon_capacity, &radar_capacity);

        target_value = new double[target_num_n];
        for(int i = 0; i < target_num_n; i++){
            fscanf (fin, "%lf,", &target_value[i]);
            printf("%d value : %.2f\n",i , target_value[i]);
        }

        probability_matrix = new double*[ssl_num]; 
        for(int i = 0; i < ssl_num; i++)
        { 
            probability_matrix[i] = new double[target_num_n];
            for(int j = 0; j < target_num_n; j++){
                probability_matrix[i][j] = 0;
            }
        }

        for(int i = 0; i < target_num_n; i++)
        { 
            for(int j = 0; j < ssl_num; j++){
                fscanf (fin, "%lf,", &probability_matrix[j][i]);
                printf("%lf ",probability_matrix[j][i]);
            }
            printf("\n");
        }

        
        time_matrix = new double*[ssl_num]; 
        for(int i = 0; i < ssl_num; i++)
        {
            time_matrix[i] = new double[target_num_n];
            for(int j = 0; j < target_num_n; j++){
                time_matrix[i][j] = 0;
            }
        }

        DEBUG_MODE = 0;
        seed = 0;
    }
}
    /*
    int status = 0;
    int f = fscanf (fin, "%d", &target_num_n);
    printf("num : %d\n", weapon_num_m);
    if(!target_value){
        printf("new Again !!!!!!!!!!!!!!!!\n");
        target_value = new double[target_num_n];
    }
    for(int i = 0; i < target_num_n; i++)
    {
        fscanf (fin, "%lf", &target_value[i]);
        printf("%d:%.0f\n", i, target_value[i]);
    }
    for(int i = 0; i < target_num_n; i++)
    {
        if(!probability_matrix[i]){
            probability_matrix[i] = new double[target_num_n];
        }
        for(int j = 0; j < weapon_num_m; j++)
        {
            fscanf(fin, "%lf", &(probability_matrix[i][j]));
            //printf("%.2f\n", p[i][j]);
        }
    }
    */





int GRM::Init_by_random(int seed){
    printf("Size n:%d, m:%d, l:%d\n", target_num_n, weapon_num_m, radar_num_k);
    int return_value = 1;
    printf("Initialize by random, seed = %d\n", seed);
    srand(seed);
    // Init Target value, 
    /*
        random from 25 - 100, integer
    */
    for(int i = 0; i < target_num_n; i++)
    {
        target_value[i] = rand()%75 + 25;
    }

    // Init Probability Matrix, 
    /*
        P[i][j]   i : ssl, j : target
        random form 0.3 - 0.9, double
    */
    for(int i = 0; i < ssl_num; i++)
    { 
        for(int j = 0; j < target_num_n; j++){
            probability_matrix[i][j] = double(rand())/double(RAND_MAX) * 0.3 + 0.6;
            time_matrix[i][j] = double(rand())/double(RAND_MAX)*0.9 + 0.1;
            //printf("%d -> %d : %.4f \n", i, j, probability_matrix[i][j]);
        }
    }
    return return_value;
}




int GRM::print_model(){
    int return_value = 1;
    printf("p[s][j] and V[j\n");
    printf("(w,r))\t");
    for(int i = 0; i < ssl_num; i++){
        printf("(%d,%d) \t", cal_weapon_index(i), cal_radar_index(i));
    }
    printf("value \n");
    for(int j = 0; j < target_num_n; j++){
        printf("T%d\t", j);
        for(int i = 0; i < ssl_num; i++){
            printf("%.2f \t", probability_matrix[i][j]);
        }
        printf("%.2f \n", target_value[j]);
    }
    
    
    printf("t[s][j]\n");
    printf("(w,r))\t");
    for(int i = 0; i < ssl_num; i++){
        printf("(%d,%d) \t", cal_weapon_index(i), cal_radar_index(i));
    }
    printf("\n");
    for(int j = 0; j < target_num_n; j++){
        printf("T%d\t", j);
        for(int i = 0; i < ssl_num; i++){
            printf("%.2f \t", time_matrix[i][j]);
        }
        printf("\n");
    }
    return return_value;
}

GRM::~GRM(){
    if(this){
        if(target_value){
            delete[] target_value;
        }
        if(probability_matrix){
            for(int t = 0; t < ssl_num; t++){
                if(probability_matrix[t]){
                    delete[] probability_matrix[t];
                }
            }
            delete[] probability_matrix;
        }
        if(time_matrix){
            for(int t = 0; t < ssl_num; t++){
                if(time_matrix[t]){
                    delete[] time_matrix[t];
                }
            }
            delete[] time_matrix;
        }
    }
}



double GRM::cal_linear_coef(int _j, int nnz, vector<int> nz_index, vector<int> nz_value){
    double q_js = target_value[_j];
    for(int i = 0; i < nnz; i++){
        int temp_ssl_index = nz_index[i];
        int temp_ssl_num = nz_value[i];
        double q_js_temp = q_js;
        q_js = q_js * pow( (1 - probability_matrix[temp_ssl_index][_j]), temp_ssl_num);
        int DEBUG_qjs = 0;
        if(DEBUG_qjs){
            printf("%s : %.2f * (1-%.2f)^%d -> %.2f \n", "q_js", q_js_temp,  probability_matrix[temp_ssl_index][_j], temp_ssl_num, q_js);
        }
    }
    return q_js;
}

double GRM::cal_tjs_coef(int _j, int nnz, vector<int> nz_index, vector<int> nz_value){
    double t_js = 0;
    if(nnz == 0){
        return t_js;
    }

    for(int i = 0; i < nnz; i++){
        int temp_ssl_index = nz_index[i];
        if(time_matrix[temp_ssl_index][_j] > t_js)
        {
            t_js = time_matrix[temp_ssl_index][_j];
        }
    }

    return t_js;
}


double GRM::set_ssl_qjs(Scene_SSL& scene_ssl){
    //int target_index_j = scene_ssl.target_index;
    //int nnz = scene_ssl.nz_weapon_radar;
    //printf("Set SSL with target : %d, index : %d\n", target_index_j, nnz);
    double q_js = cal_linear_coef(scene_ssl.target_index, scene_ssl.nz_weapon_radar, scene_ssl.weapon_radar_indices, scene_ssl.weapon_radar_num);
    scene_ssl.obj_qjs = q_js;
    return q_js;
}

double GRM::set_ssl_tjs(Scene_SSL& scene_ssl){
    double t_js = cal_tjs_coef(scene_ssl.target_index, scene_ssl.nz_weapon_radar, scene_ssl.weapon_radar_indices, scene_ssl.weapon_radar_num);
    scene_ssl.time_tjs = t_js;
    return t_js;
}