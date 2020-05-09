#include "PCA.h"
#include <igl/adjacency_list.h>
#include <vector>

int example_function(Eigen::MatrixXi& F){
    vector<vector<int>> A;
    igl::adjacency_list(F, A);
    cout << "Computed adjacency_list\n";
    return 0;
}