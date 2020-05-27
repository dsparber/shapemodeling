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


using namespace std;
using namespace Eigen;
using Viewer = igl::opengl::glfw::Viewer;

Viewer viewer;

// landmarks
std::pair<Eigen::VectorXi, Eigen::VectorXi> landmarks;

// vertex array, #V x3
Eigen::MatrixXd V_template;
Eigen::MatrixXd V_scan;
Eigen::MatrixXd V_warped;

// face array, #F x3
Eigen::MatrixXi F_template;
Eigen::MatrixXi F_scan;
Eigen::MatrixXi F_warped;

// warping
std::shared_ptr<Warping> warping;
double lambda = 1;
int iterations = 1;
double relative_distance_threshold = 0.01;
bool use_landmark_constraints = false;
std::string result_path = "../data/landmarks_example/person0_warped.obj";

bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers) {
	switch (key) {
	case '1':
		// Show the scanned face
		viewer.data().clear();
		viewer.data().set_mesh(V_scan, F_scan);
		break;
	case '2':
		// Show the template
		viewer.data().clear();
		viewer.data().set_mesh(V_template, F_template);
		break;
	case '3':
		// Compute warping and show it
		warping->warp(lambda, iterations, relative_distance_threshold, use_landmark_constraints, V_warped, F_warped);
		viewer.data().clear();
		viewer.data().set_mesh(V_warped, F_warped);
		break;
	case '4':
		// Show warping if it already exists
		if (V_warped.size() > 0 && F_warped.size() > 0) {
			viewer.data().clear();
			viewer.data().set_mesh(V_warped, F_warped);
		}
		break;
	case '5':
		// Store result in file
		warping->store_last_result_to_obj_file(result_path);
		break;
	}
	return true;
}

bool load_mesh()
{
	const string filename_template = "../data/landmarks_example/template_rigid_aligned_scaled.obj";
	const string filename_scan = "../data/landmarks_example/person0__rigid_aligned_scaled0p01.obj";
	igl::read_triangle_mesh(filename_template, V_template, F_template);
	igl::read_triangle_mesh(filename_scan, V_scan, F_scan);

	// read in landmarks
	const string landmarks_template = "../data/landmarks_example/headtemplate_23landmarks";
	const string landmarks_scan = "../data/landmarks_example/person0__23landmarks";
	landmarks = register_landmarks(landmarks_template, landmarks_scan);

	// show scanned face in the beginning
	viewer.data().clear();
	viewer.data().set_mesh(V_scan, F_scan);
	viewer.core.align_camera_center(V_scan);
	return true;
}

int main(int argc, char *argv[]) {

	load_mesh();

	// initialize warping
	warping = std::shared_ptr<Warping>(new Warping(V_template, V_scan, F_template, F_scan, landmarks));

	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]()
	{
		// Draw parent menu content
		menu.draw_viewer_menu();

		// Add new group
		if (ImGui::CollapsingHeader("Warping", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Expose variable directly ...
			ImGui::InputDouble("Lambda", &lambda);
			ImGui::InputInt("Iterations", &iterations);
			ImGui::InputDouble("Rel. dist. threshold", &relative_distance_threshold);
			ImGui::Checkbox("Use landmark constraints", &use_landmark_constraints);
			ImGui::InputText("Store in ...", result_path);
		}
	};

	viewer.callback_key_pressed = callback_key_pressed;

	viewer.launch();
}
