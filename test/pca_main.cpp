#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

/*** insert any necessary libigl headers here ***/
#include <math.h> 
#include <map>
#include <vector>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "../src/pca.h"
#include "../src/pca_class.h"
using namespace std;

using Viewer = igl::opengl::glfw::Viewer;
using ImGuiMenu = igl::opengl::glfw::imgui::ImGuiMenu;
Viewer viewer;

std::unique_ptr<PCA>  pca;

// vertex array, #V x3
Eigen::MatrixXd V;
// face array, #F x3
Eigen::MatrixXi F;
// normals
Eigen::MatrixXd N;
// eigenvectors
Eigen::MatrixXd  W;
// mean face
Eigen::VectorXd F_m;
// place holder
Eigen::MatrixXd V_new;
// eigenvalues
Eigen::VectorXd EV;

bool exampleBool = false;
int exampleInt = 2;

int PRINCIPLE_COMPONENTS = 9;
string dirPath = "../data/aligned_faces_example/default";
string templatePath = "../data/face_template/template_small.obj";
vector<float> slider_weights;

// ************************Function Declaration ************************ //
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers);
bool load_mesh(string filename);
bool set_V(Eigen::MatrixXd &Vnew);
void reshape(Eigen::VectorXd flat, int rows, int cols, Eigen::MatrixXd &matrix);
void compute_face();
// ******************************************************************** //


void reshape(Eigen::VectorXd flat, int rows, int cols, Eigen::MatrixXd &matrix){
	Eigen::Map<Eigen::MatrixXd> M(flat.data(), cols, rows);
	matrix = Eigen::MatrixXd(M.transpose());
}

void compute_face(){
	Eigen::VectorXd weights; weights.setZero(W.cols());
	Eigen::VectorXd morphedFace;
	for(int i = 0; i < PRINCIPLE_COMPONENTS; i++){
		weights(i) = slider_weights[i];
	}
	if(weights.nonZeros() != 0)
		weights.normalize();
	
	morphedFace = F_m + W * weights; // dx1
	reshape(morphedFace.transpose(), morphedFace.size()/3, 3, V_new);
	set_V(V_new);
	
}

void callback_draw_viewer_menu()
{
	pca->callback_draw_viewer_menu();
	
}
bool callback_pre_draw(Viewer &viewer)
{
	pca->callback_pre_draw(viewer);
}

bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifier)
{
	pca->callback_key_pressed(viewer, key, modifier);
}

bool callback_mouse_down(Viewer &viewer, int button, int modifier)
{
	pca->callback_mouse_down(viewer, button, modifier);
}


bool callback_mouse_up(Viewer &viewer, int button, int modifier)
{
	pca->callback_mouse_up(viewer, button, modifier);
}

bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y)
{
	pca->callback_mouse_move(viewer, mouse_x, mouse_y);
}


// bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers){
// 	switch (key) {
// 		case '1':
// 			compute_face();
// 			break;
// 		case '2' :
// 			break;
// 		case '3':
// 			break;
// 		case '4':
// 			break;
// 		case 'V':	
// 		// recompute PCA and set mean face
// 			compute_pca("../data/aligned_faces_example/example4/", 9, F_m, W, EV);
// 			reshape(F_m, F_m.size()/3, 3, V_new);
// 			set_V(V_new);
// 			break;
// 		case 'R':
// 		// Reset weights
// 			for(auto &e : slider_weights){
// 				e = 0.0f;
// 			}
// 			break;	
// 	}

// 	return true;
// }

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


int main(int argc,char *argv[]){
// 	if(argc <= 2){
// 		cout << "Usage ./pca path/to/aligned_faces/" << endl;
// 	}else{
// 		dirPath = argv[1];
// 	}
// 	if(argc == 3){
// 		templatePath = argv[2];
// 	}
// #ifndef _WIN32
// 	int opt;
// 	while((opt = getopt(argc, argv, "m:")) != -1){
// 		switch(opt){
// 			case 'm':
// 				PRINCIPLE_COMPONENTS = atoi(optarg);
// 				break;
// 		}
// 	}
// #endif
// 	// set weights to zero
// 	slider_weights = vector<float>(PRINCIPLE_COMPONENTS, 0);
// 	//set F from a template file
// 	igl::read_triangle_mesh(templatePath, V, F);
// 	compute_pca(dirPath, PRINCIPLE_COMPONENTS, F_m, W, EV);
// 	reshape(F_m, F_m.size()/3, 3, V_new);
// 	set_V(V_new);

// 	igl::opengl::glfw::imgui::ImGuiMenu menu;
// 	viewer.plugins.push_back(&menu);

// 	menu.callback_draw_viewer_menu = [&]()
// 	{
// 		// Draw parent menu content
// 		menu.draw_viewer_menu();

// 		if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
// 		{
// 			ImGui::Checkbox("Example Boolean", &exampleBool);
// 			ImGui::InputInt("Example Integer", &exampleInt);
// 			for(int i = 0; i < PRINCIPLE_COMPONENTS; i++){
// 				string weight_name = "Eigenvector " + to_string(i);
// 				ImGui::SliderFloat(weight_name.c_str(), &(slider_weights[i]), -1, 1);
// 			}
// 		}
// 	};

// 	viewer.callback_key_pressed = callback_key_pressed;

	load_mesh(templatePath);
	map<char, string> inputs;
	inputs['c'] = "hallo";
	ImGuiMenu menu;

	cout << "start constructor"<< endl;
	pca = std::unique_ptr<PCA>(new PCA(viewer));
	cout << "end constructor"<< endl;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]()
	{
		// Draw parent menu content
		menu.draw_viewer_menu();
		// Drawing custom landmark window menu
		pca->callback_draw_viewer_menu();

	};

	viewer.callback_key_pressed = callback_key_pressed;
	viewer.callback_mouse_move = callback_mouse_move;
	viewer.callback_mouse_down = callback_mouse_down;
	viewer.callback_pre_draw = callback_pre_draw;


	viewer.launch();
}
