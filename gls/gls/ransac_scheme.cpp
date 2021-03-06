#include "stdafx.h"

#include <cmath>
#include <algorithm>
#include <vector>
#include <tuple>
#include <string>
#include <iterator>
#include "Patate/grenaille.h"
#include "Eigen/Eigen"
#include "ransac_scheme.h"


Scalar RansacScheme::compute_points_dist(Point& p1, Point& p2)
{
	return sqrt(((p1.pos().transpose() - p2.pos().transpose())*(p1.pos() - p2.pos())).sum());
}


/*void RansacScheme::find_2_farthest_pairs(RansacScheme::triplet& triplet)
{
	Scalar dist2 = 0.0;
	Scalar dist3 = 0.0;
	std::tuple<Point, Point, Scalar, Scalar> pair2, pair3;
	Point p1 = std::get<0>(triplet.pair1);
	queue_copy_ = *queue_ptr_;

	while(!queue_copy_.empty())
	{
		std::tuple<Point, Point, Scalar, Scalar> pair = queue_copy_.top();
		queue_copy_.pop();
		if (is_q_unique(triplet, pair))
		{
			Point p = std::get<0>(pair);
			Scalar dist = compute_points_dist(p1, p);
			if (dist > dist2)
			{
				triplet.pair3 = triplet.pair2;
				dist3 = dist2;
				triplet.pair2 = pair;
				dist2 = dist;
			}
			else
			{
				if (dist > dist3)
				{
					triplet.pair3 = pair;
					dist3 = dist;
				}
			}
		}
	}

}*/

void RansacScheme::find_2_farthest_pairs(RansacScheme::triplet& triplet)
{
	// get second pair
	triplet.pair2 = find_farthest_pair(triplet.pair1);

	// get third pair
	triplet.pair3 = find_pair_to_maximize_triangle_surface(triplet.pair1, triplet.pair2);

}



std::tuple<Point, Point, Scalar, Scalar> RansacScheme::find_farthest_pair(std::tuple<Point, Point, Scalar, Scalar>& pair)
{
	Scalar dist_ref = 0.0;
	std::tuple<Point, Point, Scalar, Scalar> farthest_pair;
	Point p = std::get<0>(pair);
	queue_copy_ = *queue_ptr_;

	while (!queue_copy_.empty())
	{
		std::tuple<Point, Point, Scalar, Scalar> query_pair = queue_copy_.top();
		queue_copy_.pop();
		Point p_query = std::get<0>(query_pair);
		Scalar dist = compute_points_dist(p, p_query);
		if (dist > dist_ref && !(std::get<1>(query_pair).pos() == std::get<1>(pair).pos()))
		{
			dist_ref = dist_ref;
			farthest_pair = query_pair;
		}
	}

	return farthest_pair;
}

std::tuple<Point, Point, Scalar, Scalar> RansacScheme::find_pair_to_maximize_triangle_surface(std::tuple<Point, Point, Scalar, Scalar>& pair1, std::tuple<Point, Point, Scalar, Scalar>& pair2)
{
	Point p1 = std::get<0>(pair1);
	Point p2 = std::get<0>(pair2);
	std::tuple<Point, Point, Scalar, Scalar> pair3;
	Scalar max_area = 0.0;
	Scalar a = compute_points_dist(p1, p2);
	queue_copy_ = *queue_ptr_;

	while (!queue_copy_.empty())
	{
		std::tuple<Point, Point, Scalar, Scalar> pair = queue_copy_.top();
		queue_copy_.pop();
		if (!(std::get<1>(pair1).pos() == std::get<1>(pair).pos() || std::get<1>(pair2).pos() == std::get<1>(pair).pos()))
		{
			// compute area of the triangle formed by the point std::get<1>(pair1), std::get<1>(pair2), std::get<1>(pair)
			Scalar b = compute_points_dist(p1, std::get<0>(pair));
			Scalar c = compute_points_dist(p2, std::get<0>(pair));
			Scalar area = sqrt((a+b+c)*(b+c-a)*(a+c-b)*(a+b-c))/4.0;
			if (area > max_area)
			{
				max_area = area;
				pair3 = pair;
			}
		}

	}

	return pair3;

}



void RansacScheme::find_2_farthest_pairs_with_respect_to_geomVar(RansacScheme::triplet& triplet, Scalar bbox_diag_ratio)
{
	queue_copy_ = *queue_ptr_;

	// get second pair
	while (!queue_copy_.empty())
	{
		std::tuple<Point, Point, Scalar, Scalar> pair = queue_copy_.top();
		queue_copy_.pop();
		if (compute_points_dist(std::get<0>(triplet.pair1), std::get<0>(pair)) > bbox_diag_ratio && !(std::get<1>(triplet.pair1).pos() == std::get<1>(pair).pos()))
		{
			triplet.pair2 = pair;
			break;
		}
	}
		// empty queue
	while (!queue_copy_.empty())
		queue_copy_.pop();


	// get third pair
	queue_copy_ = *queue_ptr_;
	Scalar a = compute_points_dist(std::get<0>(triplet.pair1), std::get<0>(triplet.pair2));
	Scalar max_area = sqrt(3.0) / 4.0* bbox_diag_ratio*bbox_diag_ratio;
	while (!queue_copy_.empty())
	{
		std::tuple<Point, Point, Scalar, Scalar> pair = queue_copy_.top();
		queue_copy_.pop();
		if (!(std::get<1>(triplet.pair1).pos() == std::get<1>(pair).pos() || std::get<1>(triplet.pair2).pos() == std::get<1>(pair).pos()))
		{
			// compute area of the triangle formed by the point std::get<1>(pair1), std::get<1>(pair2), std::get<1>(pair)
			Scalar b = compute_points_dist(std::get<0>(triplet.pair1), std::get<0>(pair));
			Scalar c = compute_points_dist(std::get<0>(triplet.pair2), std::get<0>(pair));
			Scalar area = sqrt((a + b + c)*(b + c - a)*(a + c - b)*(a + b - c)) / 4.0;
			if (area > max_area)
			{
				triplet.pair3 = pair;
				break;
			}
		}
	}

	// empty queue
	while (!queue_copy_.empty())
		queue_copy_.pop();

}


RansacScheme::RansacScheme(pair_priority_queue& pair_prioritized) : queue_ptr_(pair_prioritized.get_queue_ptr()) {}

RansacScheme::triplet RansacScheme::pop_triplet()
{
	RansacScheme::triplet triplet;
	bool is_triplet_complete = false;

	while (queue_ptr_->size() > 2 && !is_triplet_complete)
	{
		// get first pair
		triplet.pair1 = queue_ptr_->top();
		queue_ptr_->pop();

		//get two other pairs
		queue_copy_ = *queue_ptr_;
		int counter = 1;
		while (!queue_copy_.empty())
		{
			std::tuple<Point, Point, Scalar, Scalar> pair = queue_copy_.top();
			queue_copy_.pop();
			if (!(std::get<0>(pair) == std::get<0>(triplet.pair1) || std::get<1>(pair) == std::get<1>(triplet.pair1)))
			{
				if (counter == 1)
				{
					triplet.pair2 = pair;
					counter += 1;
				}
				if (counter == 2)
				{
					if (!(std::get<0>(pair) == std::get<0>(triplet.pair2) || std::get<1>(pair) == std::get<1>(triplet.pair2)))
					{
						triplet.pair3 = pair;
						is_triplet_complete = true;
						break;
					}
				}
				
			}
		}

		// empty queue copy
		while (!queue_copy_.empty())
			queue_copy_.pop();
	}

	return triplet;
}


RansacScheme::triplet RansacScheme::pick_triplet(const int ind1, const int ind2, const int ind3)
{
	queue_copy_ = *queue_ptr_;
	RansacScheme::triplet triplet;
	int counter = 0;
	while (!queue_copy_.empty())
	{
		if (counter + 1 == ind1)
			triplet.pair1 = queue_copy_.top();
		else
		{
			if (counter + 1 == ind2)
				triplet.pair2 = queue_copy_.top();
			else
			{
				if (counter + 1 == ind3)
					triplet.pair3 = queue_copy_.top();
			}
		}

		queue_copy_.pop();
		counter += 1;
	 }

	return triplet;
}

RansacScheme::triplet RansacScheme::pop_3_farthest_pairs()
{
	RansacScheme::triplet triplet;

	if (queue_ptr_->size() > 2)
	{
		triplet.pair1 = queue_ptr_->top();
		queue_ptr_->pop();
		
		find_2_farthest_pairs(triplet);
	}
	
	return triplet;
}


RansacScheme::triplet RansacScheme::pop_3_farthest_pairs(Scalar bbox_diag_ratio)
{
	RansacScheme::triplet triplet;

	if (queue_ptr_->size() > 2)
	{
		triplet.pair1 = queue_ptr_->top();
		queue_ptr_->pop();

		find_2_farthest_pairs_with_respect_to_geomVar(triplet, bbox_diag_ratio);
	}

	return triplet;
}



std::pair<Scalar, Scalar> RansacScheme::scaleDiff(RansacScheme::triplet t)
{
	Scalar avgScale = (std::get<2>(t.pair1) + std::get<2>(t.pair2) + std::get<2>(t.pair3)) / 3.0;
	Scalar varScale = (abs(std::get<2>(t.pair1) - avgScale) + abs(std::get<2>(t.pair2) - avgScale) + abs(std::get<2>(t.pair3) - avgScale)) / 3.0;
	return std::make_pair(varScale, avgScale);
}


/*for that function, we follow the tutorial http://nghiaho.com/?page_id=671 */
Eigen::Matrix4d RansacScheme::compute_rigid_transform(RansacScheme::triplet t)
{

	// compute scale (target/source)
	Scalar scale = compute_scale(t);

	Eigen::Matrix3d scaling_mat = Eigen::Matrix3d::Zero();
	scaling_mat(0, 0) = 1.0 / scale;
	scaling_mat(1, 1) = 1.0 / scale;
	scaling_mat(2, 2) = 1.0 / scale;

	// points 
	VectorType target_pos1 = std::get<0>(t.pair1).pos();
	VectorType source_pos1 = std::get<1>(t.pair1).pos();
	VectorType target_pos2 = std::get<0>(t.pair2).pos();
	VectorType source_pos2 = std::get<1>(t.pair2).pos();
	VectorType target_pos3 = std::get<0>(t.pair3).pos();
	VectorType source_pos3 = std::get<1>(t.pair3).pos();

	// target and source data
	Eigen::Matrix3d source_data, target_data;
	source_data.col(0) = Eigen::Map<Eigen::ArrayXd>(source_pos1.data(), 3);
	source_data.col(1) = Eigen::Map<Eigen::ArrayXd>(source_pos2.data(), 3);
	source_data.col(2) = Eigen::Map<Eigen::ArrayXd>(source_pos3.data(), 3);
	target_data.col(0) = Eigen::Map<Eigen::ArrayXd>(target_pos1.data(), 3);
	target_data.col(1) = Eigen::Map<Eigen::ArrayXd>(target_pos2.data(), 3);
	target_data.col(2) = Eigen::Map<Eigen::ArrayXd>(target_pos3.data(), 3);

	// rescale target data
	target_data = scaling_mat*target_data;

	//computing centroids of both datasets
	Eigen::Vector3d centroid_source, centroid_target;
	centroid_source << (source_pos1(0,0) + source_pos2(0,0) + source_pos3(0,0)) / 3.0, (source_pos1(1,0) + source_pos2(1,0) + source_pos3(1,0)) / 3.0, (source_pos1(2,0) + source_pos2(2,0) + source_pos3(2,0)) / 3.0;
	centroid_target << (target_pos1(0,0) + target_pos2(0,0) + target_pos3(0,0)) / 3.0, (target_pos1(1,0) + target_pos2(1,0) + target_pos3(1,0)) / 3.0, (target_pos1(2,0) + target_pos2(2,0) + target_pos3(2,0)) / 3.0;

	// shift datasets to their respective centers and find rotation 
	Eigen::Matrix3d H, U, V, R;
	H = (target_data.colwise() - centroid_target)*(source_data.colwise() - centroid_source).transpose();
	Eigen::JacobiSVD<Eigen::Matrix3d> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
	U = svd.matrixU();
	V = svd.matrixV();
	R = V*U.transpose();
		//handling special reflection case
	if (R.determinant() < 0.0)
	{
		V.col(2) = -1.0*V.col(2);
		R = V*U.transpose();
	}

	// find translation
	Eigen::Vector3d translation;
	translation = source_data.col(0) - (R*target_data).col(0); 

	// global transform 
	Eigen::Matrix4d transform = Eigen::Matrix4d::Zero(4, 4);
	transform.block(0, 0, 3, 3) = R*scaling_mat;
	transform.block(0, 3, 3, 1) = translation;
	transform(3, 3) = 1.0;

	//std::cout << "transform : " << transform << std::endl;

	return transform;
}


/* not sure this is the proper way to solve registration problem for affine transformation with 4 matching points. 
The method used (by solving a linear system) returns a 4-by-4 matrix (with coefficients for rotation, translation and scaling) but solves the 12 unknown as if they were decorrelated 
In fact, it is not quite the case. Some of the terms of the matrix are related to each other and we have only 10 unknowns [1 theta, 3 rot axis, 3 translation, 3 scaling ](and not 12)*/
Eigen::Matrix4d RansacScheme::compute_rigid_transform(RansacScheme::triplet t, std::tuple<Point, Point, Scalar, Scalar> q)
{
	// 4 pairs of points 
	VectorType target_pos1 = std::get<0>(t.pair1).pos();
	VectorType source_pos1 = std::get<1>(t.pair1).pos();
	VectorType target_pos2 = std::get<0>(t.pair2).pos();
	VectorType source_pos2 = std::get<1>(t.pair2).pos();
	VectorType target_pos3 = std::get<0>(t.pair3).pos();
	VectorType source_pos3 = std::get<1>(t.pair3).pos();
	VectorType target_q_pos = std::get<0>(q).pos();
	VectorType source_q_pos = std::get<1>(q).pos();

	// target and source data
	Eigen::Matrix4d source_data = Eigen::Matrix4d::Ones(4, 4);
	Eigen::Matrix4d target_data = Eigen::Matrix4d::Ones(4, 4);
	source_data.block(0, 0, 3, 1) = Eigen::Map<Eigen::ArrayXd>(source_pos1.data(), 3);
	source_data.block(0, 1, 3, 1) = Eigen::Map<Eigen::ArrayXd>(source_pos2.data(), 3);
	source_data.block(0, 2, 3, 1) = Eigen::Map<Eigen::ArrayXd>(source_pos3.data(), 3);
	source_data.block(0, 3, 3, 1) = Eigen::Map<Eigen::ArrayXd>(source_q_pos.data(), 3);

	target_data.block(0, 0, 3, 1) = Eigen::Map<Eigen::ArrayXd>(target_pos1.data(), 3);
	target_data.block(0, 1, 3, 1) = Eigen::Map<Eigen::ArrayXd>(target_pos2.data(), 3);
	target_data.block(0, 2, 3, 1) = Eigen::Map<Eigen::ArrayXd>(target_pos3.data(), 3);
	target_data.block(0, 3, 3, 1) = Eigen::Map<Eigen::ArrayXd>(target_q_pos.data(), 3);

	// solve linear system 
	Eigen::Matrix4d transform_transpose = (target_data.transpose()).colPivHouseholderQr().solve(source_data.transpose());


	//std::cout << "transform : " << transform_transpose.transpose() << std::endl;

	return transform_transpose.transpose();
}


Scalar RansacScheme::registrationErr(Eigen::Matrix4d transform, RansacScheme::triplet& triplet)
{
	Eigen::Matrix3d R = transform.block(0, 0, 3, 3);
	Eigen::Vector3d T = transform.block(0, 3, 3, 1);
	Scalar err1 = ((Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair1).pos().data(), 3, 1) - R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair1).pos().data(), 3, 1) - T).cwiseAbs()).sum();
	Scalar err2 = ((Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair2).pos().data(), 3, 1) - R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair2).pos().data(), 3, 1) - T).cwiseAbs()).sum();
	Scalar err3 = ((Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair3).pos().data(), 3, 1) - R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair3).pos().data(), 3, 1) - T).cwiseAbs()).sum();

	//std::cout << "reg err : " << (err1 + err2 + err3) / 3.0;
	return (err1 + err2 + err3) / 3.0;
}

Scalar RansacScheme::registrationErr(Eigen::Matrix4d transform, RansacScheme::triplet& triplet, std::tuple<Point, Point, Scalar, Scalar>& q)
{
	Eigen::Matrix3d R = transform.block(0, 0, 3, 3);
	Eigen::Vector3d T = transform.block(0, 3, 3, 1);
	Scalar err1 = ((Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair1).pos().data(), 3, 1) - R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair1).pos().data(), 3, 1) - T).cwiseAbs()).sum();
	Scalar err2 = ((Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair2).pos().data(), 3, 1) - R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair2).pos().data(), 3, 1) - T).cwiseAbs()).sum();
	Scalar err3 = ((Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair3).pos().data(), 3, 1) - R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair3).pos().data(), 3, 1) - T).cwiseAbs()).sum();
	Scalar errq = ((Eigen::Map<Eigen::MatrixXd>(std::get<1>(q).pos().data(), 3, 1) - R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(q).pos().data(), 3, 1) - T).cwiseAbs()).sum();
	
	//std::cout << "reg err : " << (err1 + err2 + err3 + errq) / 4.0;
	return (err1 + err2 + err3 + errq) / 4.0;
}

Scalar RansacScheme::compute_angle(VectorType v1, VectorType v2)
{
	Scalar dot_product = v1.dot(v2);
	Scalar norm_v1 = v1.norm();
	Scalar norm_v2 = v2.norm();
	return acos(dot_product / (norm_v1*norm_v2)); // en radians
}

Scalar RansacScheme::normalErr(Eigen::Matrix4d transform, RansacScheme::triplet& triplet)
{

	Eigen::Matrix3d R = transform.block(0, 0, 3, 3);
	Scalar err1 = compute_angle(Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair1).normal().data(), 3, 1) ,  R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair1).normal().data(), 3, 1));
	Scalar err2 = compute_angle(Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair2).normal().data(), 3, 1), R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair2).normal().data(), 3, 1));
	Scalar err3 = compute_angle(Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair3).normal().data(), 3, 1), R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair3).normal().data(), 3, 1));

	//std::cout << "  normal err : " << (err1 + err2 + err3) / 3.0 << std::endl;
	return (err1 + err2 + err3) / 3.0;
}

Scalar RansacScheme::normalErr(Eigen::Matrix4d transform, RansacScheme::triplet& triplet, std::tuple<Point, Point, Scalar, Scalar>& q)
{

	Eigen::Matrix3d R = transform.block(0, 0, 3, 3);
	Scalar err1 = compute_angle(Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair1).normal().data(), 3, 1), R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair1).normal().data(), 3, 1));
	Scalar err2 = compute_angle(Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair2).normal().data(), 3, 1), R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair2).normal().data(), 3, 1));
	Scalar err3 = compute_angle(Eigen::Map<Eigen::MatrixXd>(std::get<1>(triplet.pair3).normal().data(), 3, 1), R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(triplet.pair3).normal().data(), 3, 1));
	Scalar errq = compute_angle(Eigen::Map<Eigen::MatrixXd>(std::get<1>(q).normal().data(), 3, 1), R*Eigen::Map<Eigen::MatrixXd>(std::get<0>(q).normal().data(), 3, 1));

	//std::cout << "  normal err : " << (err1 + err2 + err3 + errq) / 4.0 << std::endl;
	return (err1 + err2 + err3 + errq) / 4.0;
}


bool RansacScheme::is_q_unique(RansacScheme::triplet t, std::tuple<Point, Point, Scalar, Scalar> q)
{
	/*std::cout << "target point 1 : " << std::get<0>(t.pair1).pos().transpose() << " , target point 2 : " << std::get<0>(t.pair2).pos().transpose() << " , target point 3 : " << std::get<0>(t.pair3).pos().transpose() << std::endl;
	std::cout << "source point 1 : " << std::get<1>(t.pair1).pos().transpose() << " , source point 2 : " << std::get<1>(t.pair2).pos().transpose() << " , source point 3 : " << std::get<1>(t.pair3).pos().transpose() << std::endl;
	std::cout << "target q : " << std::get<0>(q).pos().transpose() << " , source q : " << std::get<1>(q).pos().transpose() << std::endl;*/

	if (std::get<0>(t.pair1).pos() == std::get<0>(q).pos() || std::get<0>(t.pair2).pos() == std::get<0>(q).pos() || std::get<0>(t.pair3).pos() == std::get<0>(q).pos())
		return false;
	if (std::get<1>(t.pair1).pos() == std::get<1>(q).pos() || std::get<1>(t.pair2).pos() == std::get<1>(q).pos() || std::get<1>(t.pair3).pos() == std::get<1>(q).pos())
		return false;
	return true;
}


bool RansacScheme::is_valid(std::tuple<Point, Point, Scalar, Scalar> q, RansacScheme::triplet t, Scalar max_err_reg, Scalar max_err_norm)
{
	if (!is_q_unique(t, q))
		return false;
	Eigen::Matrix4d transform = compute_rigid_transform(t, q);
	Scalar reg_err = registrationErr(transform, t, q);
	Scalar norm_err = normalErr(transform, t, q);
	if (!(reg_err < max_err_reg && norm_err < max_err_norm))
		return false;
	else
		return true;

}


Scalar RansacScheme::compute_scale(RansacScheme::triplet& triplet)
{
	Point ps1, ps2, ps3, pt1, pt2, pt3;
	Scalar a, b, c, A, B, C;
	pt1 = std::get<0>(triplet.pair1);
	ps1 = std::get<1>(triplet.pair1);
	pt2 = std::get<0>(triplet.pair2);
	ps2 = std::get<1>(triplet.pair2);
	pt3 = std::get<0>(triplet.pair3);
	ps3 = std::get<1>(triplet.pair3);

	a = compute_points_dist(pt1, pt2);
	b = compute_points_dist(pt2, pt3);
	c = compute_points_dist(pt1, pt3);
	A = compute_points_dist(ps1, ps2);
	B = compute_points_dist(ps2, ps3);
	C = compute_points_dist(ps1, ps3);

	Scalar scale = (a/A + b/B + c/C) / 3.0;
	return scale;
}


Eigen::Matrix4d RansacScheme::ransac_algorithm(int nb_iterations, Scalar max_err_scale, Scalar max_err_reg, Scalar max_err_norm)
{
	int counter = 0;
	Scalar bbox_diag_ratio = 8.0;
	while (counter < nb_iterations && queue_ptr_->size() > 2)
	{
		//triplet t = pop_triplet();
		triplet t = pop_3_farthest_pairs(bbox_diag_ratio);
		//triplet t = pick_triplet(8, 9, 23);
		/*std::cout << "triplet : " << std::endl;
		std::cout << std::get<0>(t.pair1).pos().transpose() << " , " << std::get<1>(t.pair1).pos().transpose() << " , " << std::get<2>(t.pair1) << std::endl;
		std::cout << std::get<0>(t.pair2).pos().transpose() << " , " << std::get<1>(t.pair2).pos().transpose() << " , " << std::get<2>(t.pair2) << std::endl;
		std::cout << std::get<0>(t.pair3).pos().transpose() << " , " << std::get<1>(t.pair3).pos().transpose() << " , " << std::get<2>(t.pair3) << std::endl;*/
		Scalar err_scale = scaleDiff(t).first;
		//std::cout << "err scale : " << err_scale << std::endl;
		if (err_scale < max_err_scale)
		{
			//Scalar avgScale = scaleDiff(t).second;
			Eigen::Matrix4d M = compute_rigid_transform(t);
			
			if (registrationErr(M, t) < max_err_reg && normalErr(M, t) < max_err_norm)
			{
				queue_copy_ = *queue_ptr_;
				while (!queue_copy_.empty())
				{
					std::tuple<Point, Point, Scalar, Scalar> q = queue_copy_.top();
					queue_copy_.pop();
					if (is_valid(q, t, max_err_reg, max_err_norm))
						return compute_rigid_transform(t, q);
				}
			}

		}
		counter += 1;
	}
	return Eigen::Matrix4d::Identity();
}