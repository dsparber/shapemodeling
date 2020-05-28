#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>
#include <string>
#include "../src/pca_manager.h"
#include "../src/common.h"

#ifndef _WIN32
    #include <unistd.h>
#endif

using namespace std;
using namespace Eigen;
using Viewer = igl::opengl::glfw::Viewer;

Viewer viewer;

int m = 10;
string data_path = "../data/pca/small/default";
bool s = true;

// Unique pointer to pca manager
std::unique_ptr<PCAManager> pcaManager;

bool load_mesh(const Eigen::MatrixXd &V, const Eigen::MatrixXi &F) {

	// Clearing viewer data, setting mesh and aligning camera
	viewer.data().clear();
	viewer.data().set_mesh(V, F);
	viewer.core.align_camera_center(V);
	return true;

}


bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier){
    pcaManager->callback_key_pressed(viewer, key, modifier);
    if(pcaManager->draw){
        load_mesh(pcaManager->V, pcaManager->F);
    }
}



int main(int argc, char *argv[])
{
#ifndef _WIN32
    int opt;
	while((opt = getopt(argc, argv, "m:d:s:")) != -1){
		switch(opt){
            case 'm':
                m = atoi(optarg);
				break;
            case 'd':
                data_path = optarg;
                break;
            case 's':
                int small = atoi(optarg);
                s = small > 0;
                break;
		}
	} 
#endif

	// Initialize PCA manager
	pcaManager = std::unique_ptr<PCAManager>(new PCAManager(viewer, s, data_path, m));
	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]() {
       	// Draw parent menu content
		menu.draw_viewer_menu(); 
        pcaManager->callback_draw_viewer_menu();
        if(pcaManager->draw){
            load_mesh(pcaManager->V, pcaManager->F);
        }
    };
    viewer.callback_key_pressed = callback_key_pressed;
    viewer.launch();
    return 0;
}