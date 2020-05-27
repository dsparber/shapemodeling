#ifndef FACE_ALIGNMENT_H
#define FACE_ALIGNMENT_H

#include <igl/opengl/glfw/Viewer.h>
#include <Eigen/Core>
#include <string>
#include <memory>

#include "warping.h"

using Viewer = igl::opengl::glfw::Viewer;

class FaceAlignmentManager {
public:

    FaceAlignmentManager(Viewer& viewer);

    ~FaceAlignmentManager();

    void callback_draw_viewer_menu();

    bool callback_key_pressed(Viewer& viewer, unsigned char key, int modifier);

    // parameters rigid registration
    std::string path_template = "../data/landmarks_example/headtemplate.obj";
    std::string path_scan = "../data/landmarks_example/person0_.obj";
    std::string path_landmarks_template = "../data/landmarks_example/headtemplate_23landmarks";
    std::string path_landmarks_scan = "../data/landmarks_example/person0__23landmarks";

    // parameters warping
    double lambda = 1;
    int iterations = 1;
    double relative_distance_threshold = 0.01;
    bool use_landmark_constraints = false;
    std::string result_path = "../data/landmarks_example/person0_alignment_result.obj";

private:
    Viewer& viewer;

    std::unique_ptr<Warping> warping;

    // rigidly aligned meshes
    Eigen::MatrixXd V_template;
    Eigen::MatrixXd V_scan;
    Eigen::MatrixXi F_template;
    Eigen::MatrixXi F_scan;

    // warped mesh
    Eigen::MatrixXd V_warped;
    Eigen::MatrixXi F_warped;

    void rigidly_align();
    void warp();
    void show_mesh(const int mesh_type);
};

#endif