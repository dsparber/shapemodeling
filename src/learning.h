#pragma once

#include <igl/opengl/glfw/Viewer.h>

#include <vector>
#include <string>


using Viewer = igl::opengl::glfw::Viewer;

class LearningManager {

public:

    // Const reference to glfw viewer object
    const Viewer &viewer;

    // Dimension of latent space
    int latentDimension = 10;

    // Vector with slider weights
    std::vector<float> sliderWeights;

public:

    // Path to learning directory
    std::string learningDirectory = "../learning";

    // Command to call python script in shell
    std::string pythonCommand = "python decode.py";
    //std::string pythonCommand = "/Users/Sebastian/opt/anaconda3/bin/python decode.py";

public:

    // Flag to indicate main that mesh should be reloaded with output mesh path
    bool updateMesh = false;

    // Path to output mesh file created by python script
    std::string outputMesh = "../learning/learning_out.obj";

public:

    LearningManager(const Viewer &viewer_);

    ~LearningManager();

public:

    void callback_draw_viewer_menu();

    bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier);

};
