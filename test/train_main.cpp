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

#include <string>
#include <sstream>

using namespace std;

std::string pythonCommand = "cd ../learning && python decode.py";

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

int LATENT_DIMENSION = 10;
vector<float> slider_weights;

// ************************Function Declaration ************************ //
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers);
bool load_mesh(string filename);
bool set_V(Eigen::MatrixXd &Vnew);
// ******************************************************************** //

bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers){
	switch (key) {
		case '1':
			break;
		case '2' :
			break;
		case '3':
			break;
		case '4':
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

#ifndef _WIN32
	int opt;
	while((opt = getopt(argc, argv, "m:")) != -1){
		switch(opt){
			case 'm':
				LATENT_DIMENSION = atoi(optarg);
				break;
		}
	}
#endif
	// set weights to zero
	slider_weights = vector<float>(LATENT_DIMENSION, 0);
	//set F from a template file
	//igl::read_triangle_mesh(templatePath, V, F);
	//reshape(F_m, F_m.size()/3, 3, V_new);
	//set_V(V_new);

	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]()
	{
		// Draw parent menu content
		menu.draw_viewer_menu();

		if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for(int i = 0; i < LATENT_DIMENSION; i++){
				string weight_name = "Latent " + to_string(i);
				ImGui::SliderFloat(weight_name.c_str(), &(slider_weights[i]), -1, 1);
			}

            if (ImGui::Button("Run")) {
                
				std::stringstream command;
				command << pythonCommand;
				for (float w : slider_weights) {
					command << " " << w;
				}

				system(command.str().c_str());

                load_mesh("../learning/learning_out.obj");

            }
		}
	};

	viewer.callback_key_pressed = callback_key_pressed;
	viewer.launch();
}
