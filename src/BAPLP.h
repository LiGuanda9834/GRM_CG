#include "Scene.h"
#include <vector>
#include <string.h>
#include <string>


using std::string;
using std::vector;

typedef std::vector<double> Doubles;

enum BasisStatus {
    BASESTAT_LOWER = 0, // Lower bound status
    BASESTAT_BASIC = 1, // Basic status
    BASESTAT_UPPER = 2, // Upper bound status
    BASESTAT_ZERO = 3   // Zero bound status
};

typedef BasisStatus BasisSTAT;

// The CMIPCol class represents a column (variable) in an integer programming problem.
// It contains basic information about the column, such as lower and upper bounds, objective function coefficients, and provides basic methods for operating on the column.
class Col {
public:
    // Constructors
    Col(unsigned _index, string _name, double _obj, double _lb, double _ub);
    Col(unsigned _index, string _name, Scene_SSL* _ssl);
    Col(Col* _col);

    // Destructor
    ~Col();

    // Set global lower bound
    void SetGloLb(double _lb);

    // Set global upper bound
    void SetGloUb(double _ub);

    // Get global lower bound
    double GetGloLb();

    // Get global upper bound
    double GetGloUb();

    // Get the number of non-zero elements in the column
    int GetRowNum();

    // Check if the column is a binary variable column
    bool IsBinary();

    // Get the variable corresponding to the column
    Scene_SSL* colGetVar();

    // Print column information
    void printCol();

public:
    unsigned index_;          // Column index
    double obj_;              // Coefficient in the objective function
    double lb_;               // Lower bound of the variable
    double ub_;               // Upper bound of the variable
    vector<Row> rows_;               // Rows containing non-zero elements
    Doubles vals_;            // Values of non-zero elements
    int lpipos_;              // Column index in the LP solver (-1 if not present)
    bool objchged_;           // Indicates if the objective function coefficient has changed
    bool lb_chged_;           // Indicates if the lower bound has changed
    bool ub_chged_;           // Indicates if the upper bound has changed
    bool coef_chged_;         // Indicates if the coefficient has changed
    Doubles lb_chgs_;         // Array of historical lower bound values
    Doubles ub_chgs_;         // Array of historical upper bound values
    unsigned lb_chgnum_;      // Number of lower bound changes
    unsigned ub_chgnum_;      // Number of upper bound changes
    double prisol_;           // Primal solution
    double redcost_;          // Reduced cost
    BasisSTAT basisstatus_;   // Basis status
    int notactivetime_;       // Number of times the variable is not active
    Scene_SSL* scene_ssl_;            // Corresponding variable in the problem
};


// The CMIPRow class represents a row (constraint) in an integer programming problem.
// It contains basic information about the row, such as left and right-hand sides, non-zero elements, and provides basic methods for operating on the row.
class Row {
public:
    // Constructors
    Row(unsigned _index, std::string _name, double _lhs, double _rhs);
    Row(Row* _row);

    // Destructor
    ~Row();

    // Calculate the activity of the current solution
    double GetActivity(double* _sol, double* _mincoef = nullptr, double* _maxcoef = nullptr);

    // Calculate the activity of the LP solution
    double GetActivity();

    // Print row information
    void PrintRow(double* _sol = nullptr);

    // Check if the given solution satisfies the row constraint
    bool CheckIsFeasible(double* _sol);

    // Print the fractional relaxation of the row
    void PrintRow1(double* _sol = nullptr);

private:
    unsigned index_;          // Row index
    vector<Col> cols_;               // Columns containing non-zero elements
    Doubles vals_;            // Values of non-zero elements
    double lhs_;              // Left-hand side
    double rhs_;              // Right-hand side
    unsigned lpcolnum_;       // Number of columns in cols participating in LP solving
    bool lhs_chged_;          // Indicates if the left-hand side has changed
    bool rhs_chged_;          // Indicates if the right-hand side has changed
    double activity_;         // a^Tx
    double dualsol_;          // Dual solution y
    BasisSTAT basisstatus_;   // Basis status
};

