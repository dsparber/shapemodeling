# Report - Game of Faces

## Our own scans

![wall](screenshots/got.png)

For this assignment we scanned our own faces with the following [Capture: 3D Scan Anything iOS App](https://apps.apple.com/de/app/capture-3d-scan-anything/id1444183458?l=en), which uses the TrueDepth camera. Unfortunately not all of our members were able to scan their faces, but we had some brave roommates, who were kind enough to let themself be scanned in the name of science! The scans from the app were still fairly noisy and only normalized point clouds, so we used Screened Poisson reconstruction from Meshlab and cut off all the unnecessary parts like hair, neck, etc. Next we appied Laplacian Smoothing to further clean the mesh. Lastly, we used our landmark UI tool to create the respective 22 landmarks and applied the other steps of the pipeline, like rigid alignment and warping to create morphs between our team mates. 

## Preprocessing

All the mesh boundaries have been smoothed using the bi-laplacian approach as in assignment 5.

![Preprocessing](screenshots/smooth.png)

## Landmarks

<img src="screenshots/landmarks.png" height="400">

For the landmarking tool we implemented a separate window which looks, which gives instructions on what key landmark to click in the top left corner. The current vertex in the mesh is marked with green. The user can confirm the selection with a hit on the space bar. Furthermore, the user is able to delete individual missclicked points by clicking the "Delete" button of individual boxes. In addition, a mouse hover over "Name" shows the description of the respective landmark. To load or save the mesh, the user needs to specify the path in the respective text box and click "Load"/"Save".

In total the user defines 22 landmarks with the following layout:

<img src="screenshots/landmarks_template.png" height="300">

This layout was agreed upon by all groups to reduce the overall workload and share the `landmark.txt` files between teams in a polybox folder. In it we provided the cleaned meshes and landmarks of our own scans, which we created using our tool. 

## Rigid alignment

1) The template is translated such that the mean of its vertices is in (0,0,0).
2) The template is rescaled such that the average distance of its landmarks from the mean landmark is the same as the scan.
3) The template and scan are translated so that their mean landmark is in (0,0,0).
4) The scan is rotated on the template with the best rotation that aligns their landmarks. We chose to rotate the scan on the template and not viceversa in order to then have warped templates having all the same orientation in space, which is required for PCA.

![Alignment](screenshots/alignment.png)

## Warping

1) We use an iterative method to warp the template to the scan.
2) We update the template vertices through an energy minimization procedure. We use the following energy:

<img src="https://latex.codecogs.com/svg.latex?E_{warp}=||Lx'-Lx||^2+\lambda||\operatorname{Id}_{constr}x'-c||^2" />

where:
* x : current template vertices.
* x' : new template vertices to be computed.
* c : constraints.
* <img src="https://latex.codecogs.com/svg.latex?\lambda" /> : hyperparameter that controls the relative weighting of smoothing term and constraints term.

We experimented with different constraints, such as template boundary, landmarks and points whose distance to the target mesh is smaller than a user defined threshold. In the end, we only kept the template boundary and close enough points as constraints.

For a more detailed explanation on the energy minimization and constraints, see the [Warping Theory file](Warping_Theory.pdf)

* template and target scan.

![warping](screenshots/warp1.png)

* template and warping results after 1 and 5 iterations.

![warping](screenshots/warp2.png)

## PCA

Once we have the aligned faces, we can represent each face as a 3nx1 vector, where n is the number of points for each face.


## Learning

### Overview
- Implemented with PyTorch
- Generating test data: warping script and PCA based generator 
- Tried COMA and PointNet
- Implemented our own approach, inspired by PointNet
- Integrated into pipeline to visualize
  - Custom faces: Interactive sliders to tweak latents
  - Morph faces: linearly interpolate latents
  - Random faces: Generate random latents and reconstruct from there

### Our Model

#### Architecture

1. `point_net_encoder`: Feature transform from `nx3` to `n` using the PointNet Architecture
2. `fully_connected_encoder` Fully connected network to reduce dimension to `10`, using ReLU and Dropout
3. `decoder` Fully connected decoder using ReLU and BatchNorm normalization

#### Dataflow
- Encode: `input` -> `point_net_encoder` -> `fully_connected_encoder` -> `latents`
- Decode: `latents` -> `decoder` -> `output` 

#### Details
For more details see [autoencoder.py](./learning/model/autoencoder.py) in this git repository

### Comparison to PCA

- Learning is slower
- But, faces look more realistic
  - Non linear transformation
- Learning approach works for all kinds of Point Clouds
  - Only testing data was specific for faces

### Screenshots

TODO
