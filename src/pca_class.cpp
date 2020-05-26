#include "pca_class.h"
#include <Eigen/Eigenvalues> 
using namespace std;


PCA::PCA(const Viewer &viewer_) //, map<char, string> &inputs)
     : viewer(viewer_)
{
    // read inputs
    // if(!inputs.empty()){
    //     for(auto it = inputs.begin(); it != inputs.end(); it++){
    //         switch(it->first){
    //             case 'm':
    //                 m = atoi(it->second.c_str());
    //                 break;
    //             case 'd':
    //                 dataPath = it->second;
    //                 break;
    //             case 'w':
    //                 writePath = it->second;
    //                 break;
    //             case 't':
    //                 templateFile = it->second;
    //                 break;
    //             case 'n':
    //                 n = atoi(it->second.c_str());
    //                 break;
    //             default:
    //                 break;
    //         }
    //     }
    // }


    // add seperator for consistency
    // if(dataPath.back() != SEP){
    //     dataPath += SEP;
    // }
    // if(writePath.back() != SEP){
    //     writePath += SEP;
    // }    

    cout << "read triangle mesh" << endl;
    igl::read_triangle_mesh(templateFile, V_template, F_template);
    weights = vector<double>(m, 0.0);
    // ugly code but what couldn't be done in the constructor get's pushed into main
};


void PCA::callback_draw_viewer_menu()
{

}
bool PCA::callback_pre_draw(Viewer &viewer)
{

}

bool PCA::callback_mouse_down(Viewer &viewer, int button, int modifier)
{

}
bool PCA::callback_key_pressed(Viewer &viewer, unsigned char key, int modifier)
{

}
bool PCA::callback_mouse_up(Viewer &viewer, int button, int modifier)
{

}
bool PCA::callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y)
{

}

// bool PCA::set_V(Eigen::MatrixXd &Vnew){
// 	V = Vnew;
// 	viewer.data().clear();
// 	viewer.data().set_mesh(V, F);
// 	viewer.core.align_camera_center(V);
// 	return true;
// }


bool PCA::hasSuffix(const string& s, const string& suffix)
{
    return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());    
}

void PCA::findFilesWithExt(const string &dirPath, const string &ext, vector<string> &listOfFiles)
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

void PCA::load_faces(string dirPath, Eigen::MatrixXd &S)
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

void PCA::compute_pca(string dirPath, int m, Eigen::VectorXd& mX, Eigen::MatrixXd& E, Eigen::VectorXd& EV)
{
    int n, d;
    Eigen::MatrixXd X, X_centered, L; // nxd
    load_faces(dirPath, X);

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

    EV = eigen.eigenvalues().segment(n - m, m);
}   
