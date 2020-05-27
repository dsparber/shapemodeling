#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

/*** insert any necessary libigl headers here ***/
#include <math.h> 

#include <string>
#include <sstream>
#include <memory>

#include "../src/learning.h"

using Viewer = igl::opengl::glfw::Viewer;

Viewer viewer;

// vertex array, #V x3
Eigen::MatrixXd V;
// face array, #F x3
Eigen::MatrixXi F;
// normals
Eigen::MatrixXd N;

// mean face
Eigen::VectorXd F_m;
// place holder
Eigen::MatrixXd V_new;

// Unique pointer to learning manager
std::unique_ptr<LearningManager> learningManager;

// ************************Function Declaration ************************ //
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers);
bool load_mesh(const std::string &filename);
// ********************************************************************* //


bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier){

	switch (key) {
		case '1':
			break;
		case '2':
			break;
		case '3':
			break;
		case '4':
			break;
	}

	// Passing on key pressed event to learning manager
	learningManager->callback_key_pressed(viewer, key, modifier);
	// Checking if learning manager tells main to update mesh
	if (learningManager->updateMesh) {
		// Loading new mesh created with learning python script
		load_mesh(learningManager->outputMesh);
		// Resetting mesh update flag of learning manager 
		learningManager->updateMesh = false;
	}

	return true;

}

bool load_mesh(const std::string &filename){
	
	igl::read_triangle_mesh(filename,V,F);
	viewer.data().clear();
	viewer.data().set_mesh(V, F);
	viewer.core.align_camera_center(V);

	return true;

}

int main(int argc,char *argv[]){

	// Initialize larning manager
	learningManager = std::unique_ptr<LearningManager>(new LearningManager(viewer));

	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]()
	{
		// Draw parent menu content
		menu.draw_viewer_menu();

		// Drawing custom learning window menu
		learningManager->callback_draw_viewer_menu();
		// Checking if learning manager tells main to update mesh
		if (learningManager->updateMesh) {
			// Loading new mesh created with learning python script
			load_mesh(learningManager->outputMesh);
			// Resetting mesh update flag of learning manager 
			learningManager->updateMesh = false;
		}

	};

	viewer.callback_key_pressed = callback_key_pressed;
	viewer.launch();

	return 0;

}
