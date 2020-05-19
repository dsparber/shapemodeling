#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

/*** insert any necessary libigl headers here ***/
#include <math.h> 
#ifndef _WIN32
#include <unistd.h>
#endif
#include "../src/pca.h"

using namespace std;

using Viewer = igl::opengl::glfw::Viewer;

Viewer viewer;

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

bool exampleBool = false;
int exampleInt = 2;

int PRINCIPLE_COMPONENTS = 9;
string dirPath = "../data/aligned_faces_example/example4/";
string templatePath = "/home/viviane/FS2020/shape/sm-assignment6/data/face_template/headtemplate_noneck_lesshead_4k.obj";
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

bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers){
	switch (key) {
		case '1':
			compute_face();
			break;
		case '2' :
			break;
		case '3':
			break;
		case '4':
			break;
		case 'V':	
		// recompute PCA and set mean face
			compute_pca("../data/aligned_faces_example/example4/", 9, F_m, W);
			reshape(F_m, F_m.size()/3, 3, V_new);
			set_V(V_new);
			break;
		case 'R':
		// Reset weights
			for(auto &e : slider_weights){
				e = 0.0f;
			}
			break;	
	}

	return true;
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


int main(int argc,char *argv[]){
	if(argc != 2){
		cout << "Usage ./pca path/to/aligned_faces/" << endl;
	}else{
		dirPath = argv[1];
	}

#ifndef _WIN32
	int opt;
	while((opt = getopt(argc, argv, "m:")) != -1){
		switch(opt){
			case 'm':
				PRINCIPLE_COMPONENTS = atoi(optarg);
				break;
		}
	}
#endif
	// set weights to zero
	slider_weights = vector<float>(PRINCIPLE_COMPONENTS, 0);
	//set F from a template file
	igl::read_triangle_mesh(templatePath, V, F);
	compute_pca(dirPath, PRINCIPLE_COMPONENTS, F_m, W);
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
			for(int i = 0; i < PRINCIPLE_COMPONENTS; i++){
				string weight_name = "Eigenvector " + to_string(i);
				ImGui::SliderFloat(weight_name.c_str(), &(slider_weights[i]), -1, 1);
			}
		}
	};

	viewer.callback_key_pressed = callback_key_pressed;
	viewer.launch();
}
