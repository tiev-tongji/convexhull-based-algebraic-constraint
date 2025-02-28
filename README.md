# Convex Hull-based Algebraic Constraint for Visual Quadric SLAM

* We first open source the early version of the code．
* A detailed readme will be released later.

## Abstract
Using Quadrics as the object representation has the beneﬁts of both generality and closed-form projection derivation between image and world spaces. Although numerous constraints have been proposed for dual quadric reconstruction, we found that many of them are imprecise and provide minimal improvements to localization. After scrutinizing the existing constraints, we introduce a concise yet more precise
convex hull-based algebraic constraint for object landmarks, which is applied to object reconstruction, frontend pose estimation, and backend bundle adjustment. This constraint is designed to fully leverage precise semantic segmentation, effectively mitigating mismatches between complex-shaped object contours and dual quadrics. Experiments on public datasets demonstrate that our approach is applicable to both monocular and RGB-D SLAM and achieves improved object mapping and localization than existing quadric SLAM methods.
