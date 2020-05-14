#pragma once

#include <igl/opengl/glfw/Viewer.h>

#include <Eigen/Dense>

#include <map>
#include <vector>
#include <deque>
//#include <queue>
#include <string>


using Viewer = igl::opengl::glfw::Viewer;

class LandmarkManager {

public:

    // Const reference to vertex array (#Vx3)
    const Eigen::MatrixXd &V;
    
    // Const reference to face array (#Fx3)
    const Eigen::MatrixXi &F;

    // Const reference to glfw viewer object
    const Viewer &viewer;

    // TODO: (Sebastian) Points 14 <-> 15 inconsistent in pictures on page 7 of slides
    // Map of all valid landmarks and descriptions (keys set here will be the valid landmark ids)
    std::map<int, std::string> descriptions = {
        {1, "outer end of the left brow"},
        {2, "upper-middle end of the left brow"},
        {3, "inner end of the left brow"},
        {4, "inner end of the right brow"},
        {5, "upper-middle end of the right brow"},
        {6, "outer end of the right brow"},
        {7, "outer end of the left eyelid"},
        {8, "middle end of the left eyelid"},
        {9, "inner end of the left eyelid"},
        {10, "inner end of the right eyelid"},
        {11, "middle end of the right eyelid"},
        {12, "outer end of the right eyelid"},
        {13, "outer end of the left cheek"},          // TODO: (Sebastian) find good description for this
        {14, "outer left end of the nose"},
        {15, "tip end of the nose"},
        {16, "outer right end of the nose"},
        {17, "outer end of the right cheek"},
        {18, "outer left of the mouth"},
        {19, "upper-middle end of the mouth"},
        {20, "outer right of the mouth"},
        {21, "tip of the chin"},
        {22, "TODO: 22 description"},
        {23, "TODO: 23 description"}
    };

    // Map of containing landmark ids to vertex ids
    std::map<int, int> landmarks;

    // Queue containitn all unassigned landmarks
    std::deque<int> landmarks_queue;

    // Currently selected landmark id and vertex index
    int current_landmark = -1;
    int current_vertex = -1;

public:

    LandmarkManager(const Eigen::MatrixXd &V_, const Eigen::MatrixXi &F_, const Viewer &viewer_);

    ~LandmarkManager();

private:

    int pick_landmark_vertex(int mouse_x, int mouse_y) const;

public:

    int down_mouse_x = -1;
    int down_mouse_y = -1;

    void callback_draw_viewer_menu();

    bool callback_pre_draw(Viewer &viewer);

    //bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y);

    bool callback_mouse_down(Viewer &viewer, int button, int modifier);

    bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier);

public:

    // Input and output file paths
    std::string inputFile = "../data/landmarks_example/test.txt";
    std::string outputFile = "../data/landmarks_example/test.txt";

    void read_from_file(const std::string &path);

    void write_to_file(const std::string &path);

public:

    inline std::map<int, int> &get_landmarks() {
        return landmarks;
    }

};
