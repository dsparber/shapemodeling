# Report - Game of Faces

## Preprocessing

## Landmarks

## Rigid alignment

1) The template is translated such that the mean of its vertices is in (0,0,0).
2) The template is rescaled such that the average distance of its landmarks from the mean landmark is the same as the scan.
3) The template and scan are translated so that their mean landmark is in (0,0,0).
4) The scan is rotated on the template with the best rotation that aligns their landmarks. We chose to rotate the scan on the template and not viceversa in order to then have warped templates having all the same orientation in space, which is required for PCA.


## Warping

1) We use an iterative method to warp the template to the scan.
2) We update the template vertices through an energy minimization procedure. We use the following energy:
<img src="https://latex.codecogs.com/gif.latex?P(s | O_t )=\text { Probability of a sensor reading value when sleep onset is observed at a time bin } t " />

where:
* x : current template vertices.
* x' : new template vertices to be computed.
* c : constraints.
* lambda : hyperparameter that controls the relative weighting of smoothing term and constraints term.

We experimented with different constraints, such as template boundary, scan landmarks and scan points far less than a user defined threshold from the scan. In the end, we only kept the template boundary and points far less than some threshold as constraints.

## PCA

## Learning
