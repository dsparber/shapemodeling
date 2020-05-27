#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>
#include <memory>
#include <string>
#include <Eigen/Dense>
#include "../src/warping.h"
#include "../src/rigid_registration.h"
#include "../src/common.h"

using namespace std;
using namespace Eigen;
using Viewer = igl::opengl::glfw::Viewer;

int main(int argc, char *argv[])
{
    double lambda = 0.3;
    int iterations = 5;
    double threshold = 0.017;
    std::string scan_folder = "../data/scans";                                  // Path to folder with scans.
    std::string path_template_obj = "../data/face_template/template_small.obj"; // Path to template.obj.
    std::string path_template_txt = "../data/face_template/template_small.txt"; // Path to template.txt
    std::string output_folder = "../data/warped_meshes";                        // Path to output folder.

    // Store used parameters to file
    std::ofstream outfile;
    outfile.open(output_folder + "/params", ios::out);
    outfile << "threshold = " << threshold << endl;
    outfile << "lambda = " << lambda << endl;
    outfile << "iterations = " << iterations << endl;
    outfile.close();

    warp_meshes_to_folder(scan_folder,
                          path_template_obj,
                          path_template_txt,
                          output_folder,
                          lambda, iterations, threshold);
    return 0;
}