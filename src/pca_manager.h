#ifndef PCA_MANAGER_H
#define PCA_MANAGER_H

#include <igl/opengl/glfw/Viewer.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif

using namespace std;
using Viewer = igl::opengl::glfw::Viewer;

class PCAManager {
public:

    PCAManager(const Viewer& viewer_, bool small_, string data_path, int m);
    ~PCAManager();

    void callback_draw_viewer_menu();
    bool callback_key_pressed(Viewer& viewer, unsigned char key, int modifier);

    bool draw = false;

    Eigen::MatrixXd V;
    Eigen::MatrixXi F;

private:
    const Viewer& viewer;

    int m = 10;
    int m_selection = m;
    unsigned int n = 2319;
    unsigned int nFiles;
    bool small = true;
    bool small_selection = small;

    double mone = -1.0;
    double one = 1.0;
    double zero = 0.0;

    Eigen::MatrixXd S; // covariance matrix
    Eigen::MatrixXd Faces;
    Eigen::MatrixXd V_template;
    Eigen::MatrixXi F_template;
    Eigen::MatrixXd eigenvectors;
    Eigen::VectorXd eigenvalues;
    Eigen::VectorXd mF; // mean face as vector

    // reconstruction
    Eigen::VectorXd base;
    Eigen::VectorXd wb; // base weights

    Eigen::VectorXd expression;

    // random face generator
    double random_variance = 7.0;
    double max_random_variance = 20.0;
    std::mt19937 eng;

    // eigenfaces
    Eigen::VectorXd slider;
    double weight_factor = 1000.0;


    // face morphing
    Eigen::VectorXd f1, f2;
    Eigen::VectorXd w1, w2;
    double p12 = 0.0;
    string f1_path = "../data/pca/small/person1.obj";
    string f2_path = "../data/pca/small/person2.obj";

    // expression
    double p_exp = 0.0;

    void compute_pca();
    void load_faces();
    void get_template(Eigen::MatrixXd &V, Eigen::MatrixXi &F);
    void load_template();
    void eigenface(const Eigen::VectorXd &slider, Eigen::VectorXd &base);
    void morphface();
    void change_expression(double p, Eigen::VectorXd &base);
    void random_face();
    void compute_weights(Eigen::VectorXd &face, Eigen::VectorXd &weights);
    void reconstruct(Eigen::VectorXd &face);
    void read_face(const string filename, Eigen::VectorXd &face);

    // Path variables

    string data_path = "../data/pca/small/default"; 
    string data_path_selection = data_path;
    string template_path;
    const string s_template_path = "../data/face_template/template_small.obj";
    const string l_template_path = "../data/face_template/template.obj";
    string neutral_path = "../data/pca/small/neutral.obj";
    string happy_path = "../data/pca/small/happy.obj";

};

#endif