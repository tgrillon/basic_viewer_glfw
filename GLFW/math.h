#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
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

Eigen::Matrix4f eulerAngleXY(float const& angleX, float const& angleY)
{
  float cosX = std::cos(angleX);
  float sinX = std::sin(angleX);
  float cosY = std::cos(angleY);
  float sinY = std::sin(angleY);

  Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
  result(0,0) = cosY;
  result(1,0) = -sinX * -sinY;
  result(2,0) = cosX * -sinY;
  result(1,1) = cosX;
  result(2,1) = sinX;
  result(0,2) = sinY;
  result(1,2) = -sinX * cosY;
  result(2,2) = cosX * cosY;
  
  return result;
}

Eigen::Matrix4f perspective(float fov, float aspect, float zNear, float zFar)
{
  assert(std::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0);

  const float tanHalfFov = std::tan(fov * 0.5);
  Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
  result(0, 0) = 1.0 / (aspect * tanHalfFov); 
  result(1, 1) = 1.0 / (tanHalfFov); 
  result(2, 2) = - (zFar + zNear) / (zFar - zNear); 
  result(2, 3) = - (2.0 * zFar * zNear) / (zFar - zNear); 
  result(3, 2) = - 1.0; 

  return result;
}

Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
  Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
  result(0, 0) = 2.0 / (right - left);
  result(1, 1) = 2.0 / (top - bottom);
  result(2, 2) = - 2.0 / (zFar - zNear);
  result(0, 3) = - (right + left) / (right - left);
  result(1, 3) = - (top + bottom) / (top - bottom);
  result(2, 3) = - (zFar + zNear) / (zFar - zNear);
  
  return result;
}

Eigen::Matrix4f lookAt(Eigen::Vector3f const& eye, Eigen::Vector3f const& center, Eigen::Vector3f const& up)
{
  const Eigen::Vector3f dir((center - eye).normalized());
  const Eigen::Vector3f right(dir.cross(up.normalized()).normalized());
  const Eigen::Vector3f newUp(right.cross(dir));
 
  Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
  result(0, 0) = right.x();
  result(1, 0) = right.y();
  result(2, 0) = right.z();
  result(0, 1) = newUp.x();
  result(1, 1) = newUp.y();
  result(2, 1) = newUp.z();
  result(0, 2) =-dir.x();
  result(1, 2) =-dir.y();
  result(2, 2) =-dir.z();
  result(0, 3) =-right.dot(eye);
  result(1, 3) =-newUp.dot(eye);
  result(2, 3) = dir.dot(eye);
  return result;
}