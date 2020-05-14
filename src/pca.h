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
 * Input:
 * @param dirPath   path to aligned faces directory with M faces
 * @param m         number of principle components 
 * Output:  
 * @param F_m       mean face vector 1xd, d is divisible by 3
 * @param W         dxm matrix consisting of the eigenvectors in its columns
 **/
void compute_pca(string dirPath, int m, Eigen::VectorXd& F_m, Eigen::MatrixXd& W);

/**
 * @brief Given a directory, load all meshes in the S matrix where each row represents a face
 * Input:
 * @param dir       path to aligned faces directory with M faces
 * Output:
 * @param X         Mxd matrix with each row being a face vector  
 * 
 **/
void load_faces(string dir, Eigen::MatrixXd &X);

//////////////////////////////////TODO: put into common.h/////////////////////////////////////
bool hasSuffix(const string& s, const string& suffix);

/**
 * @brief Get filenames with specified extension 
 * Input:
 * @param dirPath   path to directory
 * @param ext       extension e.g. ".obj"
 * Output:
 * @param listOfFiles   vector of filenames
 **/
void findFilesWithExt(const string &dirPath, const string &ext, vector<string> &listOfFiles);
/////////////////////////////////////////////////////////////////////////////////////////////

#endif // PCA_H_