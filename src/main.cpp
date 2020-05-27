#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

/*** insert any necessary libigl headers here ***/
#include <string>
#include <memory>
#include <math.h> 

#include "common.h"
#include "landmark.h"
#include "face_alignment.h"

using namespace std;
using namespace Eigen;

using Viewer = igl::opengl::glfw::Viewer;

Viewer viewer;

// vertex array, #V x3
Eigen::MatrixXd V;
// face array, #F x3
Eigen::MatrixXi F;

// Unique pointer to landmark manager
std::unique_ptr<LandmarkManager> landmarkManager;

// Unique pointer to face alignment manager
std::unique_ptr<FaceAlignmentManager> faceAlignmentManager;

// Pipeline mode for showing different window
enum PipelineMode {
	LANDMARK_MODE = 0,
	FACE_ALIGNMENT_MODE,
	PCA_MODE,
	BONUS_MODE
};

enum PipelineMode pipelineMode = LANDMARK_MODE;

bool exampleBool = false;
int exampleInt = 2;


// ************************Function Declaration ************************ //
bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y);
bool callback_mouse_down(Viewer &viewer, int button, int modifier);
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier);
bool callback_pre_draw(Viewer &viewer);
bool callback_init(Viewer &viewer);
bool load_mesh(const std::string &filename);
// ******************************************************************** //


bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y) {

	viewer.mouse_mode = igl::opengl::glfw::Viewer::MouseMode::Translation;
	
	return false;

}

bool callback_mouse_down(Viewer &viewer, int button, int modifier) {
	
	if (pipelineMode == LANDMARK_MODE) {
		// Passing on mouse down event to landmark manager
		landmarkManager->callback_mouse_down(viewer, button, modifier);
	}

	return false;

}

bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier) {
	
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

	if (pipelineMode == LANDMARK_MODE) {
		// Passing on key pressed event to landmark manager
		landmarkManager->callback_key_pressed(viewer, key, modifier);
	}
	else if (pipelineMode == FACE_ALIGNMENT_MODE) {
		// Passing on key pressed event to face alignment manager
		faceAlignmentManager->callback_key_pressed(viewer, key, modifier);
	}

	return false;

}

bool callback_pre_draw(Viewer &viewer) {

	//viewer.data().clear();
	//viewer.data().set_mesh(V, F);

	if (pipelineMode == LANDMARK_MODE) {
		// Pre-drawing landmark points and labels
		landmarkManager->callback_pre_draw(viewer);
	} else {
		// Removing all points and labels from viewer data
		// NOTE: Hacky way, but this prevents having to call clean() and thus set_mesh() every time
		viewer.data().set_points(Eigen::MatrixXd(0, 3), Eigen::MatrixXd(0, 3));
		viewer.data().labels_positions.resize(0, 3);
		viewer.data().labels_strings.clear();
	}

	return false; 

}

bool callback_init(Viewer &viewer) {

	return false;

}

bool load_mesh(const std::string &filename) {

	// Reading triangle from file (multiple mesh types possible)
	igl::read_triangle_mesh(filename, V, F);

	// Clearing viewer data, setting mesh and aligning camera
	viewer.data().clear();
	viewer.data().set_mesh(V, F);
	viewer.core.align_camera_center(V);

	// Initializing landmark manager
	landmarkManager = std::unique_ptr<LandmarkManager>(new LandmarkManager(V, F, viewer));

	return true;

}

int main(int argc,char *argv[]) {

	if (argc != 2) {
		std::cout << "Usage ./assignment6 <mesh.off/obj>" << std::endl;
		load_mesh("../data/aligned_faces_example/example1/fabian-brille.objaligned.obj");
	} else {
		load_mesh(argv[1]);
	}

	// Initialize face alignment manager
	faceAlignmentManager = std::unique_ptr<FaceAlignmentManager>(new FaceAlignmentManager(viewer));

	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]() {

		// Draw parent menu content
		menu.draw_viewer_menu();

		// Useful to check ImGui stuff
		//ImGui::ShowDemoWindow();

		if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {

			const char* mode[] = {"Landmark", "Face Alignment", "PCA", "Bonus"};
            int index = pipelineMode;
            if (ImGui::Combo("Pipeline Mode", &index, mode, IM_ARRAYSIZE(mode))) {
                // Updating pipeline mode
                pipelineMode = static_cast<enum PipelineMode>(index);
            }

			ImGui::Checkbox("Example Boolean", &exampleBool);
			ImGui::InputInt("Example Integer", &exampleInt);
			
		}

		if (pipelineMode == LANDMARK_MODE) {
			// Drawing custom landmark window menu
			landmarkManager->callback_draw_viewer_menu();
		}
		else if (pipelineMode == FACE_ALIGNMENT_MODE) {
			faceAlignmentManager->callback_draw_viewer_menu();
		}

	};

	viewer.callback_key_pressed = callback_key_pressed;
	//viewer.callback_mouse_move = callback_mouse_move;
	viewer.callback_mouse_down = callback_mouse_down;
	viewer.callback_pre_draw = callback_pre_draw;
	//viewer.callback_init = callback_init;

	viewer.launch();

}
