#include "pca.h"
#include "common.h"

PCA::PCA(int m_, string data_path_, string write_path_, bool small)
    :m(m_), data_path(data_path_), write_path(write_path_)
{
    if(small){
        template_path = s_template_path;
        n = 2319;
    }else{
        template_path = l_template_path;
        n = 22779;
    }

    igl::read_triangle_mesh(template_path, V_template, F_template);
    load_faces();
}

void PCA::load_template(){
    this->V = V_template;
    this->F = F_template;
}

void PCA::get_template(Eigen::MatrixXd &V_ext, Eigen::MatrixXi &F_ext){
    V_ext = V_template;
    F_ext = F_template;
}

void PCA::load_faces(){
    string file_path;
    Eigen::MatrixXd V_temp;
    Eigen::MatrixXd F_temp;
    string ext = ".obj";

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

void PCA::compute_pca()
{
    this->load_faces();
    mF = Faces.colwise().mean();
    reshape(mF, mF.size()/3, 3, M);
    if(nFiles < m){
        cout << "Not enough faces in database, choose smaller m, n = "<< n << ", m = "<<m<<endl;
        m = nFiles;
    }
        
    auto Faces_centered = Faces.rowwise() - mF.transpose();
    S = Faces_centered * Faces_centered.adjoint();
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigen(S);
    auto v = eigen.eigenvectors();
    Eigen::MatrixXd u = Faces.adjoint() * v; 
    eigenvectors = u.block(0, nFiles - m, 3*n, m); //dxm matrix of eigenvectors (in cols)
    eigenvalues = eigen.eigenvalues().segment(nFiles - m, m);
}   


void PCA::morph_face(const vector<double> &slider){
    Eigen::VectorXd weights; weights.resize(m);
    Eigen::VectorXd face;

    for(int i = 0; i < m; i++){
		weights(i) = slider[i];
	}

    if(weights.hasNaN()){
        weights = Eigen::VectorXd::Zero(m);
    }    

    face = mF + eigenvectors * weights;

    reshape(face.transpose(), face.size()/3, 3, V);
}
