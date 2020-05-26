#pragma once

#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

#include <igl/read_triangle_mesh.h>
#include <igl/writeOBJ.h>
#include <Eigen/Dense>

#include <iostream>
#include <string>
#include <map>
#include <vector>
#ifdef _WIN32
#include "dirent.h"
#define SEP '\\'
#else
#include <dirent.h>
#define SEP '/'
#endif

using namespace std;
using Viewer = igl::opengl::glfw::Viewer;
using ImGuiMenu = igl::opengl::glfw::imgui::ImGuiMenu;

class PCA{
    public:

        const Viewer &viewer;

        PCA(const Viewer &viewer_); //, map<char, string> &inputs);

        void callback_draw_viewer_menu();

        bool callback_pre_draw(Viewer &viewer);

        bool callback_mouse_down(Viewer &viewer, int button, int modifier);

        bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier);
        
        bool callback_mouse_up(Viewer &viewer, int button, int modifier);
        
        bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y);

        string templateFile = smallTemplateFile;
        string dataPath = "../data/aligned_faces_example/default/";
        string writePath = "./";
    private:
        void compute_pca(string dirPath, int m, Eigen::VectorXd& F_m, Eigen::MatrixXd& W, Eigen::VectorXd& EV);

        void load_faces(string dir, Eigen::MatrixXd &X);

        bool hasSuffix(const string& s, const string& suffix);

        void findFilesWithExt(const string &dirPath, const string &ext, vector<string> &listOfFiles);

        void random_face();

        void update_face();

        vector<double> weights;

        int down_mouse_x = -1;
        int down_mouse_y = -1;

        int m = 10;
    
        int nFiles;
        int d;  
        int n;  //number of points used in template
        string smallTemplateFile = "../data/face_template/template_small.obj";
        string largeTemplateFile = "../data/face_template/template.obj";
        Eigen::MatrixXd V_template;
        Eigen::MatrixXd F_template;
        Eigen::MatrixXd V;
        Eigen::MatrixXd F;
};