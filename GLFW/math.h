#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
// #include <glm/glm.hpp>
#include <eigen3/Eigen/Core>

// https://en.wikipedia.org/wiki/Spherical_coordinate_system

// x = theta, y = phi 
Eigen::Vector3f sphericalToCartesian(const Eigen::Vector2f& view) {
    Eigen::Vector3f result;
    result << 
        sin(view.y()) * cos(view.x()),
        cos(view.y()),
        sin(view.y()) * sin(view.x());
    return result;
}

// with z = forward 
Eigen::Vector2f cartesianToSpherical(const Eigen::Vector3f& dir){
    float signz = dir.z() > 0 ? 1 : -1;

    Eigen::Vector2f result;
    result << 
        signz * acos(dir.x()/sqrt(dir.x() * dir.x() + dir.z() * dir.z())), // theta 
        acos(dir.y()); // phi
    return result;
}

float radians(float x) {
    return M_PI / 180 * x;
}

Eigen::Vector2f radians(const Eigen::Vector2f& v) {
    Eigen::Vector2f result;
    result << 
        radians(v.x()), 
        radians(v.y());
    return result;
}