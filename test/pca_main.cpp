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
// normals
Eigen::MatrixXd N;

Eigen::MatrixXd  W;
Eigen::VectorXd F_m;
Eigen::MatrixXd V_new;

bool exampleBool = false;
int exampleInt = 2;

#define PRINCIPLE_COMPONENTS 9

float c1 = 0.0;
float c2 = 0.0;
float c3 = 0.0;
float c4 = 0.0;
float c5 = 0.0;
float c6 = 0.0;
float c7 = 0.0;
float c8 = 0.0;
float c9 = 0.0;
// ************************Function Declaration ************************ //
bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y);
bool callback_mouse_up(Viewer& viewer, int button, int modifier);
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers);
bool load_mesh(string filename);
bool callback_init(Viewer &viewer);
bool set_V(Eigen::MatrixXd &Vnew);
// ******************************************************************** //


bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y){
	viewer.mouse_mode = igl::opengl::glfw::Viewer::MouseMode::Translation;
}

void reshape(Eigen::VectorXd flat, int rows, int cols, Eigen::MatrixXd &matrix){
	Eigen::Map<Eigen::MatrixXd> M(flat.data(), cols, rows);
	matrix = Eigen::MatrixXd(M.transpose());
}

void test_eigen(){
	Eigen::MatrixXd A(4, 3);
	A << -1, 2, -3, 4, 5, 6, -7, 8, -9, 10, 11, 12;
	cout << A << endl;
	cout << A.adjoint() << endl;
	cout << A.transpose() << endl;
}

void compute_face(){
	Eigen::VectorXd weights; weights.setZero(W.cols());
	Eigen::VectorXd morphedFace;
	weights(0) = c1;
	weights(1) = c2;
	weights(2) = c3;
	weights(3) = c4;
	weights(4) = c5;
	weights(5) = c6;
	weights(6) = c7;
	weights(7) = c8;
	weights(8) = c9;
	if(weights.nonZeros() != 0)
		weights.normalize();
	
	
	morphedFace = F_m + W * weights; // dx1
	
	reshape(morphedFace.transpose(), morphedFace.size()/3, 3, V_new);
	set_V(V_new);
	
}
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers){
	switch (key) {
		case '1':
			compute_face();
			break;
		case 'V':	
			compute_pca("../data/aligned_faces_example/example4/", 9, F_m, W);
			reshape(F_m, F_m.size()/3, 3, V_new);
			set_V(V_new);
			break;
		case '3':
			cout << W.rows() << " " << W.cols() << endl;
			break;
		case '4':
			test_eigen();
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

bool set_V(Eigen::MatrixXd &Vnew){
	V = Vnew;
	viewer.data().clear();
	viewer.data().set_mesh(V, F);
	viewer.core.align_camera_center(V);
	return true;
}

bool callback_init(Viewer &viewer){
	return false;
}

bool callback_mouse_up(Viewer& viewer, int button, int modifier)
{
	
	return true;
};

int main(int argc,char *argv[]){
	if(argc != 2){
		cout << "Usage ./pca <mesh.off/obj>" << endl;
		load_mesh("../data/aligned_faces_example/example1/fabian-brille.objaligned.obj");
	}else{
		load_mesh(argv[1]);
	}

	compute_pca("../data/aligned_faces_example/example4/", PRINCIPLE_COMPONENTS, F_m, W);
	reshape(F_m, F_m.size()/3, 3, V_new);
	set_V(V_new);

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
			ImGui::SliderFloat("c1", &c1, -1, 1);
			ImGui::SliderFloat("c2", &c2, -1, 1);
			ImGui::SliderFloat("c3", &c3, -1, 1);
			ImGui::SliderFloat("c4", &c4, -1, 1);
			ImGui::SliderFloat("c5", &c5, -1, 1);
			ImGui::SliderFloat("c6", &c6, -1, 1);
			ImGui::SliderFloat("c7", &c7, -1, 1);
			ImGui::SliderFloat("c8", &c8, -1, 1);
			ImGui::SliderFloat("c9", &c9, -1, 1);
		}
	};

	viewer.callback_key_pressed = callback_key_pressed;
//   viewer.callback_mouse_move = callback_mouse_move;
//   viewer.callback_init = callback_init;
	viewer.launch();
}
