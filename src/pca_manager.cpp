#include "pca_manager.h"

#include "common.h"

#include <igl/writeOBJ.h>
#include <igl/read_triangle_mesh.h>

#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

PCAManager::PCAManager(const Viewer& viewer_, bool small_, string data_path, int m_) 
    :viewer(viewer_), small(small_), data_path(data_path), m(m_)
{
    if(small){
        template_path = s_template_path;
        n = 2319;
    }else{
        template_path = l_template_path;
        n = 22779;
        f1_path = "../data/pca/large/person1.obj";
        f2_path = "../data/pca/large/person2.obj";
        happy_path = "../data/pca/large/happy.obj";
        neutral_path = "../data/pca/small/neutral.obj";
    }

    igl::read_triangle_mesh(template_path, this->V, this->F);
    load_faces();
    cout << "Found " << nFiles << " faces.\n";

    // expression 
    Eigen::VectorXd happy, neutral;
    read_face(happy_path, happy);
    read_face(neutral_path, neutral);
    expression = happy - neutral;

    // general
    slider = Eigen::VectorXd::Zero(m);
    compute_pca();
    base = mF;

    // morphing
    read_face(f1_path, f1);
    read_face(f2_path, f2);
    compute_weights(f1, w1);
    compute_weights(f2, w2);

}

PCAManager::~PCAManager(){/* empty? */}

void PCAManager::callback_draw_viewer_menu()
{

    ImGui::SetNextWindowPos(ImVec2(180.0f * SCREEN_SCALE_X, 0.0f), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(DEFAULT_WINDOW_SIZE, ImGuiSetCond_FirstUseEver);
    ImGui::Begin("PCA", nullptr);

    if (ImGui::CollapsingHeader("Eigenfaces", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for(int i = 0; i < m; i++){
            string slider_name = "EV " + to_string(i + 1);
            if(ImGui::SliderScalar(slider_name.c_str(), ImGuiDataType_Double, &(slider[m-(i+1)]), &mone, &one, "%.3lf")){
                this->draw = true;
            }
        }
        if(this->draw){
            eigenface(weight_factor * slider.array() * eigenvalues.array(), base);
        }
        
        if(ImGui::InputDouble("Scale##PCA", &weight_factor, 0.0, 0.0, "%.3lf")){
            weight_factor = std::abs(weight_factor);
        }

        ImGui::SliderScalar("Std. Dev.##PCA", ImGuiDataType_Double, &random_variance, &zero, &max_random_variance, "%.3lf");

        float w = ImGui::GetContentRegionAvailWidth();
        float p = ImGui::GetStyle().FramePadding.x;

        if(ImGui::Button("Randomize##PCA", ImVec2(0.5f * (w-p), 0.0f))){
            random_face();
            this->draw = true;	
        }

        ImGui::SameLine(0.0f, p);

        if(ImGui::Button("Reset##PCA",  ImVec2(0.5f * (w-p), 0.0f))){
            slider = Eigen::VectorXd::Zero(m);
            eigenface(slider, base);
            this->draw = true;		
        }

        if(ImGui::Button("Choose Base##PCA", ImVec2(-1.0f, 0.0f))){
            string filename = igl::file_dialog_open();
            read_face(filename, base);
            igl::read_triangle_mesh(filename, this->V, this->F);
            this->draw = true;
        }
        
        if(ImGui::Button("Reconstruct Base##PCA", ImVec2(-1.0f, 0.0f))){
            reconstruct(base);
            this->draw = true;
        }

    }

    if (ImGui::CollapsingHeader("Face Morphing", ImGuiTreeNodeFlags_DefaultOpen))
    {

        float w = ImGui::GetContentRegionAvailWidth();
        float p = ImGui::GetStyle().FramePadding.x;

        ImGui::PushItemWidth(-0.25f * (w - p));
        ImGui::PushID("Face 1 Text Path##PCA");
        ImGui::InputText("", f1_path);
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button("Face 1##PCA", ImVec2(-1.0f, 0.0f))){
            f1_path = igl::file_dialog_open();
            if (f1_path.length() > 0) {
                read_face(f1_path, f1);
                compute_weights(f1, w1);
            } else {
                std::cout << "The provided path was empty, please repeat." << std::endl;
            }
        }

        ImGui::PushItemWidth(-0.25f * (w - p));
        ImGui::PushID("Face 2 Text Path##PCA");
        ImGui::InputText("", f2_path);
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button("Face 2##PCA", ImVec2(-1.0f, 0.0f))){
            f2_path = igl::file_dialog_open();
            if (f2_path.length() > 0) {
                read_face(f2_path, f2);
                compute_weights(f2, w2);
            } else {
                std::cout << "The provided path was empty, please repeat." << std::endl;
            }
        }

        if(ImGui::SliderScalar("Morph##PCA", ImGuiDataType_Double, &p12, &zero, &one, "%.3lf")){
            morphface();
            this->draw = true;
        }

    }
    
    if (ImGui::CollapsingHeader("Expression Changer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if(ImGui::SliderScalar("Expression##PCA", ImGuiDataType_Double, &p_exp, &mone, &one, "%.3lf")){
            change_expression(p_exp, base);
            this->draw = true;
        }

    } 
    ImGui::End();
}

bool PCAManager::callback_key_pressed(Viewer& viewer, unsigned char key, int modifier)
{
	switch (key) {
        case 'M':
            reshape(mF, mF.size()/3, 3, V);
            base = mF;
            this->draw = true;
            break;
        case 'R':
            random_face();
            this->draw = true;
            break;
	}
    return true;
}


void PCAManager::load_faces(){
    string file_path;
    Eigen::MatrixXd V_temp;
    Eigen::MatrixXd F_temp;
    string ext = ".obj";
    vector<string> listOfFiles;
    findFilesWithExt(data_path, ext, listOfFiles);
    nFiles = listOfFiles.size();
    Faces.setZero(nFiles, 3*n);
    for(int i = 0; i < nFiles; i++){
        file_path = data_path + "/" + listOfFiles[i];
        cout << listOfFiles[i] << endl;
        igl::read_triangle_mesh(file_path, V_temp, F_temp);
        V_temp.transposeInPlace();
        Faces.row(i) = Eigen::Map<Eigen::RowVectorXd>(V_temp.data(), V_temp.size());
        V_temp.resize(0,3);
    }
}

void PCAManager::compute_pca()
{
    mF = Faces.colwise().mean();
    if(nFiles < m){
        cout << "Not enough faces in database, choose smaller m, n = "<< n << ", m = "<<m<<endl;
        m = nFiles;
    }
        
    auto Faces_centered = Faces.rowwise() - mF.transpose();
    S = Faces_centered * Faces_centered.adjoint();
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigen(S);
    auto v = eigen.eigenvectors();
    Eigen::MatrixXd u = Faces.adjoint() * v; 
    eigenvectors = u.block(0, nFiles - m, 3*n, m); //3nxm matrix of eigenvectors (in cols)
    eigenvectors.colwise().normalize(); // normalize
    eigenvalues = eigen.eigenvalues().segment(nFiles - m, m).colwise().normalized();
}   


void PCAManager::eigenface(const Eigen::VectorXd &slider, Eigen::VectorXd &base){
    Eigen::VectorXd weights;
    weights = slider;
    Eigen::VectorXd face;

    if(weights.hasNaN()){
        weights = Eigen::VectorXd::Zero(m);
    }    

    face = base + eigenvectors * weights;
    reshape(face.transpose(), face.size()/3, 3, this->V);
}


void PCAManager::random_face(){
    std::normal_distribution<double> dist(0.0, random_variance);
    Eigen::VectorXd face, random_weights;
    random_weights.resize(m);
    face = mF;
    for(int i = 0; i < m;i++){
        random_weights[i] = dist(eng) / sqrt(eigenvalues[i]);
    }
    eigenface(random_weights, mF);
}

void PCAManager::morphface(){
    Eigen::VectorXd face;
    Eigen::VectorXd weights;
    weights = w1 - p12*(w1 - w2);
    eigenface(weights, mF);
}

void PCAManager::change_expression(double p, Eigen::VectorXd &base){
    Eigen::VectorXd face;
    face = base + p * expression;
    reshape(face.transpose(), face.size()/3, 3, V);
}

void PCAManager::compute_weights(Eigen::VectorXd &face, Eigen::VectorXd &weights){
    weights.setZero(m);
    Eigen::VectorXd diff = face - mF;
    weights = eigenvectors.transpose() * diff;
}

void PCAManager::reconstruct(Eigen::VectorXd &face){
    Eigen::VectorXd weights;
    compute_weights(face, weights);
    eigenface(weights, mF);
}

void PCAManager::read_face(const string filename, Eigen::VectorXd &face){
    Eigen::MatrixXd V_t;
    Eigen::MatrixXi F_t;
    igl::read_triangle_mesh(filename, V_t, F_t);
    V_t.transposeInPlace();
    face = Eigen::Map<Eigen::VectorXd>(V_t.data(), V_t.size());
}
