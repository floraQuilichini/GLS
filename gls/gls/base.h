#pragma once
#include <cmath>
#include <algorithm>
#include <iostream>
#include "Patate/grenaille.h"
#include "Eigen/Eigen"

class Point
{
public:
	enum { Dim = 3 };
	typedef double Scalar;
	typedef Eigen::Matrix<Scalar, Dim, 1>   VectorType;
	typedef Eigen::Matrix<Scalar, Dim, Dim> MatrixType;
	typedef Eigen::Matrix<Scalar, Dim, 1>   VectorType;
	MULTIARCH inline Point(const VectorType& _pos = VectorType::Zero(), const VectorType& _normal = VectorType::Zero()) : m_pos(_pos), m_normal(_normal) {}
	MULTIARCH inline const VectorType& pos()    const { return m_pos; }
	MULTIARCH inline VectorType& pos() { return m_pos; }
	MULTIARCH inline const VectorType& normal()    const { return m_normal; }
	MULTIARCH inline VectorType& normal() { return m_normal; }
	bool inline operator==(const Point& p)
	{
		return (this->pos().x() == p.pos().x()) && (this->pos().x() == p.pos().x()) && (this->pos().x() == p.pos().x());
	}
	

private:
	VectorType m_pos, m_normal;
};


struct PointComp {
	bool operator()(const Point& p1, const Point& p2) const
	{
		if (p1.pos().x() < p2.pos().x())
			return true;
		else
		{
			if (p1.pos().x() == p2.pos().x())
			{
				if (p1.pos().y() < p2.pos().y())
					return true;
				else
				{
					if (p1.pos().x() == p2.pos().x())
					{
						if (p1.pos().z() < p2.pos().z())
							return true;
						else
							return false;
					}
					return false;
				}
			}
			return false;
		}
	}
};
