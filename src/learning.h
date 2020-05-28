#pragma once

#include <igl/opengl/glfw/Viewer.h>

#include <vector>
#include <string>
#include <random>

using Viewer = igl::opengl::glfw::Viewer;

class LearningManager {

public:

    // Const reference to glfw viewer object
    const Viewer &viewer;

    // Dimension of latent space
    int latentDimension = 10;

    // Vector with slider weights
    std::vector<float> sliderWeights;

    // Random seed object for weight randomization
    std::mt19937 eng;

public:

    LearningManager(const Viewer &viewer_);

    ~LearningManager();

public:

    // Flag to indicate main that mesh should be reloaded with output mesh path
    bool updateMesh = false;

    // Path to output mesh file created by python script
    std::string outputMesh = "../learning/learning_out.obj";

private:

    // Path to learning directory
    std::string learningDirectory = "../learning";

    // Command to call python dcecode script in shell
    std::string pythonDecodeCommand = "python decode.py";
    //std::string pythonDecodeCommand = "/Users/Sebastian/opt/anaconda3/bin/python decode.py";

    std::string pythonEncodeCommand = "python encode.py";
    //std::string pythonEncodeCommand = "/Users/Sebastian/opt/anaconda3/bin/python encode.py";

public:

    void callback_draw_viewer_menu();

    bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier);

private:

    // Path for morph face 1 and 2
    std::string morphFace1 = "../data/warped_meshes/alain_normal_warped.obj";
    std::string morphFace2 = "../data/warped_meshes/selina-neutral_warped.obj";

    // Latent vectors gathered from encoding morph faces 1 and 2
    std::vector<float> morphFace1Latents;
    std::vector<float> morphFace2Latents;

    // Path of latent file created by python encode script
    std::string latentFile = "../learning/latents.txt";

    // Reading latent vectors from latent file of python encode script 
    void loadLatentVectors(const std::string &fileName);

private:
    
    // Full system command to call python encode script
    std::string systemEncodeCommand;
    
    // Full system command to call python decode script
    std::string systemDecodeCommand;

    // Updating system commands for calling python encode/decode
    void updateCommands();

};
