#include "pca.h"
#include <Eigen/Eigenvalues> 
using namespace std;

bool hasSuffix(const string& s, const string& suffix)
{
    return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());    
}

void findFilesWithExt(const string &dirPath, const string &ext, vector<string> &listOfFiles)
{

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

void load_faces(string dirPath, Eigen::MatrixXd &S)
{
    vector<string> listOfFiles;
    string filepath;
    Eigen::MatrixXd V;
    Eigen::MatrixXd F;
    int nbFaces;
    int nbPoints;
    string ext = ".obj";
    findFilesWithExt(dirPath, ext, listOfFiles);

    nbFaces = listOfFiles.size();
    filepath = dirPath + "/" + listOfFiles.front();
    igl::read_triangle_mesh(filepath, V, F);
    nbPoints = V.rows();
    S.setZero(nbFaces, 3*nbPoints);
    for(int i = 0; i < nbFaces; i++){
        filepath = dirPath + "/" + listOfFiles[i];
        cout << listOfFiles[i] << endl;
        igl::read_triangle_mesh(filepath, V, F);
        V.transposeInPlace();
        S.row(i) = Eigen::Map<Eigen::RowVectorXd>(V.data(), V.size());
        V.resize(0,3);
    }
}

void compute_pca(string dirPath, int m, Eigen::VectorXd& mX, Eigen::MatrixXd& E)
{
    int n, d;
    Eigen::MatrixXd X, X_centered, L; // nxd
    load_faces(dirPath, X);

    cout << "m: "<< m << endl;
    n = X.rows();
    d = X.cols();
    mX = X.colwise().mean();
    if(n < m){
        cout << "Not enough faces in database or choose smaller m, n = "<< n << ", m = "<<m<<endl;
        m = n;
    }
        
    X_centered = X.rowwise() - mX.transpose();
    // Want eigenvectors u of X^T*X in dxd --> to big
    // Instead compute eigenvectors v of X*X^T, with u = X^T*v
    L = X_centered * X_centered.adjoint();
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigen(L);
    auto v = eigen.eigenvectors();

    //find eigenvectors with the m biggest eigenvalues
    Eigen::MatrixXd u = X.adjoint() * v; 
    // return m last eigenvectors, since they are sorted by increasing eigenvalues
    E = u.block(0, n - m, d, m); //dxm matrix of eigenvectors (in cols)
    //TODO: Also save eigenvalues?
    cout << "Finish compute_pca " << endl;
}   
