#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>

// https://en.wikipedia.org/wiki/Spherical_coordinate_system

// x = theta, y = phi 
glm::vec3 sphericalToCartesian(const glm::vec2& view) {
    return { 
        sin(view.y) * cos(view.x),
        cos(view.y),
        sin(view.y) * sin(view.x)
    };
}

// with z = forward 
glm::vec2 cartesianToSpherical(const glm::vec3& dir){
    float signz = dir.z > 0 ? 1 : -1;

    return {
        signz * acos(dir.x/sqrt(dir.x * dir.x + dir.z * dir.z)), // theta 
        acos(dir.y), // phi
    };
}