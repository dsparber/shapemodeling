#include <igl/read_triangle_mesh.h>
#include <igl/writeOBJ.h>
/*** insert any necessary libigl headers here ***/
#include <math.h> 
#ifndef _WIN32
#include <unistd.h>
#endif
#include "../src/common.h"

using namespace std;

// vertex array, #V x3
Eigen::MatrixXd V;
// face array, #F x3
Eigen::MatrixXi F;
// normals
Eigen::MatrixXd N;
// eigenvectors
Eigen::MatrixXd  W;
// mean face vector
Eigen::VectorXd F_m;
// mean face matrix
Eigen::MatrixXd M; 

//
Eigen::VectorXd eigenvalues;

int N_SAMPLES = 10;
int PRINCIPLE_COMPONENTS = 9;
string dirPath = "../data/aligned_faces_example/example4/";
string templatePath = "../data/face_template/headtemplate_noneck_lesshead_4k.obj";
string writePath;


// ************************Function Declaration ************************ //
void reshape(Eigen::VectorXd flat, int rows, int cols, Eigen::MatrixXd &matrix);
void compute_random_face();
// ******************************************************************** //


void reshape(Eigen::VectorXd flat, int rows, int cols, Eigen::MatrixXd &matrix){
	Eigen::Map<Eigen::MatrixXd> M(flat.data(), cols, rows);
	matrix = Eigen::MatrixXd(M.transpose());
}

void compute_random_face(Eigen::MatrixXd &RF){
	Eigen::VectorXd weights; weights.setZero(W.cols());
	Eigen::VectorXd morphedFace;
    // define weigths
	if(weights.nonZeros() != 0)
		weights.normalize();
	
	morphedFace = F_m + W * weights; // dx1
	reshape(morphedFace.transpose(), morphedFace.size()/3, 3, RF);
}

int main(int argc,char *argv[]){

    // read inputs
	if(argc < 2){
		cout << "Usage ./pca path/to/write/folder/ -m <#components> -d <data path> -t <template file>" << endl;
        return -1;
	}else{
		writePath = argv[1];
	}

	int opt;
	while((opt = getopt(argc, argv, "d:m:t:n:")) != -1){
		switch(opt){
			case 'm':
				PRINCIPLE_COMPONENTS = atoi(optarg);
				break;
            case 'd':
                dirPath = optarg;
                break;
            case 't':
                templatePath = optarg;
                break;
            case 'n':
                N_SAMPLES = atoi(optarg);
                break;
		}
	}

	//set F from a template file
	bool success = igl::read_triangle_mesh(templatePath, V, F);

    if(!success){
        cout << "template file not found!\n";
        return -1;
    }

	compute_pca(dirPath, PRINCIPLE_COMPONENTS, F_m, W, eigenvalues);
	// reshape(F_m, F_m.size()/3, 3, M);

    Eigen::VectorXd morphedFace;
    Eigen::VectorXd weights;
    string filename = "generated_face";
    string filepath;
    Eigen::VectorXd randVec;
    for(int i = 0; i < N_SAMPLES; i++){
        randVec = Eigen::VectorXd::Random(PRINCIPLE_COMPONENTS);
        weights = randVec.array() / eigenvalues.array();
        weights.normalize();
        morphedFace = F_m + W * weights; // dx1
        reshape(morphedFace.transpose(), morphedFace.size()/3, 3, V);
        filepath = writePath + filename + "_" + to_string(i) + ".obj";
        igl::writeOBJ(filepath, V, F);
        cout << "Written face to: "<< filepath << endl;
    }

}
