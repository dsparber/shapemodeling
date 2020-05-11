#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

/*** insert any necessary libigl headers here ***/
#include <math.h> 
#include "../src/pca.h"

using namespace std;
using namespace Eigen;

using Viewer = igl::opengl::glfw::Viewer;

Viewer viewer;

// vertex array, #V x3
Eigen::MatrixXd V;
// face array, #F x3
Eigen::MatrixXi F;

bool exampleBool = false;
int exampleInt = 2;


// ************************Function Declaration ************************ //
bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y);
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers);
bool load_mesh(string filename);
bool callback_init(Viewer &viewer);
// ******************************************************************** //


bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y){
	viewer.mouse_mode = igl::opengl::glfw::Viewer::MouseMode::Translation;
}


bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers){
	Eigen::MatrixXd  W;
	Eigen::VectorXd F_m;
	switch (key) {
		case '1':
			compute_pca("../data/aligned_faces_example/example1/", F_m, W);
			break;
		case '2':
			break;
		case '3':
			break;
		case '4':
			break;
	}

}

bool load_mesh(string filename){
	igl::read_triangle_mesh(filename,V,F);
	viewer.data().clear();
	viewer.data().set_mesh(V, F);
	viewer.core.align_camera_center(V);

	return true;
}

bool callback_init(Viewer &viewer){
	return false;
}

int main(int argc,char *argv[]){
	if(argc != 2){
		cout << "Usage ./pca <mesh.off/obj>" << endl;
		load_mesh("../data/aligned_faces_example/example1/fabian-brille.objaligned.obj");
	}else{
		load_mesh(argv[1]);
	}

	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]()
	{
		// Draw parent menu content
		menu.draw_viewer_menu();

		if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Example Boolean", &exampleBool);
			ImGui::InputInt("Example Integer", &exampleInt);
		}
	};

	viewer.callback_key_pressed = callback_key_pressed;
//   viewer.callback_mouse_move = callback_mouse_move;
//   viewer.callback_init = callback_init;
	viewer.launch();
}
