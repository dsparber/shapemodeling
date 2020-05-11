#ifndef PCA_H_ 
#define PCA_H_

#include <iostream>
#include <string>
#include <dirent.h>
#include <vector>

#include <Eigen/Core>
#include <igl/adjacency_list.h>
#include <igl/read_triangle_mesh.h>



using namespace std;
using namespace std;


/**
 *  
 **/
void compute_pca(string dirPath, Eigen::VectorXd& S_avg, Eigen::MatrixXd& S);

/**
 * @brief Given a directory, load all meshes in the S matrix where each row represents a face
 * 
 * 
 **/
void load_faces(
    string dir,
    Eigen::MatrixXd S);


bool hasSuffix(const string& s, const string& suffix);
void findFilesWithExt(const string &dirPath, const string &ext, vector<string> &listOfFiles);


#endif // PCA_H_