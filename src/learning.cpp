#include "learning.h"

#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

#include <sstream>
#include <cstdlib>


LearningManager::LearningManager(const Viewer &viewer_)
    : viewer(viewer_), sliderWeights(latentDimension, 0.0f) {

}

LearningManager::~LearningManager() {

}

void LearningManager::callback_draw_viewer_menu() {

    ImGui::SetNextWindowPos(ImVec2(180.0f * 1.0f, 0.0f), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200.0f, 500.0f), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Learning Window", nullptr);

    if (ImGui::CollapsingHeader("Latent Weights", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        static float sliderMin = -10.0f;
        static float sliderMax = 10.0f;

        float sliderMinMax[] = {sliderMin, sliderMax};
        if (ImGui::InputFloat2("Slider Min/Max##Learning", sliderMinMax, "%.1f")) {

            // Checking that min is smaller than max
            if (sliderMinMax[0] < sliderMinMax[1]) {
                
                // Updating slider min and max
                sliderMin = sliderMinMax[0];
                sliderMax = sliderMinMax[1];

                // Iterating through slider weights and clamping them in interval [min, max] if necessary
                for (int i = 0; i < sliderWeights.size(); ++i) {
                    sliderWeights[i] = std::min(std::max(sliderMin, sliderWeights[i]), sliderMax); 
                }

            }

        }

        // Iterating and drawing all slider weights
        for (int i = 0; i < sliderWeights.size(); ++i) {
            std::string weightName = "Latent " + std::to_string(i + 1);
            ImGui::PushID(i);
            ImGui::SliderFloat(weightName.c_str(), &sliderWeights[i], sliderMin, sliderMax);
            ImGui::PopID();
        }

        if (ImGui::Button("Reset Weights##Learning")) {
            // Resetting slider weights
            for (int i = 0; i < sliderWeights.size(); ++i){
                sliderWeights[i] = 0.0f;
            }
        }

    }

    if (ImGui::CollapsingHeader("Python Script", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::InputText("Directory##Learning", learningDirectory);

        ImGui::InputText("Script##Learning", pythonCommand);

        ImGui::InputText("Output##Learning", outputMesh);

        // Constructing system command
        std::string systemCommand;
        {
            // Constructing argument string
            std::stringstream argumentStream;
            for (float w : sliderWeights) {
                argumentStream << " " << w;
            }
            // Constructing system command string 
            std::stringstream systemStream;
            systemStream << "cd " << learningDirectory << " && " << pythonCommand << argumentStream.str();
            systemCommand = systemStream.str();
        }

        ImGui::PushItemWidth(-40);
        ImGui::PushID("System Command##Learning");
        ImGui::InputText("", systemCommand, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button("Run##Learning", ImVec2(-1.0f, 20.0f))) {
            
            // Calling system command that changes directory and calls python script
            std::system(systemCommand.c_str());

            // Setting flag to indicate main that mesh should be updated to location
            updateMesh = true;

        }

    }

    ImGui::End();

}

bool LearningManager::callback_key_pressed(Viewer &viewer, unsigned char key, int modifier) {

    switch (key) {
        case 'r': 
            // Resetting slider weights
            for (int i = 0; i < sliderWeights.size(); ++i){
                sliderWeights[i] = 0.0f;
            }  
            break;
        case 'p': 
            // Constructing argument string
            std::stringstream arguments;
            for (float w : sliderWeights) {
                arguments << " " << w;
            }

            std::stringstream systemCommand;
            systemCommand << "cd " << learningDirectory << " && " << pythonCommand << arguments.str();

            // Calling system command that changes directory and calls python script
            std::system(systemCommand.str().c_str());

            // Setting flag to indicate main that mesh should be updated to location
            updateMesh = true;
            break;
    }

    return false;

}

