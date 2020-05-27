#include "face_alignment.h"

#include "rigid_registration.h"

#include <imgui/imgui.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>

#include <igl/file_dialog_open.h>

FaceAlignmentManager::FaceAlignmentManager(Viewer& viewer) : viewer(viewer) {/* empty? */}

FaceAlignmentManager::~FaceAlignmentManager() {/* empty? */}

void FaceAlignmentManager::callback_draw_viewer_menu() {
    ImGui::SetNextWindowPos(ImVec2(180.0f * 1.0f, 0.0f), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Face Alignment", nullptr);

    if (ImGui::CollapsingHeader("Rigid Registration", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Paths for registration,\nthe order is:\n- template\n- template landmarks\n- scan\n- scan landmarks");

        if (ImGui::Button("Browse")) {
            path_template = igl::file_dialog_open();

            path_landmarks_template = igl::file_dialog_open();

            path_scan = igl::file_dialog_open();

            path_landmarks_scan = igl::file_dialog_open();

            if (path_template.length() > 0 && path_scan.length() > 0 && path_landmarks_template.length() > 0 && path_landmarks_scan.length() > 0)
                std::cout << "Successfully set paths for face alignments." << std::endl;
            else
                std::cout << "One of the provided paths was empty, please repeat." << std::endl;
        }

        if (ImGui::Button("Rigidly align")) {
            if (path_template.length() > 0 && path_scan.length() > 0 && path_landmarks_template.length() > 0 && path_landmarks_scan.length() > 0)
                rigidly_align();
            else
                std::cout << "Rigid alignment failed: One of the provided paths was empty." << std::endl;
        }
    }

    if (ImGui::CollapsingHeader("Warping", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputDouble("Lambda", &lambda);
        ImGui::InputInt("Iterations", &iterations);
        ImGui::InputDouble("Rel. dist. threshold", &relative_distance_threshold);
        ImGui::Checkbox("Use landmark constraints", &use_landmark_constraints);

        if (ImGui::Button("Warp")) {
            warp();
        }

        ImGui::InputText("Result path", result_path);
        if (ImGui::Button("Store warped mesh")) {
            if (warping == nullptr)
                std::cout << "Cannot store mesh before rigidly aligning and warping." << std::endl;
            else
                warping->store_last_result_to_obj_file(std::string(result_path));
        }
    }

    ImGui::End();
}

bool FaceAlignmentManager::callback_key_pressed(Viewer& viewer, unsigned char key, int modifier) {
    switch (key) {
    case '1':
        // Show scanned face (rigidly aligned)
        show_mesh(1);
        break;
    case '2':
        // Show template (rigidly aligned)
        show_mesh(2);
        break;
    case '3':
        // Show warped mesh
        show_mesh(3);
        break;
    }
    return true;
}

void FaceAlignmentManager::rigidly_align() {
    std::pair<Eigen::VectorXi, Eigen::VectorXi> landmarks;
    rigid_registration(
        path_landmarks_template,
        path_landmarks_scan,
        path_template,
        path_scan,
        V_template,
        V_scan,
        F_template,
        F_scan,
        landmarks
    );
    warping = std::unique_ptr<Warping>(new Warping(V_template, V_scan, F_template, F_scan, landmarks));
    show_mesh(1);
    std::cout << "Rigidly aligned, press '1' for the scanned face and '2' for the template." << std::endl;
}

void FaceAlignmentManager::warp() {
    if (warping == nullptr) { // not rigidly aligned yet
        rigidly_align();
    }
    if (V_template.size() == 0 || V_scan.size() == 0) {
        std::cout << "Cannot warp: Rigid alignment failed." << std::endl;
        return;
    }
    warping->warp(lambda, iterations, relative_distance_threshold, use_landmark_constraints, V_warped, F_warped);
    std::cout << "Press '3' for the warped face." << std::endl;
    show_mesh(3);
}

void FaceAlignmentManager::show_mesh(const int mesh_type) {
    viewer.data().clear();
    if (V_scan.size() > 0) {
        viewer.core.align_camera_center(V_scan);
    }
    switch (mesh_type) {
    case 1:
        // Show scanned face (rigidly aligned)
        if (V_scan.size() > 0 && F_scan.size() > 0) {
            viewer.data().set_mesh(V_scan, F_scan);
        }
        break;
    case 2:
        // Show template (rigidly aligned)
        if (V_template.size() > 0 && V_template.size() > 0) {
            viewer.data().set_mesh(V_template, F_template);
        }
        break;
    case 3:
        // Show warped mesh
        if (V_warped.size() > 0 && F_warped.size() > 0) {
            viewer.data().set_mesh(V_warped, F_warped);
        }
        break;
    }
}