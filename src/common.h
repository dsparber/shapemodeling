#ifndef COMMON_H_ 
#define COMMON_H_

#include <iostream>
#include <string>

#include <vector>

#include <Eigen/Core>
#include <igl/adjacency_list.h>
#include <igl/read_triangle_mesh.h>

#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif

#include <imgui/imgui.h>

#define SCREEN_SCALE_X 2.0f
#define SCREEN_SCALE_Y 1.5f

#define DEFAULT_WINDOW_SIZE ImVec2(200.0f * SCREEN_SCALE_X, 600.0f * SCREEN_SCALE_Y)

using namespace std;


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


/**
 * @brief Reshape vector into matrix
 * Input:
 * @param flat      input vector
 * @param rows      row dimension of matrix
 * @param cols      column dimension of matrix
 * Output:
 * @param matrix    output matrix
 */
void reshape(Eigen::VectorXd flat, int rows, int cols, Eigen::MatrixXd &matrix);

void compute_pca(string dirPath, int m, Eigen::VectorXd& F_m, Eigen::MatrixXd& W, Eigen::VectorXd& eigenvalues);
void load_faces(string dir, Eigen::MatrixXd &X);
void read_face_vector(string filename, Eigen::VectorXd &v);
#endif // COMMON_H_