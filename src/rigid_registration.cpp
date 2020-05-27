#include "rigid_registration.h"
#include <igl/read_triangle_mesh.h>
#include <igl/slice.h>
#include <igl/writeOBJ.h>
#include "common.h"
#include "warping.h"

void rigid_registration(const std::string &landmarks_template,
                        const std::string &landmarks_scan,
                        const std::string &path_template,
                        const std::string &path_scan,
                        Eigen::MatrixXd &W_template,
                        Eigen::MatrixXd &W_scan,
                        Eigen::MatrixXi &F_template,
                        Eigen::MatrixXi &F_scan,
                        std::pair<Eigen::VectorXi, Eigen::VectorXi> &landmarks)
{
    // Read the obj files.
    Eigen::MatrixXd V_template, V_scan;
    igl::read_triangle_mesh(path_template, V_template, F_template);
    igl::read_triangle_mesh(path_scan, V_scan, F_scan);

    // Read txt files.
    landmarks = register_landmarks(landmarks_template, landmarks_scan);

    // Compute registration.
    rigid_registration_core(V_template, V_scan, landmarks.first, landmarks.second, W_template, W_scan);
}

std::pair<Eigen::VectorXi, Eigen::VectorXi> register_landmarks(const std::string &path1, const std::string &path2)
{
    std::ifstream file1;
    file1.open(path1);
    int index1, label1, index2, label2;
    std::vector<std::pair<int, int>> v1{}, v2{};
    while (file1 >> index1 && file1 >> label1)
    {
        v1.push_back({label1, index1});
    }
    file1.close();
    sort(v1.begin(), v1.end());

    Eigen::VectorXi vec1(v1.size());
    for (int i = 0; i < v1.size(); ++i)
    {
        vec1(i) = v1[i].second;
    }

    std::ifstream file2;
    file2.open(path2);
    while (file2 >> index2 && file2 >> label2)
    {
        v2.push_back({label2, index2});
    }
    file2.close();
    sort(v2.begin(), v2.end());

    Eigen::VectorXi vec2(v2.size());
    for (int i = 0; i < v2.size(); ++i)
    {
        vec2(i) = v2[i].second;
    }

    return {vec1, vec2};
}

void rigid_registration_core(const Eigen::MatrixXd &V_template,
                             const Eigen::MatrixXd &V_scan,
                             const Eigen::VectorXi &idx_template,
                             const Eigen::VectorXi &idx_scan,
                             Eigen::MatrixXd &W_template,
                             Eigen::MatrixXd &W_scan)
{
    // Extract the two matrices of correspondences.
    Eigen::MatrixXd M_template, M_scan;
    Eigen::Vector3d cols = {0, 1, 2};
    igl::slice(V_template, idx_template, cols, M_template);
    igl::slice(V_scan, idx_scan, cols, M_scan);

    // Mean vertex vectors.
    Eigen::VectorXd mean_template, mean_scan;
    mean_template = V_template.colwise().mean();
    mean_scan = V_scan.colwise().mean();

    // Center in (0,0,0) both template and scan and store into W1, W2.
    W_template = V_template.rowwise() - mean_template.transpose();
    W_scan = V_scan.rowwise() - mean_scan.transpose();
    W_scan = V_scan;

    // Rescale the template to the scan.
    double dist_scan, dist_template;
    Eigen::VectorXd mean_land_scan, mean_land_template;
    mean_land_template = M_template.colwise().mean();
    mean_land_scan = M_scan.colwise().mean();
    dist_template = (M_template.rowwise() - mean_land_template.transpose()).rowwise().norm().mean();
    dist_scan = (M_scan.rowwise() - mean_land_scan.transpose()).rowwise().norm().mean();
    double lambda;
    lambda = dist_scan / dist_template;
    W_template *= lambda;

    // Adjust the corresponding M matrices.
    igl::slice(W_template, idx_template, cols, M_template);
    igl::slice(W_scan, idx_scan, cols, M_scan);

    // Ajust translation.
    mean_land_template = M_template.colwise().mean();
    mean_land_scan = M_scan.colwise().mean();
    W_template = W_template.rowwise() - mean_land_template.transpose();
    W_scan = W_scan.rowwise() - mean_land_scan.transpose();
    igl::slice(W_template, idx_template, cols, M_template);
    igl::slice(W_scan, idx_scan, cols, M_scan);

    // Compute the best rotation and rotate the template.
    Eigen::MatrixXd X, U, U_tilde, R;

    //X = M_template.transpose() * M_scan;
    X = M_scan.transpose() * M_template;

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(X, Eigen::ComputeFullV | Eigen::ComputeFullU);
    U = svd.matrixU();
    U_tilde = svd.matrixV();
    if ((U_tilde * U.transpose()).determinant() > 0)
    {
        R = U_tilde * U.transpose();
    }
    else
    {
        U.col(2) *= -1;
        R = U_tilde * U.transpose();
    }

    // Rotate the template.
    //W_template = (R * W_template.transpose()).transpose();
    // Rotate the scan.
    W_scan = (R * W_scan.transpose()).transpose();
}

void warp_meshes_to_folder(const std::string &folder_scans, const std::string &template_obj,
                           const std::string template_txt, const std::string output_path,
                           const double lambda, const int &iterations, const double relative_distance_threshold, const bool use_landmark_constraints)
{
    vector<string> obj_files;
    findFilesWithExt(folder_scans, ".obj", obj_files);
    int n_files = obj_files.size();

    for (int i = 0; i < n_files; ++i)
    {
        cout << "Processing file " << i + 1 << " out of " << n_files << endl;

        size_t lastindex = obj_files[i].find_last_of(".");
        std::string rawname = obj_files[i].substr(0, lastindex);

        std::string scan_obj = obj_files[i];
        std::string scan_txt = rawname + ".txt";

        Eigen::MatrixXd W_template, W_scan, V_warped;
        Eigen::MatrixXi F_template, F_scan, F_warped;
        std::pair<Eigen::VectorXi, Eigen::VectorXi> landmarks;

        // First perform rigid alignment.
        rigid_registration(template_txt, folder_scans + "/" + scan_txt, template_obj, folder_scans + "/" + scan_obj,
                           W_template, W_scan, F_template, F_scan, landmarks);

        // Then warp the template to scan.
        std::shared_ptr<Warping> warp;
        warp = std::shared_ptr<Warping>(new Warping(W_template, W_scan, F_template, F_scan, landmarks));
        warp->warp(lambda, iterations, relative_distance_threshold, use_landmark_constraints, V_warped, F_warped);

        // Finally translate vertices such that their mean is in 0.
        Eigen::VectorXd mean;
        mean = V_warped.colwise().mean();
        V_warped = V_warped.rowwise() - mean.transpose();

        // Write to disk.
        igl::writeOBJ(output_path + "/" + rawname + "_warped" + ".obj", V_warped, F_warped);
    }
    cout << "Finished to write warped meshes to file." << endl;
}
