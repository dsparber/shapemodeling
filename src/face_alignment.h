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

    FaceAlignmentManager();

    ~FaceAlignmentManager();

    void callback_draw_viewer_menu();

    bool callback_key_pressed(Viewer& viewer, unsigned char key, int modifier);

    // parameters rigid registration
    std::string path_template = "../data/face_template/template_small.obj";
    std::string path_landmarks_template = "../data/face_template/template_small.txt";
    std::string path_scan = "../data/landmarks_example/person0_.obj";
    std::string path_landmarks_scan = "../data/landmarks_example/person0__23landmarks";

    // parameters warping
    double lambda = 0.3;
    int iterations = 5;
    double relative_distance_threshold = 0.017;
    bool use_landmark_constraints = false;
    std::string result_path = "../data/landmarks_example/person0_alignment_result.obj";

    // parameters for updating mesh from main
    // 0 for no update, 1 for scan, 2 for template, 3 for warped mesh
    int mesh_to_show = 0;

    // rigidly aligned meshes
    Eigen::MatrixXd V_template;
    Eigen::MatrixXd V_scan;
    Eigen::MatrixXi F_template;
    Eigen::MatrixXi F_scan;

    // warped mesh
    Eigen::MatrixXd V_warped;
    Eigen::MatrixXi F_warped;

private:
    std::unique_ptr<Warping> warping;

    void rigidly_align();
    void warp();
};

#endif
