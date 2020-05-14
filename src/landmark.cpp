#include "landmark.h"

#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

#include <igl/unproject_onto_mesh.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <algorithm>

LandmarkManager::LandmarkManager(const Eigen::MatrixXd &V_, const Eigen::MatrixXi &F_, const Viewer &viewer_)
    : V(V_), F(F_), viewer(viewer_) {
    
    // Filling landmarks queue
    for (auto &e : descriptions) {
        landmarks_queue.push_back(e.first);
    }

    // Pushing first landmark off queue and setting it as current selection landmark
    if (!landmarks_queue.empty()) {
        current_landmark = landmarks_queue.front();
        landmarks_queue.pop_front();
    }

}

LandmarkManager::~LandmarkManager() {

}

int LandmarkManager::pick_landmark_vertex(int mouse_x, int mouse_y) const {

    int vi = -1;

    // Initializing return arguments for face id and barycentric vector
    int fid;
    Eigen::Vector3f bc;
    
    // Casting a ray in the view direction starting from the mouse position
    double x = viewer.current_mouse_x;
    double y = viewer.core.viewport(3) - viewer.current_mouse_y;

    // Checkign if a ray from the camera through the mouse positions x and y intersect with the mesh
    if (igl::unproject_onto_mesh(Eigen::Vector2f(x,y), viewer.core.view /* viewer.data().model*/,
                                 viewer.core.proj, viewer.core.viewport, V, F, fid, bc)) {
        
        // Getting baricentric maximum index
        bc.maxCoeff(&vi);
        // Setting vertex id from maximum baricentric index of face
        vi = F(fid,vi);

    }

    return vi;

}

void LandmarkManager::callback_draw_viewer_menu() {

    ImGui::SetNextWindowPos(ImVec2(180.0f * 1.0f, 0.0f), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200.0f, 500.0f), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Landmark Window", nullptr);

    if (ImGui::CollapsingHeader("Load/Save", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::PushItemWidth(-40);
        ImGui::PushID("Input File##Landmark");

        ImGui::InputText("", inputFile);
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Load##Landmark", ImVec2(-1.0f, 20.0f))) {
            // Reading landmark values from input file path
            read_from_file(inputFile);
            std::cout << "Loaded landmarks from " << inputFile << std::endl;
        }

        ImGui::PushItemWidth(-40);
        ImGui::PushID("Output File##Landmark");
        ImGui::InputText("", outputFile);
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Save##Landmark", ImVec2(-1.0f, 20.0f))) {
            // Writing landmark values to output file
            write_to_file(outputFile);
            std::cout << "Saved landmarks to " << outputFile << std::endl;
        }

    }

    if (ImGui::CollapsingHeader("Instructions", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (descriptions.find(current_landmark) != descriptions.end()) {

            ImGui::Text("Please click:");

            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 180.0f);
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", descriptions[current_landmark].c_str());
            ImGui::PopTextWrapPos();

            ImGui::Text("Press [space] to confirm selection");

        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No instruction for landmark id %d", current_landmark);
        }
        
    }

    if (ImGui::CollapsingHeader("List", ImGuiTreeNodeFlags_DefaultOpen)) {

        int n = landmarks.size();

        // Initializing vector for later deletion of potential landmarks
        std::vector<int> landmarks_deleted;

        // Iterating through all set landmarks
        int i = 0;
        for (auto &e : landmarks) {
            
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            
            ImGui::BeginChild(std::to_string(i).c_str(), ImVec2(0.0f, 58.0f), true, ImGuiWindowFlags_NoScrollWithMouse/* | ImGuiWindowFlags_MenuBar*/);
            ImGui::Columns(3, NULL, false);
            {
                {
                    ImGui::PushID(i + n);
                    ImGui::Text("Landmark");
                    ImGui::PopID();
                    
                    ImGui::NextColumn();

                    int landmark_id = e.first;
                    ImGui::PushItemWidth(-1.0f);
                    ImGui::PushID(i + 2*n);
                    ImGui::InputInt("", &landmark_id, 0, 0, ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopID();
                    ImGui::PopItemWidth();

                    ImGui::NextColumn();
                    ImGui::PushID(i + 3*n);
                    ImGui::Button("Name", ImVec2(-1.0f, 0.0f));
                    if (ImGui::IsItemHovered()) {
                        
                        std::string tooltip;
                        if (descriptions.find(landmark_id) != descriptions.end()) {
                            tooltip = descriptions[landmark_id];
                        } else {
                            tooltip = "invalid";
                        }
                        ImGui::SetTooltip("%s", tooltip.c_str());
                    }
                    ImGui::PopID();
                }
                ImGui::NextColumn();
                {
                    ImGui::PushID(i + 4*n);
                    ImGui::Text("Vertex");
                    ImGui::PopID();

                    ImGui::NextColumn();

                    int vertex_id = e.second;
                    ImGui::PushItemWidth(-1.0f);
                    ImGui::PushID(i + 5*n);
                    ImGui::InputInt("", &vertex_id, 0, 0, ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopID();
                    ImGui::PopItemWidth();

                    ImGui::NextColumn();

                    ImGui::PushID(i + 6*n);
                    if (ImGui::Button("Delete", ImVec2(-1.0f, 0.0f))) {
                        std::cout << "Deleting landmark " << e.first << std::endl;
                        landmarks_deleted.push_back(e.first);
                    }
                    ImGui::PopID(); 
                }
            }
            ImGui::EndChild();          
            ImGui::PopStyleVar();

            i++;

        }

        // Deleting landmarks for which button was pressed and putting them back into queue
        for (int l : landmarks_deleted) {

            // Remove landmark id from set landmarks
            landmarks.erase(l);

            // Checking if current landmark is valid and if true putting it back on the queue
            if (current_landmark != -1) {
                landmarks_queue.push_front(current_landmark);
            }

            // Setting current landmark to be deleted one
            current_landmark = l;

        }

        // Sorting landmarks queue
        //std::sort(landmarks_queue.begin(), landmarks_queue.begin());

    }

    ImGui::End();

}

bool LandmarkManager::callback_pre_draw(Viewer &viewer) {

    const Eigen::RowVector3d red = Eigen::RowVector3d(1.0, 0.0, 0.0);
    const Eigen::RowVector3d green = Eigen::RowVector3d(0.0, 1.0, 0.0);

    // Removing all points and labels from viewer data
    // NOTE: Hacky way, but this prevents having to call clean() and thus set_mesh() every time
    viewer.data().set_points(Eigen::MatrixXd(0, 3), Eigen::MatrixXd(0, 3));
    viewer.data().labels_positions.resize(0, 3);
    viewer.data().labels_strings.clear();

    // Iterating through all landmarks and adding point and landmark label for drawing
    for (const auto &e : landmarks) {

        int vi = e.second;
        
        // Checking if index is in bounds
        if (vi < V.rows()) {
            viewer.data().add_points(V.row(vi), red);
            viewer.data().add_label(V.row(vi), std::to_string(e.first));
        }

    }

    // If valid adding selected point and landmark label for drawing
    if (current_landmark != -1 && current_vertex != -1) {

        // Checking if index is in bounds
        if (current_vertex < V.rows()) {
            viewer.data().add_points(V.row(current_vertex), green);
            viewer.data().add_label(V.row(current_vertex), std::to_string(current_landmark));
        }

    }

    return false;

}

/*
bool LandmarkManager::callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y) {
    return false;
}
*/

bool LandmarkManager::callback_mouse_down(Viewer &viewer, int button, int modifier) {
    
    if (button == (int) Viewer::MouseButton::Right) {
        return false;
    }

    down_mouse_x = viewer.current_mouse_x;
    down_mouse_y = viewer.current_mouse_y;

    // Setting current vertex index to be closest mesh vertex to mouse position
    current_vertex = pick_landmark_vertex(down_mouse_x, down_mouse_y);

    return false;

}

bool LandmarkManager::callback_key_pressed(Viewer &viewer, unsigned char key, int modifier) {

    if (key == ' ') {

        // Checking if current landmark and vertex selections are valid
        if (current_landmark == -1 || current_vertex == -1) {
            std::cerr << "No valid landmark selection" << std::endl;
            return false;
        }

        // Checking if current landmark id is already exists and selection is thus invalid
        if (landmarks.find(current_landmark) != landmarks.end()) {
            std::cerr << "Landmark already exists" << std::endl;
            return false;
        }

        // Confirming selection and adding (landmark, vertex) pair to landmarks map
        landmarks.insert(std::pair<int,int>(current_landmark, current_vertex));
        
        // Checking if landmark queue is empty or new current landmark can be set
        if (!landmarks_queue.empty()) {
            current_landmark = landmarks_queue.front();
            landmarks_queue.pop_front();
        } else {
            current_landmark = -1;
        }

        // Unsetting current vertex index
        current_vertex = -1;

    }

    return false;

}

void LandmarkManager::read_from_file(const std::string &path) {

    // Cleaning landmarks map and landmarks queue
    landmarks.clear();
    landmarks_queue.clear();

    // Invalidating current landmarks and vertex selection
    current_landmark = -1;
    current_vertex = -1;

    std::ifstream fs(path);
    if (fs.fail()) {
        std::cerr << "Could not open landmark file: " << path << std::endl;
        return;
    }

    std::string line;
    while (getline(fs, line)) {

        // Initializing input string stream with line string
        std::istringstream ls(line);

        // "%d %d"
        int vertex_index;
        int landmark_index;
        
        // Parsing line to get vertex and landmark index
        ls >> vertex_index;
        ls >> landmark_index;

        // Inserting landmark and vertex into landmark map
        landmarks.insert(std::pair<int,int>(landmark_index, vertex_index));

    }

    fs.close();

    // Iterating through all description landmark ids and adding the ones to landmarks queue that are not lind landmarks map
    for (auto &e : descriptions) {
        // Checking if description landmark id is in landmarks map and if not putting it into landmarks queue
        if (landmarks.find(e.first) == landmarks.end()) {
            landmarks_queue.push_back(e.first);
        }
    }

    // Setting it as current selection landmark as first landmarks queue entry or invalid value
    if (!landmarks_queue.empty()) {
        current_landmark = landmarks_queue.front();
        landmarks_queue.pop_front();
    } else {
        current_landmark = -1;
    }

}

void LandmarkManager::write_to_file(const std::string &path) {

    std::ofstream fs(path);
    if (fs.fail()) {
        std::cerr << "Could not open landmark file: " << path << std::endl;
        return;
    }

    // Iterating through all landmarks
    for (auto &e : landmarks) {
        fs << e.second << " " << e.first << std::endl;
    }

    fs.close();

}
