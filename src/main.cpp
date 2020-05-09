#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <igl/local_basis.h>
#include <igl/grad.h>
#include <igl/min_quad_with_fixed.h>
#include <igl/cotmatrix.h>


/*** insert any necessary libigl headers here ***/
#include <igl/boundary_loop.h>
#include <igl/map_vertices_to_circle.h>
#include <igl/harmonic.h>
#include <igl/lscm.h>
#include <igl/adjacency_matrix.h>
#include <igl/sum.h>
#include <igl/diag.h>
#include <igl/speye.h>
#include <igl/repdiag.h>
#include <igl/cat.h>
#include <igl/dijkstra.h>

#include <math.h> 

using namespace std;
using namespace Eigen;

typedef Eigen::Triplet<double> T;

using Viewer = igl::opengl::glfw::Viewer;

Viewer viewer;

// vertex array, #V x3
Eigen::MatrixXd V;

// face array, #F x3
Eigen::MatrixXi F;

// UV coordinates, #V x2
Eigen::MatrixXd UV; // (u1, u2, ..., un, v1, v2, ..., vn)

int idx0 = -1;
int idx1 = -1;

double maxDistortion = 100;
bool showingUV = false;
bool freeBoundary = false;
bool withColor = false;
double TextureResolution = 10;
int distortionMetric = 2;
igl::opengl::ViewerCore temp3D;
igl::opengl::ViewerCore temp2D;

Eigen::MatrixXd colors;


// ************************Function Declaration ************************ //
void compute_colors();
static void computeSurfaceGradientMatrix(Eigen::SparseMatrix<double> &D1, Eigen::SparseMatrix<double> &D2);
void Redraw();
bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y);
static inline void SSVD2x2(const Eigen::Matrix2d& J, Eigen::Matrix2d& U, Eigen::Matrix2d& S, Eigen::Matrix2d& V);
void ConvertConstraintsToMatrixForm(VectorXi indices, MatrixXd positions, Eigen::SparseMatrix<double> &C, VectorXd &d);
void computeParameterization(int type);
bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers);
bool load_mesh(string filename);
bool callback_init(Viewer &viewer);
// ******************************************************************** //


void compute_colors(){
	colors = Eigen::MatrixXd::Zero(F.rows(), 3);
	Eigen::VectorXd D; D.setZero(F.rows());
	Eigen::MatrixXd J(F.rows(), 4);
	Eigen::Matrix2d Jt, Rt, U, S, V, temp;
	Eigen::SparseMatrix<double> Dx, Dy;
	computeSurfaceGradientMatrix(Dx, Dy);

	J.col(0) = Dx * UV.col(0);
	J.col(1) = Dy * UV.col(0);
	J.col(2) = Dx * UV.col(1);
	J.col(3) = Dy * UV.col(1);

	switch(distortionMetric){
		case(0):
		// Conformal - angle preserving D(J) = ||J + JT - tr(J)I||_F^2
		D = 2 * (J.col(0) - J.col(3)).array().square() + 2 * (J.col(1) + J.col(2)).array().square();
		break;
		case(1):
		// Isometric - length preserving D(J) = || J - R ||_F^2
		for(int i = 0; i < F.rows(); i++){
			Jt << J(i, 0), J(i, 1), J(i, 2), J(i, 3);
			SSVD2x2(Jt, U, S, V);
			temp << 1, 0, 0, (U*V.transpose()).determinant();
			Rt = U * temp * V.transpose();
			D(i) = (Jt - Rt).array().square().sum();
		}
		break;
		case(2):
		// Authalic - area preserving D(J) = (det J - 1)^2
		D = J.col(0).cwiseProduct(J.col(3)) - J.col(1).cwiseProduct(J.col(2));
		D -= Eigen::VectorXd::Ones(F.rows());
		D = D.cwiseProduct(D);
		break;
		default:
		std::cout << "Please use a valid value for distortion metric: 0 - conformal, 1 - isometric, 2 - authalic\n";
	}
	//Map distortion to 0:255
	Eigen::VectorXd maxColor = Eigen::VectorXd::Ones(F.rows());
	Eigen::VectorXd distortionColor(F.rows());

	cout << "Summed distortion: "<<  D.sum() << endl;

	distortionColor = (D - Eigen::VectorXd::Constant(F.rows(), D.mean()));
	double std = distortionColor.dot(distortionColor) / distortionColor.size();
	if(std != 0)
		distortionColor = (distortionColor / (sqrt(std)));
	distortionColor = (distortionColor.array() * 0.5) + 0.5; //shift to [0,1] interval
	colors.col(0) = maxColor;
	colors.col(1) = maxColor - distortionColor;
	colors.col(2) = maxColor - distortionColor;
}

void Redraw()
{
	viewer.data().clear();

	if (!showingUV)
	{
		viewer.data().set_mesh(V, F);
		viewer.data().set_face_based(false);

    if(UV.size() != 0)
    {
      viewer.data().set_uv(TextureResolution*UV);
      viewer.data().show_texture = true;
    }
	}
	else
	{
		viewer.data().show_texture = false;
		viewer.data().set_mesh(UV, F);
	}

	if(withColor){
		compute_colors();
		viewer.data().show_texture = false;
		viewer.data().set_colors(colors);
	}
}

bool callback_mouse_move(Viewer &viewer, int mouse_x, int mouse_y)
{
	if (showingUV)
		viewer.mouse_mode = igl::opengl::glfw::Viewer::MouseMode::Translation;
	return false;
}

static void computeSurfaceGradientMatrix(SparseMatrix<double> & D1, SparseMatrix<double> & D2)
{
	MatrixXd F1, F2, F3;
	SparseMatrix<double> DD, Dx, Dy, Dz;

	igl::local_basis(V, F, F1, F2, F3);
	igl::grad(V, F, DD);

	Dx = DD.topLeftCorner(F.rows(), V.rows());
	Dy = DD.block(F.rows(), 0, F.rows(), V.rows());
	Dz = DD.bottomRightCorner(F.rows(), V.rows());

	D1 = F1.col(0).asDiagonal()*Dx + F1.col(1).asDiagonal()*Dy + F1.col(2).asDiagonal()*Dz;
	D2 = F2.col(0).asDiagonal()*Dx + F2.col(1).asDiagonal()*Dy + F2.col(2).asDiagonal()*Dz;
}


static inline void SSVD2x2(const Eigen::Matrix2d& J, Eigen::Matrix2d& U, Eigen::Matrix2d& S, Eigen::Matrix2d& V)
{
	double e = (J(0) + J(3))*0.5;
	double f = (J(0) - J(3))*0.5;
	double g = (J(1) + J(2))*0.5;
	double h = (J(1) - J(2))*0.5;
	double q = sqrt((e*e) + (h*h));
	double r = sqrt((f*f) + (g*g));
	double a1 = atan2(g, f);
	double a2 = atan2(h, e);
	double rho = (a2 - a1)*0.5;
	double phi = (a2 + a1)*0.5;

	S(0) = q + r;
	S(1) = 0;
	S(2) = 0;
	S(3) = q - r;

	double c = cos(phi);
	double s = sin(phi);
	U(0) = c;
	U(1) = s;
	U(2) = -s;
	U(3) = c;

	c = cos(rho);
	s = sin(rho);
	V(0) = c;
	V(1) = -s;
	V(2) = s;
	V(3) = c;
}

void ConvertConstraintsToMatrixForm(VectorXi indices, MatrixXd positions, Eigen::SparseMatrix<double> &C, VectorXd &d)
{
	// Convert the list of fixed indices and their fixed positions to a linear system
	// Hint: The matrix C should contain only one non-zero element per row and d should contain the positions in the correct order.
	
	// flatten positions into d vector
	d.resize(2*positions.rows());
	for(int i = 0; i < positions.rows(); i++){
		d(i) = positions(i, 0);
		d(i + positions.rows()) = positions(i, 1);
	}

	// construct C matrix
	C.resize(2*positions.rows(), 2*V.rows());
	vector<T> coefficients; // build list of triplets for sparse matrix C
	for(int i = 0; i < positions.rows(); i++){
		coefficients.push_back(T(i, indices(i), 1));
		coefficients.push_back(T(i + positions.rows(), indices(i)+V.rows(), 1));
	}
	C.setFromTriplets(coefficients.begin(), coefficients.end());
}

void computeParameterization(int type)
{
	VectorXi fixed_UV_indices; //#longest loop
	MatrixXd fixed_UV_positions;

	SparseMatrix<double> A;
	VectorXd b;
	Eigen::SparseMatrix<double> C;
	VectorXd d;
	int npoints = V.rows(); int npositions = fixed_UV_positions.rows();
	// Find the indices of the boundary vertices of the mesh and put them in fixed_UV_indices
	if (!freeBoundary)
	{
		// The boundary vertices should be fixed to positions on the unit disc. Find these position and
		// save them in the #V x 2 matrix fixed_UV_position.
		igl::boundary_loop(F, fixed_UV_indices); // find longest loop
		fixed_UV_positions.resize(V.rows(), 2);
		igl::map_vertices_to_circle(V, fixed_UV_indices, fixed_UV_positions);
	}
	else
	{
		// Fix two UV vertices. This should be done in an intelligent way. Hint: The two fixed vertices should be the two most distant one on the mesh.
		
		//do computation once
		if(idx0 == -1){

		// find 2 most distanced vertices

		// compute adjacency list
			vector<vector<int>> VV;
			igl::adjacency_list(F, VV);
			set<int> targets;
			VectorXd min_distance, previous;
			double maxD = 0.0;
			double D;
			int idx;
			VectorXi boundary_vertices;
			igl::boundary_loop(F, boundary_vertices); // find longest loop

			// call dijkstra on boundary vertices
			for(int i = 0; i < boundary_vertices.size(); i++){
				igl::dijkstra(boundary_vertices[i], targets, VV, min_distance, previous);
				D = min_distance.maxCoeff(&idx);
				if(D > maxD){
					maxD = D;
					idx0 = boundary_vertices[i];
					idx1 = idx;
				}
			}
		}

		fixed_UV_indices.resize(2);
		fixed_UV_indices(0) = idx0;
		fixed_UV_indices(1) = idx1;

		fixed_UV_positions.resize(2, 2);
		fixed_UV_positions << 0, 1, 0, -1;
	}

	ConvertConstraintsToMatrixForm(fixed_UV_indices, fixed_UV_positions, C, d);

	// Find the linear system for the parameterization (1- Tutte, 2- Harmonic, 3- LSCM, 4- ARAP)
	// and put it in the matrix A.
	// The dimensions of A should be 2#V x 2#V.
	A.resize(2* npoints, 2* npoints);
	if (type == '1') {
		// Add your code for computing uniform Laplacian for Tutte parameterization
		// Hint: use the adjecency matrix of the mesh
		Eigen::SparseMatrix<double> adj;
		Eigen::SparseVector<double> deg;
		Eigen::SparseMatrix<double> Lu; //uniform laplacian Lu = deg - adj degree on diagonal, -adj otherwise
		igl::adjacency_matrix(F, adj);
		igl::sum(adj, 1, deg);
		igl::diag(deg, Lu); Lu -= adj; //L = D - A
		vector<T> coeffA; //apply on u and v coordinates
		for(int k=0; k<Lu.outerSize(); ++k){
			for(Eigen::SparseMatrix<double>::InnerIterator it(Lu,k); it; ++it){
				coeffA.push_back(T(it.row(), it.col(), it.value())); // u
				coeffA.push_back(T(it.row() + npoints, it.col() + npoints, it.value())); // v
			}
		}
		A.setFromTriplets(coeffA.begin(), coeffA.end());
		// A*=2; 
		b.setZero(2 * npoints);
	}

	if (type == '2') {
		// Add your code for computing cotangent Laplacian for Harmonic parameterization
		// Use can use a function "cotmatrix" from libIGL, but ~~~~***READ THE DOCUMENTATION***~~~~
		Eigen::SparseMatrix<double> Lc;

		// can use derivation from exercise
		Eigen::SparseMatrix<double> Dx, Dy;
		computeSurfaceGradientMatrix(Dx, Dy);

		// area diagonal matrix 
		Eigen::VectorXd areas(F.rows());	
		igl::doublearea(V, F, areas);
		Eigen::SparseMatrix<double> areasM;
		areasM.resize(areas.size(), areas.size());
		vector<T> coeff;
		for(int i = 0; i < areas.size(); i++){
			coeff.push_back(T(i, i, areas(i)));
		}
		areasM.setFromTriplets(coeff.begin(), coeff.end());

		Eigen::SparseMatrix<double> a00, a01, a10, a11;
		a00.resize(Dx.cols(), Dx.cols());
		a01.resize(Dx.cols(), Dx.cols());
		a10.resize(Dx.cols(), Dx.cols());
		a11.resize(Dx.cols(), Dx.cols());
		a00 = Dx.transpose()*areasM*Dx + Dy.transpose()*areasM*Dy;
		a11 = a00;
		
		vector<vector<Eigen::SparseMatrix<double>>> a(2, vector<Eigen::SparseMatrix<double>>(2));
		a[0][0] = a00;
		a[0][1] = a01;
		a[1][0] = a10;
		a[1][1] = a11;
		igl::cat(a, A);

		// igl::cotmatrix(V, F, Lc);

		// vector<T> coeffA;
		// for(int k=0; k<Lc.outerSize(); ++k){
		// 	for(Eigen::SparseMatrix<double>::InnerIterator it(Lc,k); it; ++it){
		// 		coeffA.push_back(T(it.row(), it.col(), it.value())); // u
		// 		coeffA.push_back(T(it.row() + npoints, it.col() + npoints, it.value())); // v
		// 	}
		// }
		
		// A.setFromTriplets(coeffA.begin(), coeffA.end());
		// A *= 2;
		b.setZero(2 * npoints);
	}

	if (type == '3') {
		// Add your code for computing the system for LSCM parameterization
		// Note that the libIGL implementation is different than what taught in the tutorial! Do not rely on it!!

		// compute surface gradients
		Eigen::SparseMatrix<double> Dx, Dy;
		computeSurfaceGradientMatrix(Dx, Dy);

		// area diagonal matrix 
		Eigen::VectorXd areas(F.rows());	
		igl::doublearea(V, F, areas);
		Eigen::SparseMatrix<double> areasM;
		areasM.resize(areas.size(), areas.size());
		vector<T> coeff;
		for(int i = 0; i < areas.size(); i++){
			coeff.push_back(T(i, i, areas(i)));
		}
		areasM.setFromTriplets(coeff.begin(), coeff.end());

		//compute A^TA	

		Eigen::SparseMatrix<double> a00, a01, a10, a11;
		a00.resize(Dx.cols(), Dx.cols());
		a01.resize(Dx.cols(), Dx.cols());
		a10.resize(Dx.cols(), Dx.cols());
		a11.resize(Dx.cols(), Dx.cols());
		a00 = Dx.transpose()*areasM*Dx + Dy.transpose()*areasM*Dy;
		a01 = -Dx.transpose()*areasM*Dy + Dy.transpose()*areasM*Dx;
		a10 = -Dy.transpose()*areasM*Dx + Dx.transpose()*areasM*Dy;
		a11 = a00;

		//System 2* (DxTADx+ DyTADy, -DxTADy + DyTADx, DxTADy - DyTADx, DxTADx+ DyTADy), use 2 from doublearea
		vector<vector<Eigen::SparseMatrix<double>>> a(2, vector<Eigen::SparseMatrix<double>>(2));
		a[0][0] = 2*a00;
		a[0][1] = 2*a01;
		a[1][0] = 2*a10;
		a[1][1] = 2*a11;

		igl::cat(a, A);
		b.setZero(2 * npoints);
	}


	if (type == '4') {
		// Add your code for computing ARAP system and right-hand side
		// Implement a function that computes the local step first
		Eigen::SparseMatrix<double> Dx, Dy;
		computeSurfaceGradientMatrix(Dx, Dy);

		Eigen::VectorXd Dxu = Dx * UV.col(0);
		Eigen::VectorXd Dyu = Dy * UV.col(0);
		Eigen::VectorXd Dxv = Dx * UV.col(1);
		Eigen::VectorXd Dyv = Dy * UV.col(1);

		Eigen::VectorXd R00, R01, R10, R11;
		R00.resize(F.rows());
		R01.resize(F.rows());
		R10.resize(F.rows());
		R11.resize(F.rows());

		// Use SSVD2x2 solve for every triangle
		for(int i = 0; i < F.rows(); i++){
			Eigen::Matrix2d J, U, S, V, R;
			J << Dxu(i), Dxv(i), Dyu(i), Dyv(i);
			SSVD2x2(J, U, S, V);
			Eigen::Matrix2d temp;
			temp << 1, 0, 0, (U.transpose() * V).determinant();
			R = U * temp * V.transpose();
			R00(i) = R(0,0);
			R01(i) = R(0,1);
			R10(i) = R(1,0);
			R11(i) = R(1,1);
		}

		// Then construct the matrix with the given rotation matrices
		Eigen::VectorXd areas(F.rows());	
		igl::doublearea(V, F, areas);
		Eigen::SparseMatrix<double> areasM;
		areasM.resize(areas.size(), areas.size());
		vector<T> coeff;
		for(int i = 0; i < areas.size(); i++){
			coeff.push_back(T(i, i, areas(i)));
		}
		areasM.setFromTriplets(coeff.begin(), coeff.end());

		Eigen::SparseMatrix<double> a00, a01, a10, a11;
		a00.resize(Dx.cols(), Dx.cols());
		a01.resize(Dx.cols(), Dx.cols());
		a10.resize(Dx.cols(), Dx.cols());
		a11.resize(Dx.cols(), Dx.cols());
		a00 = Dx.transpose()*areasM*Dx + Dy.transpose()*areasM*Dy;
		a11 = a00;

		vector<vector<Eigen::SparseMatrix<double>>> a(2, vector<Eigen::SparseMatrix<double>>(2));
		a[0][0] = a00;
		a[0][1] = a01;
		a[1][0] = a10;
		a[1][1] = a11;
		igl::cat(a, A);

		b.resize(2*npoints);
		b.segment(0, npoints) = Dx.transpose()*areasM* R00 + Dy.transpose()*areasM*R10;
		b.segment(npoints, npoints) = Dx.transpose()*areasM* R01 + Dy.transpose()*areasM*R11;

	}

	// Solve the linear system.
	// Construct the system as discussed in class and the assignment sheet
	// Use igl::cat to concatenate matrices
	// Use Eigen::SparseLU to solve the system. Refer to tutorial 3 for more detail
	Eigen::SparseMatrix<double> M;
	Eigen::SparseMatrix<double> zeros;
	vector<vector<Eigen::SparseMatrix<double>>> m(2, vector<Eigen::SparseMatrix<double>>(2));
	zeros.resize(C.rows(), C.rows());
	m[0][0] = A;
	m[0][1] = C.transpose();
	m[1][0] = C;
	m[1][1] = zeros;

	igl::cat(m, M);

	Eigen::VectorXd rHS(b.size() + d.size());
	rHS << b, d;

	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	solver.compute(M);
	Eigen::VectorXd uv = solver.solve(rHS);
	
	// The solver will output a vector
	UV.resize(npoints, 2);
	
	UV.col(0) = uv.segment(0, npoints);
	UV.col(1) = uv.segment(npoints, npoints);

}

bool callback_key_pressed(Viewer &viewer, unsigned char key, int modifiers) {
	switch (key) {
	case '1':
	case '2':
	case '3':
	case '4':
		computeParameterization(key);
		break;
	case '5':
			// Add your code for detecting and displaying flipped triangles in the
			// UV domain here
		withColor = !withColor;
		break;	
	case '+':
		TextureResolution /= 2;
		break;
	case '-':
		TextureResolution *= 2;
		break;
	case ' ': // space bar -  switches view between mesh and parameterization
    if(showingUV)
    {
      temp2D = viewer.core;
      viewer.core = temp3D;
      showingUV = false;
    }
    else
    {
      if(UV.rows() > 0)
      {
        temp3D = viewer.core;
        viewer.core = temp2D;
        showingUV = true;
      }
      else { std::cout << "ERROR ! No valid parameterization\n"; }
    }
    break;
	}
	Redraw();
	return true;
}

bool load_mesh(string filename)
{
  igl::read_triangle_mesh(filename,V,F);
  Redraw();
  viewer.core.align_camera_center(V);
  showingUV = false;

  return true;
}

bool callback_init(Viewer &viewer)
{
	temp3D = viewer.core;
	temp2D = viewer.core;
	temp2D.orthographic = true;

	return false;
}

int main(int argc,char *argv[]) {
  if(argc != 2) {
    cout << "Usage ./assignment6 <mesh.off/obj>" << endl;
    load_mesh("../data/aligned_faces_example/example1/fabian-brille.objaligned.obj");
  }
  else
  {
    // Read points and normals
    load_mesh(argv[1]);
  }

	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);

	menu.callback_draw_viewer_menu = [&]()
	{
		// Draw parent menu content
		menu.draw_viewer_menu();

		// Add new group
		if (ImGui::CollapsingHeader("Parmaterization", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Expose variable directly ...
			ImGui::Checkbox("Free boundary", &freeBoundary);

			// TODO: Add more parameters to tweak here...
			ImGui::InputInt("Distortion Measure", &distortionMetric);
		}
	};

  viewer.callback_key_pressed = callback_key_pressed;
  viewer.callback_mouse_move = callback_mouse_move;
  viewer.callback_init = callback_init;

  viewer.launch();
}
