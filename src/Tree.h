// Branch-and-bound Tree

#ifndef _TREE_H_
#define _TREE_H_

#include "Node.h"
#include "BAPLP.h"
#include "COLCHG.h"

// auto CompareNodeScoreDescending = [](const Node &a, const Node &b)
// {
//    return a.lb > b.lb;
// };

enum SearchStrategy {
    DepthFirst, PrimalBoundFirst
};


// GapType enumeration for representing the gap type in the branch and bound tree.
enum GapType
{
    GAP_VALID,      // Valid gap.
    GAP_INVALID,    // Invalid gap.
    GAP_UNBOUNDED   // Unbounded gap.
};
typedef enum GapType GAPTYPE;

// Tree class represents a branch and bound tree containing nodes, lower bounds, upper bounds, etc.
class Tree {
public:
    // Global
    double lowerBound;         // Global lower bound.
    double upperBound;         // Global upper bound.
    double isCutOffBound;      // Cut-off upper bound for branch and bound.
    bool isBoundChg;           // Flag indicating whether there is a constraint boundary change.

    // Node-related
    unsigned nodeNum;          // Total number of nodes.
    Node* root;                // Root node.
    vector<Node*> children;    // Child nodes.
    vector<Node*> siblings;    // Sibling nodes.
    vector<Node*> errors;      // LP error nodes.
    Heap* leaves;              // Leaf nodes.
    Node* focusNode;           // Currently focused node.
    int childNum;              // Number of child nodes.
    int siblingNum;            // Number of sibling nodes.
    int leaveNum;              // Number of leaf nodes.
    int childrenIter;          // Child nodes iterator.
    int siblingsIter;          // Sibling nodes iterator.
    int leavesIter;            // Leaf nodes iterator.

    // Depth information
    unsigned maxDepth;         // Maximum depth of the branch and bound tree.
    int nDepthStat;            // Depth statistics (current depth?)

    // Gap-related
    double gap;                // Gap between the current best solution and lower bound.
    GAPTYPE gapType;           // Gap type.

    // Update path-related
    Node* fork;                // Common ancestor of the last focusNode and current focusNode.
    unsigned forkPos;          // Position of the fork node.
    vector<Node*> path;        // Eager path.

    // Pruning-related
    vector<Node*> pruned;      // Pruned nodes.
    unsigned prunedNum;        // Number of pruned nodes.

    // Pointers
    SOLVEREnvPtr env;          // Pointer to the solver environment.
    Statistics* stat;          // Pointer to the Statistics object.

    // Member functions
    // Basic member functions
    Tree(SOLVEREnvPtr env, Statistics* stat);    // Constructor
    ~Tree();                                      // Destructor
    bool Init();                                  // Initialization
    bool Reset();                                 // Reset Tree
    bool GetMinLowerBound();                      // Get the current global lower bound.
    bool CalGap();                                // Calculate the gap between the current best solution and lower bound.
    bool IsGapSmall();                            // Check if the gap is small enough.
    bool AppendNewBound(Col*, double&, bool);     // Append new bound.

    // Update path-related member functions
    bool FindFork();                              // Find the common ancestor of the last focusNode and current focusNode.
    bool PrintPath();                             // Print the current path.
    bool UpdatePath();                            // Update the eager path.

    // Pruning-related member functions
    bool PruneUpdateBoundChg(Node* node);         // Update the bound and perform pruning.
    bool AddPruned(Node* node);                   // Add a pruned node.
    bool DeleteNode(Node* node);                  // Delete a node.
    bool PruneByPath(Node* node);                 // Prune based on the path.
    bool DeletePruned();                          // Delete pruned nodes
    bool DropRedundance();                        // Delete redundant nodes.

    // Branching-related member functions
    bool Branch(SOLVERLP* lp, Col* col, double&, double&, double&, Node*);        // Branch on a node.
    bool Branch(SOLVERLP* lp, Col* col, double&, double&, double&, Node*, int);   // Branch on a node, allowing one-sided branching.

    // Node handling-related member functions
    bool FlushChildrenSiblings();                 // Convert child nodes to sibling nodes, and sibling nodes to leaf nodes.
    bool FlushToLeaves();                         // Convert all child and sibling nodes to leaf nodes.
    unsigned GetCurDiffDepth();                   // Get the difference between the current node depth and maximum depth.

    // Pseudocost and estimation-related member functions
    bool UpdatePseudocost(SOLVERLP* lp);                         // Update pseudocost values.
    bool UpdatePseudocostObjLim(SOLVERLP* lp);                   // Update pseudocost values, reaching the objective limit.
    bool UpdatePseudocostInf(SOLVERLP* lp);                      // Infeasible case, update pseudocost values based on Danna and Lodi.
    double CalcEstimate(Col*, double&, double&);                 // Calculate node's estimation value.
    //bool UpdateEstimate(BranchCand*);                            // Update estimation value.
};


// The MinHeap class is a min-heap used to assist the branch and bound tree
// in quickly selecting a node during node selection.
class Heap {
public:
    Tree* tree;                      // Pointer to the Tree class, used to store tree structure information.
    std::vector<Node*> data;         // Vector of Node types, stores nodes in the heap.
    Statistics* stat;                // Pointer to the Statistics class, used to manage statistical information.
    unsigned size;                   // Unsigned integer representing the size of the heap.

    // Constructor, initializes a MinHeap instance based on the given Statistics and Tree.
    Heap(Statistics* stat_, Tree* tree_);

    // Destructor.
    ~Heap();

    // Adds a node to the heap.
    bool Append1(Node*& node);

    // Retrieves the best node (with the smallest weight) from the heap.
    Node* GetBest();

    // Retrieves the lowest-level leaf node from the heap.
    Node* GetLowest();

    // Gets the number of leaf nodes in the heap.
    int GetLeafNum();

    // Gets the lower bound of the nodes in the heap.
    double GetLowerBound();

    // Deletes a node from the heap based on the given position 'pos'.
    bool deletebypos(unsigned pos);

    // Initializes the heap based on the node selection strategy in the parameters.
    bool Init();
};
/*
class Tree 
{
   public:
      Tree(bool DepthFirst) //: que(CompareNodeScoreDescending)
      {
         if (DepthFirst)
            searchStrategy = SearchStrategy::DepthFirst;
         else
            searchStrategy = SearchStrategy::PrimalBoundFirst;
      }

      ~Tree() = default;

      void pushByMove(const Node &node)
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
         {
            tree.push_back(std::move(node));
         }
         // else
         // {
         //    que.push(std::move(node));
         // }
      }

      void emplace(const Node *parent, long nodeId, bool branchOnNum, int a, int b, int c = -1)
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
         {
            tree.emplace_back(parent, nodeId, branchOnNum, a, b, c);
         }
         // else
         // {
         //    que.emplace(parent, nodeId, branchOnNum, a, b, c);
         // }
      }

      const Node &next()
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            return tree.back();
         // else
         //    return que.top();
      }

      void pop()
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            tree.pop_back();
         // else
         //    que.pop();
      }

      bool empty() const
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            return tree.empty();
         // else
         //    return que.empty();
      }

      int size() const
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            return tree.size();
         // else
         //    return que.size();
      }

      double updateGlobalLb()
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
         {
            double globalLb = tree.front().lb;
            for (auto &node: tree) {
                if (globalLb > node.lb) {
                    globalLb = node.lb;
                }
            }
            return globalLb;
         }
         // else
         // {
         //    return que.top().lb;
         // }
      }

   public:
      SearchStrategy searchStrategy;

   private:
      std::vector<Node> tree;
      // std::priority_queue<Node, std::vector<Node>, decltype(CompareNodeScoreDescending)> que;

};

*/
#endif //_TREE_H_
