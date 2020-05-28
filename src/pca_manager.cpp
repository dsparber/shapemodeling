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
        f1_path = "../data/aligned_faces_example/large/person1.obj";
        f2_path = "../data/aligned_faces_example/large/person2.obj";
        happy_path = "../data/aligned_faces_example/large/happy.obj";
        neutral_path = "../data/aligned_faces_example/large/neutral.obj";
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

    ImGui::SetNextWindowPos(ImVec2(180.0f * 1.0f, 0.0f), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("PCA", nullptr);

    if(ImGui::CollapsingHeader("Write to File", ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::InputText("File name", write_file);
        if(ImGui::Button("Write to File")){
            string full_write_path = write_path + write_file;
            bool success = igl::writeOBJ(full_write_path, this->V, this->F);
            if(success){
                cout << "Written to " << full_write_path << endl;
            }
        }
    }

    if (ImGui::CollapsingHeader("Eigenfaces", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for(int i = 0; i < m; i++){
            string slider_name = "Eigenvector " + to_string(i);
            if(ImGui::SliderScalar(slider_name.c_str(), ImGuiDataType_Double, &(slider[m-(i+1)]), &mone, &one)){
                this->draw = true;
            }
        }
        if(this->draw){
            eigenface(weight_factor * slider.array() * eigenvalues.array(), base);
        }
        ImGui::InputDouble("weight factor", &weight_factor);

        if(ImGui::Button("Reset weights")){
            slider = Eigen::VectorXd::Zero(m);
            eigenface(slider, base);
            this->draw = true;		
        }

        if (ImGui::Button("Choose Base"))
        {
            string filename = igl::file_dialog_open();
            read_face(filename, base);
            igl::read_triangle_mesh(filename, this->V, this->F);
            this->draw = true;
        }
        
        if (ImGui::Button("Reconstruct Base"))
        {
            reconstruct(base);
            this->draw = true;
        }

    }

    if (ImGui::CollapsingHeader("Random Face Generator", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderScalar("Random Variance", ImGuiDataType_Double, &random_variance, &zero, &max_random_variance);
    }

    if (ImGui::CollapsingHeader("Face Morphing", ImGuiTreeNodeFlags_DefaultOpen))
    {

        float w = ImGui::GetContentRegionAvailWidth();
        float p = ImGui::GetStyle().FramePadding.x;
        if (ImGui::Button("Face 1", ImVec2((w-p)/2.0f, 0)))
        {
            f1_path = igl::file_dialog_open();
            read_face(f1_path, f1);
            compute_weights(f1, w1);
        }

        if (ImGui::Button("Face 2", ImVec2((w-p)/2.0f, 0)))
        {
            f2_path = igl::file_dialog_open();
            read_face(f2_path, f2);
            compute_weights(f2, w2);
        }

        if(ImGui::SliderScalar("p", ImGuiDataType_Double, &p12, &zero, &one)){
            morphface();
            this->draw = true;
        }

    }
    
    if (ImGui::CollapsingHeader("Expression Changer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::SliderScalar("Expression", ImGuiDataType_Double, &p_exp, &mone, &one))
        {
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

void PCAManager::recompute_pca(){
    if(small){
        template_path = s_template_path;
        n = 2319;
    }else{
        template_path = l_template_path;
        n = 22779;
        f1_path = "../data/aligned_faces_example/large/person1.obj";
        f2_path = "../data/aligned_faces_example/large/person2.obj";
        happy_path = "../data/aligned_faces_example/large/happy.obj";
        neutral_path = "../data/aligned_faces_example/large/neutral.obj";
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