#pragma once
#include <vector>
#include <string>
#include <tuple>
#include "base.h"
#include "Eigen/Eigen"
#include "matching_prioritization.h"


class RansacScheme
{
private:
	std::priority_queue<std::tuple<Point, Point, Scalar, Scalar>, std::vector<std::tuple<Point, Point, Scalar, Scalar>>, mycomparison>* queue_ptr_;
	std::priority_queue<std::tuple<Point, Point, Scalar, Scalar>, std::vector<std::tuple<Point, Point, Scalar, Scalar>>, mycomparison> queue_copy_;

public:
	RansacScheme(pair_priority_queue& pair_prioritized);
	struct triplet {
		std::tuple<Point, Point, Scalar, Scalar> pair1;
		std::tuple<Point, Point, Scalar, Scalar> pair2;
		std::tuple<Point, Point, Scalar, Scalar> pair3;
	};
	triplet pop_triplet();
	std::pair<Scalar, Scalar> scaleDiff(triplet t);
	Eigen::Matrix4d compute_rigid_transform(triplet);
	Eigen::Matrix4d compute_rigid_transform(triplet, std::tuple<Point, Point, Scalar, Scalar> q);
	Scalar registrationErr(Eigen::Matrix4d transform, std::vector<std::tuple<Point, Point, Scalar, Scalar>>& pairs_source_target);
	Scalar compute_angle(VectorType v1, VectorType v2);
	Scalar normalErr(Eigen::Matrix4d transform, std::vector<std::tuple<Point, Point, Scalar, Scalar>>& pairs_source_target);
	bool is_q_unique(triplet t, std::tuple<Point, Point, Scalar, Scalar> q);
	bool is_valid(std::tuple<Point, Point, Scalar, Scalar> q, triplet t, std::vector<std::tuple<Point, Point, Scalar, Scalar>>& pairs_source_target, Scalar max_err_reg, Scalar max_err_norm);
	void rescale_data(std::vector<std::tuple<Point, Point, Scalar, Scalar>>& pairs_source_target, Scalar avgScale);
	Eigen::Matrix4d ransac_algorithm(int nb_iterations, Scalar max_err_scale, Scalar max_err_reg, Scalar max_err_norm, std::vector<std::tuple<Point, Point, Scalar, Scalar>>& pairs_source_target);
};