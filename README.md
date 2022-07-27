# GLS
descriptors GLS
This repository has been created to compute the GLS (growing least square) descriptor on point clouds/meshes. 
It is the implementation in C++ of the paper "Relative Scale Estimation and 3D Registration of Multi-Modal Geometry Using Growing Least Squares" written by Nicolas Melldo, Matteo Dellepiane and Roberto Scopigno. (https://www.researchgate.net/publication/286510263_Relative_Scale_Estimation_and_3D_Registration_of_Multi-Modal_Geometry_Using_Growing_Least_Squares)
Our code relies on some functions from the Patate library (avalaible on inria gitlab "https://gitlab.inria.fr/patate/patate"). 
This library have then been replaced by the PONCA library (https://www.irit.fr/recherches/STORM/MelladoNicolas/patate-library/)
We didn't test our code with the new version of the library. That's why we provided in zip file the code of the former library developped by Mellado and it's team. 
To make the GLS code works, the user must set the path to the Patate library in his C++ project. Please, refer to the README included in the Patate zip file to add external dependency (Eigen)

This project comes with data provided for testing purpose. 
Our main code is located in the file "/gls/compute_gls.cpp"
3 applications can be generated with our GLS code : 
   - 1st application : computing GLS descriptors. 
     Uncomment from line 138 to line 254 and Comment from line 256 to end
     Then, to test the code, use the data contained in the folder "data_example_1" and run the following command in a terminal: 
     bunny.ply  bunny_source_interest_point.ply  0.1292 4.9115 500 bunny_profiles.txt 1 
     The first parameter is the original mesh/point cloud
     The second parameter is a subset of keypoints from the original point cloud. GLS will be computed on these keypoints. 
     The third and fourth parameters are respectively the minimum and maximum scale (see "https://www.researchgate.net/publication/286510263_Relative_Scale_Estimation_and_3D_Registration_of_Multi-Modal_Geometry_Using_Growing_Least_Squares" p6)
     A good choice for the minimum scale is to take the minimum of the average edge length (for mesh) (or point distance (for point cloud)) between the two objects to register. 
     A good choice for the maximum scale is to take the bigest bounding box diagonal between the two objects to register. Another valid choice is to take the maximum of the average edge length between the two objects, and multiply it by a big consant (let's say 5). 
     The fifth parameter is the number of scale samples (see "https://www.researchgate.net/publication/286510263_Relative_Scale_Estimation_and_3D_Registration_of_Multi-Modal_Geometry_Using_Growing_Least_Squares" p6)
     The sixth parameter is the output filename where the algorithm will write the point profiles (i.e. the descriptor)
     and the optional last parameter is a boolean to set to 1 if we want the algorithm to compute and write the geometric variations. 
     
     -2nd application : Estimate the relative scale between the two object to register & Estimate the matching points between the two objects
      Uncomment from line 256 to line 331 and Comment from line 138 to 254 and from line 334 to end. 
      This application is the continuity of the first one. To make it works, the user must first compute the descriptors on the two objects A and B he wants to register. 
      So, he ill have to run 1rst application on object A, 1rst application on object B, and then use both the generated descriptor files in application 2. 
      The folder "data_example_2" contains the results that one would obtained if he ran application 1 on both source and target meshes contained "data_example_1"
      To test the code, use the data contained in the folder "data_example_2" and run the following command in a terminal: 
      bunny_vsa_complete_profiles.txt  bunny_downsampled_vsa_complete_profiles.txt  bunny_vsa_bunny_downsampled_vsa_matching.txt 1
      The first parameter is the descriptor file of object A (in our case source_bunny). 
      The second parameter is the descriptor file of object B (in our case target_bunny).
      The third parameter is the output filename where the algorithm will write the corresponding pair of points and the estimated scale factor. 
      The last parameter is optional and must be consistant with what you choose for last parameter in application 1. If you didnt add the optional parameter in application 1, don't add it here. Otherwise, do so. 
      
      
     
     

