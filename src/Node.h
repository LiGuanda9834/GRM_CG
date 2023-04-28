#ifndef MCFDR_NODE_H
#define MCFDR_NODE_H

#include <queue>
#include "Scene.h"
#include "Statistics.h"
#include "COLCHG.h"

enum NodeType
{
    CHILD,      // Child node
    SIBLING,    // Sibling node
    LEAF,       // Leaf node
    FORK,       // Fork, common ancestor of two leaf nodes
    PRUNED,     // Pruned node
    SUBTREE,    // Root node of a subtree being processed
    ERROR       // Error node
};
typedef enum NodeType NODETYPE;

enum NodeBoundType
{
    BranchBOUND,  // Bound change due to branching
    INFERBOUND,   // Bound not obtained through branching
    StrongBOUND   // Bound obtained through strong branching
};


// The NODE class represents a node in the branch and bound tree.
// This class contains information related to the node, such as lower bound, estimate, child nodes, etc.
class Node {
public:
    double lowerBound;                // Lower bound
    double estimate;                  // Estimate
    Node* parent;                     // Parent node
    std::vector<Node*> children;      // Child nodes
    int branchColPos;                 // Branching column position
    double branchColSol;              // Branching column's current relaxed solution value
    int index;                        // Index (-1 represents an ERROR node)
    NODETYPE type;                    // Node type
    unsigned childnum;                // Number of unpruned child nodes
    unsigned depth;                   // Current node depth in the tree
    bool active;                      // Whether the node is on the active path
    bool isSolved;                    // Whether the current node has been solved
    Statistics* stat;                 // Pointer to the Statistics object
    COLCHG* colchg;                   // Pointer to the COLCHG object

    // default constructor, used to initialize the root node.
    Node(){}

    // Constructor, initializes a Node object based on the given parameters,
    // including index, node type, lower bound, parent node, and depth, etc.
    // It also initializes other member variables.
    Node(Statistics* stat0, int index_, NODETYPE type_, double lowerBound_, Node* parent_, unsigned depth_);

    // Another constructor, in addition to initializing a Node object,
    // constructs the constraint modifications of the node based on the given LP information.
    Node(Statistics* stat_, SOLVERLP* lp_, int index_, NODETYPE type_, double lowerBound_, Node* parent_, unsigned depth_);

    // Destructor, checks and deletes 'lpbasis' and 'bChg'
    ~Node();
    
    // Initilize the root node
    void Init(){}

    // Sets the branching column position and corresponding solution.
    void SetBranchCol(int pos, double sol);

    // Gets the branching column position and corresponding solution.
    void GetBranchCol(int& pos, double& sol);

    // Prints the information of all parent nodes of the current node, including index, active status, and constraint modification.
    void PrintParent();

    // Constructs a bound change object for the current node using the given lp information.
    void ConstrBchg(SOLVERLP* lp_);

    // Constructs an LP basis object for the current node using the given lp information.
    void ConstrBasis(SOLVERLP* lp_);

    // Sets the LP basis for the given lp based on the current node or its parent node's LP basis information.
    void SetLPBasis(SOLVERLP* lp_);
};


/*
enum NodeType
{
   CHILD, 			//子节点
   SIBLING, 		//兄弟节点
   LEAF, 			//叶节点
   FORK, 			//叉子，两个叶节点的共同祖先
   PRUNED, 			//被剪枝
   SUBTREE, 		//被处理为子树的根节点
   ERROR 			//错误
};
typedef enum NodeType NODETYPE;

enum GapType
{
   GAP_VALID,
   GAP_INVALID,
   GAP_UNBOUNDED
};
typedef enum GapType GAPTYPE;

enum NodeBoundType
{
   CMIPBranchBOUND,			//分支的界变化
   INFERBOUND,			//不是分支得到的
   StrongBOUND			//强分支得到的界 
};

typedef enum NodeBoundType NODEBOUNDTYPE;

enum NodeStatus {
    Unsolved, 
    Infeasible, 
    PrunedByBound, 
    Fractional, 
    Integral
};
typedef enum NodeStatus NODESTATUS;

// This class to take down a spcific lp solution. 
// Still need to update to the WTA form
class LpSol : public std::vector<std::pair<Scene *, double>>
{
   public:
      double obj;

   public:
      LpSol() = default;
      ~LpSol(){};

      double accumulate()
      {
         double total = 0;
         for (auto &p : *this)
         {
            total += p.second;
         }
         return total;
      }

      double cost(int x, int y)
      {
         
      }
      //  bool integerFeasible() {
      //      for (auto v: *this) {
      //          if (!huq::integral(v.second)) return false;
      //      }
      //      return true;
      //  }
};

class Node
{
   public:
      Node() = default;

      Node(const Node *_parent, long _nodeId, bool branchOnNum, int a, int b, int c = -1)
          : parent(_parent), nodeId(_nodeId),
            lbNumVehicle(branchOnNum ? a : _parent->lbNumVehicle),
            ubNumVehicle(branchOnNum ? b : _parent->ubNumVehicle),
            forbidden(_parent->forbidden),
            status(NodeStatus::Unsolved), lpsol(nullptr),
            lb(_parent == nullptr ? 0 : _parent->lb)
      {
         if (!branchOnNum)
         {
            if (c == 0)
            {
               forbidden[a][b] = true;
            }
            else if (c == 1)
            {
               auto nVertex = forbidden.size();
               for (int i = 0; i < nVertex; ++i)
               {
                  if (i != a && !forbidden[i][b])
                  {
                     forbidden[i][b] = true;
                  }
                  if (i != b && !forbidden[a][i])
                  {
                     forbidden[a][i] = true;
                  }
               }
            }
            else
            {
               std::cerr << "invalid branch\r\n";
            }
         }
      }

      ~Node()
      {
         parent = nullptr;
         if (lpsol != nullptr)
         {
            delete lpsol;
            lpsol = NULL;
         }
      }

      std::string statusStr() const
      {
         if (status == NodeStatus::Unsolved)
            return "Unsolved";
         else if (status == NodeStatus::Infeasible)
            return "Infeasible";
         else if (status == NodeStatus::PrunedByBound)
            return "Pruned";
         else if (status == NodeStatus::Fractional)
            return "Fractional";
         else if (status == NodeStatus::Integral)
            return "Integral";
         else
            return "Unknown";
      }
      // Use this tho check 
      bool valid(const Scene &temp_scene) const
      {
         for (int i = 0; i < temp_scene.weapon_indices.size(); ++i)
         {
            int target_id = temp_scene.target_index, weapon_id = temp_scene.weapon_indices[i];
            if (forbidden[target_id][weapon_id]) {
                return false;
            }
        }
        return true;
      }

   public:
      const Node *parent;
      long nodeId;

      int lbNumVehicle;
      int ubNumVehicle;

      // Use this to checkout weather a weapon could attack a target
      // forbiddeb[target][weapon] == 0 means weapon could not reach target
      std::vector<std::vector<bool>> forbidden;

      NodeStatus status;
      LpSol *lpsol;
      double lb;
};

*/
#endif //MCFDR_NODE_H
