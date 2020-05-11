#include "pca.h"

using namespace std;

bool hasSuffix(const string& s, const string& suffix)
{
    return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());    
}

void findFilesWithExt(const string &dirPath, const string &ext, vector<string> &listOfFiles){

    DIR *dir = opendir(dirPath.c_str());
    if(!dir){
        cout << "Not a directory: "<< dirPath << endl;
    }

    dirent *entry;
    while((entry = readdir(dir))!= NULL){
        if(hasSuffix(entry->d_name, ext)){
            listOfFiles.push_back(string(entry->d_name));
        }
    }
    closedir(dir);
}

void load_faces(
    string dirPath,
    Eigen::MatrixXd &S)
{
    vector<string> listOfFiles;
    string filepath;
    Eigen::MatrixXd V;
    Eigen::MatrixXd F;
    int nbFaces;
    int nbPoints;
    string ext = ".obj";
    findFilesWithExt(dirPath, ext, listOfFiles);
    cout << endl;
    if(listOfFiles.empty()){
        cout << "No faces found "<< endl;
        return;
    }

    nbFaces = listOfFiles.size();
    filepath = dirPath + "/" + listOfFiles.front();
    igl::read_triangle_mesh(filepath, V, F);
    nbPoints = V.rows();
    S.setZero(nbFaces, 3*nbPoints);
    for(int i = 0; i < nbFaces; i++){
        filepath = dirPath + "/" + listOfFiles[i];
        igl::read_triangle_mesh(filepath, V, F);
        V.transposeInPlace();
        S.row(i) = Eigen::Map<Eigen::RowVectorXd>(V.data(), V.size());
        V.resize(0,3);
    }
}

void compute_pca(string dirPath, Eigen::VectorXd& F_m, Eigen::MatrixXd& W){
    Eigen::MatrixXd F, F_centered, covariance;
    load_faces(dirPath, F);
    F_m = F.colwise().mean();
    F_centered = F.rowwise() - F_m.transpose();
    covariance = F_centered.adjoint() * F_centered;
    cout << covariance.rows() << " "<< covariance.cols()<< endl;
    //TODO covariance matrix is 6957 x 6957 -> right dimension?

}   
