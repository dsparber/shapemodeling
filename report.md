# Report - Game of Faces

## Our own scans

![wall](screenshots/got.png)

## Preprocessing

All the mesh boundaries have been smoothed using the bi-laplacian approach as in assignment 5.

![Preprocessing](screenshots/smooth.png)

## Landmarks

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

We experimented with different constraints, such as template boundary, scan landmarks and scan points far less than a user defined threshold from the scan. In the end, we only kept the template boundary and points far less than some threshold as constraints.

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
