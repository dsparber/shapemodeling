#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "warping.h"

#include <igl/boundary_loop.h>
#include <igl/slice.h>
#include <igl/cotmatrix.h>
#include <igl/writeOBJ.h>

const double SPATIAL_INDEX_RESOLUTION = 15;

Warping::Warping(const Eigen::MatrixXd &V_template, const Eigen::MatrixXd &V_scan, const Eigen::MatrixXi &F_template, const Eigen::MatrixXi &F_scan, std::pair<Eigen::VectorXi, Eigen::VectorXi> landmarks)
	: V_template(V_template), V_scan(V_scan), F_template(F_template), F_scan(F_scan), landmarks(landmarks) {
	spatial_index = std::shared_ptr<SpatialIndex>(new SpatialIndex(V_scan, SPATIAL_INDEX_RESOLUTION));
}

Warping::~Warping() {/* empty? */}

void Warping::warp(const double lambda, const int iterations, const double relative_distance_threshold, const bool use_landmark_constraints, Eigen::MatrixXd &V_warped, Eigen::MatrixXi &F_warped) {
	std::cout << "Started warping" << std::endl;
	// set F_warped to F_template since it does not change
	F_warped = F_template;

	// calculate absolute distance threshold
	Eigen::Vector3d template_bounding_box_diagonal = V_template.colwise().maxCoeff() - V_template.colwise().minCoeff();
	double absolute_distance_threshold = relative_distance_threshold * template_bounding_box_diagonal.norm();

	// loop over number of iterations
	Eigen::MatrixXd V_current, V_next;
	V_current = V_template;
	for (int i = 0; i < iterations; i++) {
		Warping::warping_iteration(lambda, absolute_distance_threshold, i, use_landmark_constraints, V_current, V_next);
		V_current = V_next;
	}
	V_warped = V_current;
	V_warp_result = V_current;
	std::cout << "Finished warping" << std::endl;
}

void Warping::store_last_result_to_obj_file(
	const std::string path
) {
	if (V_warp_result.size() > 0) {
		if (igl::writeOBJ(path, V_warp_result, F_template)) {
			std::cout << "Successfully stored warped mesh in '" << path << "'" << std::endl;
		}
		else {
			std::cout << "Failed storing warped mesh in '" << path << "'" << std::endl;
		}
	}
	else {
		std::cout << "There is no warped mesh to store, run 'warp' first." << std::endl;
	}
}

void Warping::warping_iteration(const double lambda, const double absolute_distance_threshold, const int iteration_number, const bool use_landmark_constraints, Eigen::MatrixXd &V_current, Eigen::MatrixXd &V_next) {
	std::cout << "Warping iteration " << iteration_number << std::endl;

	// create constraints
	Eigen::SparseMatrix<double> C;
	Eigen::MatrixXd D;
	create_constraints_and_targets(absolute_distance_threshold, use_landmark_constraints, C, D);

	// get cotangent laplacian
	Eigen::SparseMatrix<double> L;
	igl::cotmatrix(V_current, F_template, L);
	L = -L;

	// prepare sparse left hand side and solver
	Eigen::SparseMatrix<double> lhs = (L.transpose() * L) + lambda * (C.transpose() * C);
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> cholesky;
	cholesky.compute(lhs);

	// compute right hand side
	Eigen::MatrixXd tmp = C.transpose() * D;
	Eigen::MatrixXd rhs = (L.transpose() * L) * V_current + lambda * tmp;

	// solve
	V_next = cholesky.solve(rhs);
}

void Warping::create_constraints_and_targets(const double absolute_distance_threshold, const bool use_landmark_constraints, Eigen::SparseMatrix<double> &C, Eigen::MatrixXd &D) {
	// compute boundary targets
	Eigen::VectorXi boundary_indices;
	Eigen::MatrixXd boundary_targets;
	igl::boundary_loop(F_template, boundary_indices);
	igl::slice(V_template, boundary_indices, 1, boundary_targets);

	// landmark constraints are already given
	Eigen::VectorXi landmark_indices;
	Eigen::MatrixXd landmark_targets;
	if (use_landmark_constraints) {
		landmark_indices = landmarks.first;
		igl::slice(V_scan, landmarks.second, 1, landmark_targets);
	}

	// compute closest point on scan for template vertices, create targets by absolute distance threshold
	Eigen::MatrixXd closest_scan_vertices;
	Eigen::VectorXd closest_scan_distances;
	compute_closest_scan_vertices(closest_scan_vertices, closest_scan_distances);
	std::vector<int> below_threshold_indices_vec = {};
	for (int i = 0; i < closest_scan_distances.rows(); i++) {
		if (closest_scan_distances(i) <= absolute_distance_threshold) {
			below_threshold_indices_vec.push_back(i);
		}
	}
	Eigen::VectorXi below_threshold_indices = Eigen::Map<Eigen::VectorXi, Eigen::Unaligned>(below_threshold_indices_vec.data(), below_threshold_indices_vec.size());
	Eigen::MatrixXd below_threshold_targets;
	igl::slice(closest_scan_vertices, below_threshold_indices, 1, below_threshold_targets);

	// merge the indices and targets together with priority: boundary > landmarks > below threshold
	std::vector<int> unique_indices = {};
	std::vector<Eigen::Vector3d> unique_targets = {};
	int i = 0, j = 0, k = 0; // i for boundary, j for landmarks, k for below threshold
	while (i < boundary_indices.size() || j < landmark_indices.size() || k < below_threshold_indices.size()) {
		// current boundary index is smallest of the remaining ones
		if (i < boundary_indices.size()
			&& (j >= landmark_indices.size() || boundary_indices(i) <= landmark_indices(j))
			&& (k >= below_threshold_indices.size() || boundary_indices(i) <= below_threshold_indices(k))) {

			unique_indices.push_back(boundary_indices(i));
			unique_targets.push_back(boundary_targets.row(i));

			// avoid adding the same index twice
			if (j < landmark_indices.size() && boundary_indices(i) == landmark_indices(j))
				j++;
			if (k < below_threshold_indices.size() && boundary_indices(i) == below_threshold_indices(k))
				k++;
			i++;
		}
		// current landmark index is smallest of the remaining ones
		else if (j < landmark_indices.size()
			&& (k >= below_threshold_indices.size() || landmark_indices(j) <= below_threshold_indices(k))) {

			unique_indices.push_back(landmark_indices(j));
			unique_targets.push_back(landmark_targets.row(j));

			if (k < below_threshold_indices.size() && landmark_indices(j) == below_threshold_indices(k))
				k++;
			j++;
		}
		// current below threshold index is smallest of the remaining ones
		else {
			unique_indices.push_back(below_threshold_indices(k));
			unique_targets.push_back(below_threshold_targets.row(k));

			k++;
		}
	}

	// fill in C and D
	C.resize(unique_indices.size(), V_template.rows());
	C.reserve(unique_indices.size());
	D.resize(unique_targets.size(), 3);
	for (int i = 0; i < unique_indices.size(); i++) {
		C.insert(i, unique_indices[i]) = 1;
		D.row(i) = unique_targets[i];
	}
	C.makeCompressed();
}

void Warping::compute_closest_scan_vertices(Eigen::MatrixXd &closest_scan_vertices, Eigen::VectorXd &closest_scan_distances) {
	closest_scan_vertices.resize(V_template.rows(), 3);
	closest_scan_distances.resize(V_template.rows());
	for (int i = 0; i < V_template.rows(); i++) {
		Eigen::Vector3d v_i = V_template.row(i);

		// use spatial grid to check fewer scan vertices
		std::vector<int> spatial_index_neighborhood = {};
		int radius = 1;
		while (spatial_index_neighborhood.size() < 1) {
			spatial_index_neighborhood = spatial_index->getNeighborhoodIndicesFromPoint(radius++, v_i);
		}

		// loop over close scan vertices
		double closest_scan_distance = INFINITY;
		int closest_scan_index = -1;
		for (int j = 0; j < spatial_index_neighborhood.size(); j++) {
			int ind_j = spatial_index_neighborhood[j];
			double distance = (v_i - Eigen::Vector3d(V_scan.row(ind_j))).norm();
			if (distance < closest_scan_distance) {
				closest_scan_distance = distance;
				closest_scan_index = ind_j;
			}
		}

		closest_scan_vertices.row(i) = V_scan.row(closest_scan_index);
		closest_scan_distances(i) = closest_scan_distance;
	}
}

Warping::SpatialIndex::SpatialIndex(const Eigen::MatrixXd &points, const int resolution) : resolution(resolution) {
	bb_min = points.colwise().minCoeff();
	dim_extended = (Eigen::Vector3d(points.colwise().maxCoeff()) - bb_min) * 1.01; // enlargen it slightly to not lose the outermost vertices
	spacingX = dim_extended(0) / resolution;
	spacingY = dim_extended(1) / resolution;
	spacingZ = dim_extended(2) / resolution;
	grid = std::vector<std::vector<int>>(pow(resolution, 3), std::vector<int>());

	for (int i = 0; i < points.rows(); i++) {
		Eigen::Vector3d relativeCoordinates = Eigen::Vector3d(points.row(i)) - bb_min;
		int x = floor((relativeCoordinates(0) / spacingX));
		int y = floor((relativeCoordinates(1) / spacingY));
		int z = floor((relativeCoordinates(2) / spacingZ));
		int index = x + resolution * (y + resolution * z);
		grid[index].push_back(i);
	}
}

Warping::SpatialIndex::~SpatialIndex() {/* empty? */}

std::vector<int> Warping::SpatialIndex::getNeighborhoodIndicesFromPoint(int radius, Eigen::Vector3d &point) {
	Eigen::Vector3d relativePoint = point - bb_min;
	int x = floor((relativePoint(0) / spacingX));
	int y = floor((relativePoint(1) / spacingY));
	int z = floor((relativePoint(2) / spacingZ));

	std::vector<int> indices = {};
	for (int xi = std::max(0, x - radius); xi <= std::min(resolution - 1, x + radius); xi++) {
		for (int yi = std::max(0, y - radius); yi <= std::min(resolution - 1, y + radius); yi++) {
			for (int zi = std::max(0, z - radius); zi <= std::min(resolution - 1, z + radius); zi++) {
				int ind = xi + resolution * (yi + resolution * zi);

				for (int i : grid[ind]) {
					indices.push_back(i);
				}
			}
		}
	}

	return indices;
}
