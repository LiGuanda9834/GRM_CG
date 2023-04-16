#include "Scene.h"
#include <functional>

Scene::Scene(int _j, std::vector<int> _scene){
    target_index = _j;
    weapon_indices = std::vector<int>();
    for(int i = 0; i < _scene.size(); i++){
        weapon_indices.push_back(_scene[i]);
    }
}

void Scene::Set_Scene(int _j, std::vector<int> _scene){
    target_index = _j;
    weapon_indices = std::vector<int>();
    for(int i = 0; i < _scene.size(); i++){
        if(_scene[i] == 1){
            weapon_indices.push_back(i);
        }
    }
}

void Scene::PrintScene(){
    printf("Target %d : ", target_index);
    for(int i = 0; i < weapon_indices.size(); i++){
        printf("%d ", weapon_indices[i]);
    }
    printf("\n");
}

bool Scene::is_same_w_t(Scene compare_scene){
    bool is_same = 1;
    if(compare_scene.target_index != target_index){
        is_same = 0;
        return is_same;
    }
    if(compare_scene.weapon_indices.size() != weapon_indices.size()){
        is_same = 0;
        return is_same;  
    }
    for(int i = 0; i < weapon_indices.size(); i++){
        if(compare_scene.weapon_indices[i] != weapon_indices[i]){
            is_same = 0;
            return is_same;
        }
    }
    return is_same;
}

void ScenePool::print_all_scene(){
    printf("There are \t %d Scenes in the Scene Pool. \n", Scene_num);

    for_each(this->begin(), this->end(), std::mem_fun_ref(&Scene::PrintScene));
}

void ScenePool::add_scene(Scene scene){
      Scene_num++;
      push_back(scene);
}

void ScenePool::print_scene_by_target(int target_num){
    int scene_num = size();
    vector<Scene>* target_scenes = new vector<Scene>[target_num];
    for(int i = 0; i < scene_num; i++){
        Scene temp_scene = at(i);
        target_scenes[temp_scene.target_index].push_back(temp_scene);
    }
    for(int t = 0; t < target_num; t++){
        printf("Target : %d, Size : %d\n", t, target_scenes[t].size() );
        for(int s = 0; s < target_scenes[t].size(); s++){
            target_scenes[t][s].PrintScene();
        }
    }
    if(target_scenes){
        delete[] target_scenes;
        target_scenes = NULL;
    }
}

Scene_SSL::Scene_SSL(int _j, std::vector<int> _scene, int _m, int _k){
    index = -1;
    obj_qjs = 0.0;
    time_tjs = 0.0;
    redcost = 0.0;

    nz_weapon_radar = 0;
    target_index = _j;
    weapon_num_m = _m;
    radar_num_k = _k;
    weapon_radar_indices = std::vector<int>();
    weapon_radar_num = std::vector<int>();

    activated_weapons_num = std::vector<int>(_m, 0);
    activated_radars_num = std::vector<int>(_k, 0);

    for(int i = 0; i < _scene.size(); i++){
        if(_scene[i] > 0){
            int temp_num = _scene[i];
            weapon_radar_indices.push_back(i);
            weapon_radar_num.push_back(temp_num);
            nz_weapon_radar++;

            // calculate in this scene, weapon i use how much, n_si and n_sk
            int weapon_index = cal_weapon_index(i);
            int radar_index = cal_radar_index(i);
            activated_radars_num[radar_index] += temp_num;
            activated_weapons_num[weapon_index] += temp_num;
        }
    }
}

Scene_SSL::Scene_SSL(int t_index, int w_index, int r_index, int m_, int k_){
    index = -1;
    target_index = t_index;
    obj_qjs = 0.0;
    time_tjs = 0.0;
    redcost = 0.0;

    weapon_num_m = m_;
    radar_num_k = k_;

    activated_weapons_num = std::vector<int>(m_, 0);
    activated_radars_num = std::vector<int>(k_, 0);

    nz_weapon_radar = 1;
    activated_radars_num[r_index] = 1;
    activated_weapons_num[w_index] = 1;

    weapon_radar_indices = std::vector<int>();
    weapon_radar_num = std::vector<int>();
    weapon_radar_indices.push_back(cal_ssl_index(r_index, w_index));
    weapon_radar_num.push_back(1);
}



void Scene_SSL::Set_Scene(int _j, std::vector<int> _scene, int _m, int _k){
    index = -1;
    obj_qjs = 0.0;
    redcost = 0.0;

    nz_weapon_radar = 0;
    target_index = _j;
    weapon_num_m = _m;
    radar_num_k = _k;
    weapon_radar_indices = std::vector<int>();
    weapon_radar_num = std::vector<int>();

    activated_weapons_num = std::vector<int>(_m, 0);
    activated_radars_num = std::vector<int>(_k, 0);

    for(int i = 0; i < _scene.size(); i++){
        if(_scene[i] > 0){
            int temp_num = _scene[i];
            weapon_radar_indices.push_back(i);
            weapon_radar_num.push_back(temp_num);
            nz_weapon_radar++;

            // calculate in this scene, weapon i use how much, n_si and n_sk
            int weapon_index = cal_weapon_index(i);
            int radar_index = cal_radar_index(i);
            activated_radars_num[radar_index] += temp_num;
            activated_weapons_num[weapon_index] += temp_num;
        }
    }
}


void Scene_SSL::PrintScene(){
    //printf("Target : %d\n", target_index);
    printf("%d SSL on target %d : ", nz_weapon_radar, target_index);
    if(nz_weapon_radar == 0){
        printf("No SSL on this target\n");
        return;
    }
    for(int i = 0; i < weapon_radar_indices.size(); i++){
        int ssl_index = weapon_radar_indices[i];
        printf("(%d,%d):%d, ", cal_weapon_index(ssl_index), cal_radar_index(ssl_index), weapon_radar_num[i]);
    }
    printf("\n");
}

bool Scene_SSL::is_same_w_r_t(Scene_SSL compare_scene){
    bool is_same = 1;
    if(compare_scene.target_index != target_index){
        is_same = 0;
        return is_same;
    }
    if(nz_weapon_radar != compare_scene.nz_weapon_radar){
        is_same = 0;
        return is_same;  
    }
    for(int i = 0; i < nz_weapon_radar; i++){
        if(compare_scene.weapon_radar_indices[i] != weapon_radar_indices[i]){
            is_same = 0;
            return is_same;
        }
        if(compare_scene.weapon_radar_num[i] != weapon_radar_num[i]){
            is_same = 0;
        return is_same;
        }
    }
    return is_same;
}

void SSL_Pool::print_all_scene(){
    printf("There are \t %d Scenes in the Scene Pool. \n", SSL_num);
    for_each(this->begin(), this->end(), std::mem_fun_ref(&Scene_SSL::PrintScene));
    printf("Print finished \n");
}

void SSL_Pool::add_scene(Scene_SSL _ssl){
      SSL_num++;
      push_back(_ssl);
}

void SSL_Pool::print_scene_by_target(int target_num){
    int scene_num = size();
    vector<Scene_SSL>* target_scenes = new vector<Scene_SSL>[target_num];

    for(int i = 0; i < scene_num; i++){
        Scene_SSL temp_ssl = at(i);
        target_scenes[temp_ssl.target_index].push_back(temp_ssl);
    }
    for(int t = 0; t < target_num; t++){
        printf("Target : %d, Size : %d\n", t, target_scenes[t].size() );
        for(int s = 0; s < target_scenes[t].size(); s++){
            target_scenes[t][s].PrintScene();
        }
    }
    if(target_scenes){
        delete[] target_scenes;
        target_scenes = NULL;
    }
}