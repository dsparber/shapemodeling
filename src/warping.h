#ifndef WARPING_H
#define WARPING_H

#include <memory>
#include <string>

class Warping {

public:
	/** Constructor
	*
	* Input:
	* @param V_template	 -  vertices of the (rigidly aligned) template
	* @param V_scan  -  vertices of the (rigidly aligned) scan
	* @param F_template	 -  faces of the (rigidly aligned) template
	* @param F_scan  -  faces of the (rigidly aligned) scan
	* @param landmarks  -  landmarks, see register_landmarks in rigid_registration.h
	*/
	Warping(
		const Eigen::MatrixXd &V_template,
		const Eigen::MatrixXd &V_scan,
		const Eigen::MatrixXi &F_template,
		const Eigen::MatrixXi &F_scan,
		std::pair<Eigen::VectorXi, Eigen::VectorXi> landmarks
	);

	/** Destructor */
	~Warping();

	/** Performs the warping
	*
	* Input:
	* @param lambda  -  regularization factor
	* @param iterations  -  number of warping iterations
	* @param relative_distance_threshold  -  distance threshold to determine which vertices should be fixed as targets, relative to diagonal of template bounding box
	* Output:
	* @param V_warped  -  vertices of the warped mesh
	* @param F_warped  -  faces of the warped mesh
	*/
	void warp(
		const double lambda,
		const int iterations,
		const double relative_distance_threshold,
		Eigen::MatrixXd &V_warped,
		Eigen::MatrixXi &F_warped // this output is currently only provided as an easy interface for other users and is the same as F_template
	);

	/** Stores last warped mesh to .obj file
	*
	* Input:
	* @param path  -  path where to store the mesh
	*/
	void store_last_result_to_obj_file(
		const std::string path
	);

private:
	// see constructor
	const Eigen::MatrixXd &V_template;
	const Eigen::MatrixXd &V_scan;
	const Eigen::MatrixXi &F_template;
	const Eigen::MatrixXi &F_scan;
	std::pair<Eigen::VectorXi, Eigen::VectorXi> landmarks;

	// last warping result, used to store to file
	Eigen::MatrixXd V_warp_result;

	/** Performs one warping iteration
	*
	* Input:
	* @param lambda  -  regularization factor
	* @param absolute_distance_threshold  -  distance threshold to determine which vertices should be fixed as targets, absolute value
	* @param iteration_number  -  the current iteration number, used for user notification
	* @param V_current  -  vertices of the current template warping state
	* Output:
	* @param V_next  -  vertices of the warping state after one additional iteration
	*/
	void warping_iteration(
		const double lambda,
		const double absolute_distance_threshold,
		const int iteration_number,
		Eigen::MatrixXd &V_current,
		Eigen::MatrixXd &V_next
	);

	/** Creates the constraint matrix and targets
	*
	* Input:
	* @param absolute_distance_threshold  -  distance threshold to determine which vertices should be fixed as targets, absolute value
	* Output:
	* @param C  -  constraint matrix (Id|constr on the exercise slides)
	* @param D  -  target matrix with 3 columns (c on the exercise slides)
	*/
	void create_constraints_and_targets(
		const double absolute_distance_threshold,
		Eigen::SparseMatrix<double> &C,
		Eigen::MatrixXd &D
	);

	/** Computes the closest scan vertex for every template vertex and their distance
	*
	* Output:
	* @param closest_scan_vertices  -  closest scan vertices
	* @param closest_scan_distances  -  distances to the closest scan vertex
	*/
	void compute_closest_scan_vertices(
		Eigen::MatrixXd &closest_scan_vertices,
		Eigen::VectorXd &closest_scan_distances
	);

	/** Spatial index used to speed up the compute_closest_scan_vertices method
	*
	* copied from jelak assignment 2 and adjusted to fit this task
	*/
	class SpatialIndex {
	public:
		SpatialIndex(
			const Eigen::MatrixXd &points,
			const int resolution
		);
		~SpatialIndex();

		std::vector<int> getNeighborhoodIndicesFromPoint(int radius, Eigen::Vector3d &point);

	private:
		std::vector<std::vector<int>> grid;
		int resolution;
		Eigen::Vector3d bb_min;
		Eigen::Vector3d dim_extended;
		double spacingX, spacingY, spacingZ;
	};

	std::shared_ptr<SpatialIndex> spatial_index;
};

#endif
