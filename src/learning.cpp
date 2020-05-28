#include "learning.h"

#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

#include <sstream>
#include <fstream>
#include <cstdlib>
#include <chrono>


LearningManager::LearningManager(const Viewer &viewer_)
    : viewer(viewer_), 
      sliderWeights(latentDimension, 0.0f), 
      eng((static_cast<uint_fast32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()))) {

    // Updating system commands for calling python encode/decode
    void updateCommands();

}

LearningManager::~LearningManager() {

}

void LearningManager::callback_draw_viewer_menu() {

    ImGui::SetNextWindowPos(ImVec2(180.0f * 1.0f, 0.0f), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200.0f, 600.0f), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Learning Window", nullptr);

    if (ImGui::CollapsingHeader("Morphing", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::InputText("Face 1##Learning", morphFace1);
        ImGui::InputText("Face 2##Learning", morphFace2);

        bool dirty = false;

        if (ImGui::Button("Load Faces##Learning", ImVec2(-1.0f, 20.0f))) {
            
            // Updating system commands
            updateCommands();

            // Calling system command that changes directory and calls python script
            std::system(systemEncodeCommand.c_str());

            // Loading latent vectors from file created by encode python script
            loadLatentVectors(latentFile);

            // Updating flag
            dirty = true;

        }

        static float morphWeight = 0.5f;
        if (ImGui::SliderFloat("Morph", &morphWeight, 0.0f, 1.0f) || dirty) {
            if (sliderWeights.size() == morphFace1Latents.size()
                && sliderWeights.size() == morphFace2Latents.size()) {
                for (int i = 0; i < morphFace1Latents.size(); ++i) {
                    sliderWeights[i] = (1.0f - morphWeight) * morphFace1Latents[i] + morphWeight * morphFace2Latents[i];
                }
            } else {
                std::cerr << "Error: No morph faces loaded" << std::endl;
            }

        }

    }

    if (ImGui::CollapsingHeader("Latent Weights", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        static float sliderMin = -2.5f;
        static float sliderMax = 2.5f;

        float sliderMinMax[] = {sliderMin, sliderMax};
        if (ImGui::InputFloat2("Min/Max##Learning", sliderMinMax, "%.1f")) {

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

        if (ImGui::Button("Randomize##Learning", ImVec2(87.5f, 20.0f))) {
            
            // Setting standard deviation as 0.25 of min/max interval
            float standardDeviation = 0.25f * (sliderMax - sliderMin);

            // Initializing normal distribution object
            std::normal_distribution<float> dist(0.0f, standardDeviation);

            // Iterating through slider weights and randomly setting them in interval [min, max]
            for (int i = 0; i < sliderWeights.size(); ++i) {
                sliderWeights[i] = dist(eng);
            }

        }

        ImGui::SameLine();

        if (ImGui::Button("Reset##Learning", ImVec2(87.5f, 20.0f))) {
            // Resetting slider weights
            for (int i = 0; i < sliderWeights.size(); ++i){
                sliderWeights[i] = 0.0f;
            }
        }

        if (ImGui::Button("Apply##Learning", ImVec2(182.5f, 20.0f))) {
            
            // Updating system commands
            updateCommands();

            // Calling system command that changes directory and calls python script
            std::system(systemDecodeCommand.c_str());

            // Setting flag to indicate main that mesh should be updated to location
            updateMesh = true;

        }

    }

    if (ImGui::CollapsingHeader("Advanced Setting", ImGuiTreeNodeFlags_None)) {

        const float textWidth = 100.0f;

        ImGui::PushItemWidth(textWidth);
        ImGui::InputText("Directory##Learning", learningDirectory);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(textWidth);
        ImGui::InputText("Latents##Learning", latentFile);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(textWidth);
        ImGui::InputText("Mesh##Learning", outputMesh);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(textWidth);
        ImGui::InputText("Encode Py##Learning", pythonEncodeCommand);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(textWidth);
        ImGui::InputText("Decode Py##Learning", pythonDecodeCommand);
        ImGui::PopItemWidth();

        // Updating system commands
        updateCommands();

        ImGui::PushItemWidth(textWidth);
        ImGui::InputText("Encode Sys##Learning", systemEncodeCommand, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(textWidth);
        ImGui::InputText("Decode Sys##Learning", systemDecodeCommand, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

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
            // Updating system commands
            updateCommands();

            // Calling system decode command that changes directory and calls python decode script
            std::system(systemDecodeCommand.c_str());

            // Setting flag to indicate main that mesh should be updated to location
            updateMesh = true;
            break;
    }

    return false;

}

void LearningManager::loadLatentVectors(const std::string &fileName) {

    // Cleaning morph face latent vectors
    morphFace1Latents.clear();
    morphFace2Latents.clear();

    std::ifstream fs(fileName);
    if (fs.fail()) {
        std::cerr << "Could not open latent vector file: " << fileName << std::endl;
        return;
    }

    std::string line;
    if (getline(fs, line)) {

        // Initializing input string stream with line string
        std::istringstream ls(line);

        for (int i = 0; i < latentDimension; ++i) {
            // "%f "
            float w;   
            ls >> w;
            morphFace1Latents.push_back(w);
        }

    }

    if (getline(fs, line)) {

        // Initializing input string stream with line string
        std::istringstream ls(line);

        for (int i = 0; i < latentDimension; ++i) {
            // "%f "
            float w;   
            ls >> w;
            morphFace2Latents.push_back(w);
        }

    }

    fs.close();

} 

void LearningManager::updateCommands() {
        
    // Constructing system encode command
    {
        // Constructing argument string
        std::stringstream argumentPathsStream;
        argumentPathsStream << " " << morphFace1 << " " << morphFace2;

        // Constructing system encode command string 
        std::stringstream systemEncodeStream;
        systemEncodeStream << "cd " << learningDirectory << " && " << pythonEncodeCommand << argumentPathsStream.str();
        systemEncodeCommand = systemEncodeStream.str();
    }

    // Constructing system decode command
    {
        // Constructing argument string
        std::stringstream argumentWeightsStream;
        for (float w : sliderWeights) {
            argumentWeightsStream << " " << w;
        }
        // Constructing system encode command string 
        std::stringstream systemDecodeStream;
        systemDecodeStream << "cd " << learningDirectory << " && " << pythonDecodeCommand << argumentWeightsStream.str();
        systemDecodeCommand = systemDecodeStream.str();
    }

}
