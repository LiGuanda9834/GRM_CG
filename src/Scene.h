#ifndef _SCENE_H__
#define _SCENE_H__

#include "print.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

/**
 * @brief Each Scene meaning a set of weapons
 * 
*/
class Scene{
   public:
                                 // name of Scene. @todo Is that neccessary
      int index;                 // index of a column @todo how to give a id to a columns
      int target_index;          // y           : the index of the target
      double obj_qjs;            // q_js        : objective coff q_js
      double redcost;            // sum(xu)+v-q : redust cost of the specific scene
      std::vector<int> weapon_indices;  // s           : all the weapons in the Scene
   public:
      Scene() = default;
      Scene(int _j, std::vector<int> _scene);

      void Set_Scene(int _j, std::vector<int> _scene);

      ~Scene() = default;
      
      bool is_same_w_t(Scene compare_scene);

      inline bool emptyScene() const
      {
         return weapon_indices.size() < 1;
      }

      void PrintScene();
};



class ScenePool : public std::vector<Scene> {
public:
   int Scene_num;

   ScenePool(){Scene_num = 0;}
   ~ScenePool() = default;

   void print_all_scene();
   void add_scene(Scene scene);


   // @todo print scenes by target
   void print_scene_by_target(int target_num);
};




class Scene_SSL{
   public:
                                 // name of Scene. @todo Is that neccessary
      int index;                 // index of a column @todo how to give a id to a columns
      int target_index;          // y           : the index of the target
      double obj_qjs;            // q_js        : objective coff q_js
      double time_tjs;           // t_js        : how long s on j take 
      double redcost;            // sum(xu)+v-q : redust cost of the specific scene

      int weapon_num_m;
      int radar_num_k;

      // The number of weapons and radars that used in this SSL Scene
      std::vector<int> activated_weapons_num;
      std::vector<int> activated_radars_num;

      // s           : all the weapon and radars used in the Scene(nonzero)
      int nz_weapon_radar;
      std::vector<int> weapon_radar_indices; //  which ssl is activated
      std::vector<int> weapon_radar_num;     //  how many ssl has been used
   public:
      Scene_SSL() = default;

      // Set from [1,0,1,0,0,1,0,0] to [1,3,6], 
      Scene_SSL(int _j, std::vector<int> _scene, int m, int k);

      //Init a scene only has one ssl with w(w_index) and r(r_index), only assign it once
      Scene_SSL(int t_index, int w_index, int r_index, int m, int k);
      

      // Just like construct function, set all the 
      void Set_Scene(int _j, std::vector<int> _scene, int m, int k);
      
      void PrintScene();

      bool is_same_w_r_t(Scene_SSL compare_scene);


      ~Scene_SSL() = default;

      inline bool emptyScene() const
      {
         return weapon_radar_indices.size() < 1;
      }

      // indices = N, then weapon_index=(N % weapon_num_m),  radar_index=(N / radar_num_k)
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


class SSL_Pool : public std::vector<Scene_SSL> {
public:
   int SSL_num;

   SSL_Pool(){SSL_num = 0;}
   ~SSL_Pool() = default;

   void print_all_scene();
   void add_scene(Scene_SSL _ssl);

   // @todo print scenes by target
   void print_scene_by_target(int target_num);
};

#endif //ORA_PATH_H
