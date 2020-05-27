#ifndef PCA_H
#define PCA_H

#include <igl/writeOBJ.h>
#include <igl/readOBJ.h>
#include <igl/read_triangle_mesh.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

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
        Eigen::MatrixXd V; //
        Eigen::MatrixXi F;
        Eigen::MatrixXd V_template;
        Eigen::MatrixXi F_template;
        Eigen::MatrixXd eigenvectors;
        Eigen::VectorXd eigenvalues;
        Eigen::VectorXd mF; // mean face
        Eigen::MatrixXd M; 

        string data_path; 
        string write_path;
        string template_path;

        unsigned int m = 10;
        unsigned int n = 2319;
        unsigned int nFiles;


        void compute_pca();

        /**
         * @brief Load all meshes from data_path in the Face matrix where each row represents a face
         * 
         **/
        void load_faces();
        void get_template(Eigen::MatrixXd &V, Eigen::MatrixXi &F);
        void load_template();
        void morph_face(const vector<double> &slider);

    private:
        const string s_template_path = "../data/face_template/template_small.obj";
        const string l_template_path = "../data/face_template/template.obj";
        vector<string> listOfFiles;
};


#endif // PCA_H