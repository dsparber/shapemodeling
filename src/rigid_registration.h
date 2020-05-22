#ifndef RIGID_REGISTRATION_H
#define RIGID_REGISTRATION_H

#include <igl/read_triangle_mesh.h>
#include <igl/slice.h>

/**
 * @brief This function reads the landmark txt files and returns two vectors of vertex
 * indices ordered by the corresponding landmark number.
 */
std::pair<Eigen::VectorXi, Eigen::VectorXi> register_landmarks(const std::string &path1, const std::string &path2);

/**
 * @brief Core function for rigid registration.
 * 
 * @param V_template : template vertices.
 * @param V_scan : scan vertices.
 * @param idx_template : template landmarks indices.
 * @param idx_scan : scan landmarks indices.
 * @param W_template : (output) new template vertices.
 * @param W_scan : (output) new scan vertices. 
 */
void rigid_registration_core(const Eigen::MatrixXd &V_template,
                             const Eigen::MatrixXd &V_scan,
                             const Eigen::VectorXi &idx_template,
                             const Eigen::VectorXi &idx_scan,
                             Eigen::MatrixXd &W_template,
                             Eigen::MatrixXd &W_scan);
/**
 * @brief This function operates the rigid registration taking as
 * input four paths (scan and template landmarks and obj files).
 * It basically puts together the two previous functions.
 */
void rigid_registration(const std::string &landmarks_template,
                        const std::string &landmarks_scan,
                        const std::string &path_template,
                        const std::string &path_scan,
                        Eigen::MatrixXd &W_template,
                        Eigen::MatrixXd &W_scan,
                        Eigen::MatrixXi &F_template,
                        Eigen::MatrixXi &F_scan,
                        std::pair<Eigen::VectorXi, Eigen::VectorXi> &landmarks);
/**
 * @brief Produces all warped obj files.
 * 
 * @param folder_scans : path to folder with all scans we want to do warp template on.
 * @param template_obj : path to template.obj file.
 * @param template_txt : path to template.txt file.
 * @param output_path : path to directory where we want to store warped files.
 * @param lambda : regularization parameter for warping.
 * @param iterations : number of warping iterations.
 * @param relative_distance_threshold : distance threshold for warping.
 */
void warp_meshes_to_folder(const std::string &folder_scans, const std::string &template_obj,
                           const std::string template_txt, const std::string output_path,
                           const double lambda, const int &iterations,
                           const double relative_distance_threshold);

#endif