#include "common.h"
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

void reshape(Eigen::VectorXd flat, int rows, int cols, Eigen::MatrixXd &matrix){
	Eigen::Map<Eigen::MatrixXd> M(flat.data(), cols, rows);
	matrix = Eigen::MatrixXd(M.transpose());
}