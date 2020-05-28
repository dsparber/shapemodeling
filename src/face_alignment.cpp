#include "face_alignment.h"

#include "rigid_registration.h"

#include <imgui/imgui.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>

#include <igl/file_dialog_open.h>

#include <cmath>

FaceAlignmentManager::FaceAlignmentManager() {

}

FaceAlignmentManager::~FaceAlignmentManager() {

}

void FaceAlignmentManager::callback_draw_viewer_menu() {
    
    ImGui::SetNextWindowPos(ImVec2(180.0f * 1.0f, 0.0f), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200.0f, 600.0f), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Face Alignment", nullptr);
    
    if (ImGui::CollapsingHeader("Rigid Registration", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        ImGui::PushItemWidth(-90.0f);
        ImGui::InputText("Temp. mesh##FaceAlignment", path_template);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(-90.0f);
        ImGui::InputText("Temp. landmarks##FaceAlignment", path_landmarks_template);
        ImGui::PopItemWidth();

        if (ImGui::Button("Browse scan mesh##FaceAlignment", ImVec2(-1.0f, 0.0f))) {
            path_scan = igl::file_dialog_open();

            if (path_scan.length() > 0) {
                std::cout << "Successfully set the scan mesh path to: " << path_scan << std::endl;
            } else {
                std::cout << "The provided path was empty, please repeat." << std::endl;
            }
        }

        if (ImGui::Button("Browse scan landmarks##FaceAlignment", ImVec2(-1.0f, 0.0f))) {
            path_landmarks_scan = igl::file_dialog_open();

            if (path_landmarks_scan.length() > 0) {
                std::cout << "Successfully set scan landmarks path to: " << path_landmarks_scan << std::endl;
            } else {
                std::cout << "The provided path was empty, please repeat." << std::endl;
            }
        }

        if (ImGui::Button("Rigidly align##FaceAlignment", ImVec2(-1.0f, 0.0f))) {
            if (path_scan.length() > 0 && path_landmarks_scan.length() > 0) {
                rigidly_align();
            } else {
                std::cout << "Rigid alignment failed: One of the provided paths was empty." << std::endl;
            }
        }
    }

    if (ImGui::CollapsingHeader("Warping", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        ImGui::PushItemWidth(-90.0f);
        if (ImGui::InputDouble("Lambda##FaceAlignment", &lambda, 0.0, 0.0, "%.4lf")) {
            // Setting lambda to be positive
            lambda = std::abs(lambda);
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(-90.0f);
        if (ImGui::InputInt("Iterations##FaceAlignment", &iterations)) {
            // Setting iterations to zero if user input is negative
            iterations = std::max(0, iterations);
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(-90.0f);
        if (ImGui::InputDouble("Dist. threshold##FaceAlignment", &relative_distance_threshold, 0.0, 0.0, "%.4lf")) {
            // Setting relative distance to be positive
            relative_distance_threshold = std::abs(relative_distance_threshold);
        }
        ImGui::PopItemWidth();

        ImGui::Checkbox("Use landmark constraints##FaceAlignment", &use_landmark_constraints);

        if (ImGui::Button("Warp##FaceAlignment", ImVec2(-1.0f, 0.0f))) {
            warp();
        }

        ImGui::PushItemWidth(-40);
        ImGui::PushID("Result Path##FaceAlignment");
        ImGui::InputText("", result_path);
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Save##FaceAlignment", ImVec2(-1.0f, 0.0f))) {
            if (warping == nullptr) {
                std::cout << "Cannot store mesh before rigidly aligning and warping." << std::endl;
            } else {
                warping->store_last_result_to_obj_file(std::string(result_path));
            }
        }

    }

    ImGui::End();
}

bool FaceAlignmentManager::callback_key_pressed(Viewer& viewer, unsigned char key, int modifier) {
    switch (key) {
    case '1':
        // Show scanned face (rigidly aligned)
        if (V_scan.size() > 0 && F_scan.size() > 0) {
            mesh_to_show = 1;
        }
        break;
    case '2':
        // Show template (rigidly aligned)
        if (V_template.size() > 0 && V_template.size() > 0) {
            mesh_to_show = 2;
        }
        break;
    case '3':
        // Show warped mesh
        if (V_warped.size() > 0 && F_warped.size() > 0) {
            mesh_to_show = 3;
        }
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
    mesh_to_show = 1;
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
    mesh_to_show = 3;
}