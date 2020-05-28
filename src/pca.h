#ifndef PCA_H
#define PCA_H

#include <igl/writeOBJ.h>
#include <igl/readOBJ.h>
#include <igl/read_triangle_mesh.h>

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

class PCA{
    public:
        PCA(){}
        PCA(int m_, string data_path_, string write_path_, bool small);
        ~PCA(){}

        Eigen::MatrixXd S; // covariance matrix
        Eigen::MatrixXd Faces;
        Eigen::MatrixXd V; // V and F we want to show
        Eigen::MatrixXi F; //
        Eigen::MatrixXd V_template;
        Eigen::MatrixXi F_template;
        Eigen::MatrixXd eigenvectors;
        Eigen::VectorXd eigenvalues;
        Eigen::VectorXd mF; // mean face
        Eigen::MatrixXd M; 
        Eigen::VectorXd expression;
        string data_path; 
        string write_path;
        string template_path;

        unsigned int m = 10;
        unsigned int n = 2319;
        unsigned int nFiles;

        double variance = 0.1;

        void compute_pca();

        /**
         * @brief Load all meshes from data_path in the Face matrix where each row represents a face
         * 
         **/
        void load_faces();
        void get_template(Eigen::MatrixXd &V, Eigen::MatrixXi &F);
        void load_template();
        void eigenface(const Eigen::VectorXd &slider, Eigen::VectorXd &base);
        void morphface(Eigen::VectorXd &f1, Eigen::VectorXd &f2, Eigen::VectorXd &w1, Eigen::VectorXd &w2, double p);
        void change_expression(double p, Eigen::VectorXd &base);
        void random_face();
        void compute_weights(Eigen::VectorXd &face, Eigen::VectorXd &weights);
        void reconstruct(Eigen::VectorXd &face);
        void read_face(const string filename, Eigen::VectorXd &face);
    private:
        const string s_template_path = "../data/face_template/template_small.obj";
        const string l_template_path = "../data/face_template/template.obj";
        vector<string> listOfFiles;
        std::mt19937 eng{std::chrono::high_resolution_clock::now().time_since_epoch().count()};
        const string neutral_path = "../data/aligned_faces_example/default/jan-neutral.objaligned.obj";
        const string happy_path = "../data/aligned_faces_example/default/jan-smile.objaligned.obj";
};


#endif // PCA_H