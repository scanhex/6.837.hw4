#include "Triangle.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>
#include <utility>

#include "Plane.hpp"

namespace GLOO {
Triangle::Triangle(const glm::vec3& p0,
                   const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& n0,
                   const glm::vec3& n1,
                   const glm::vec3& n2) : positions_{p0, p1, p2}, normals_{n0, n1, n2} { }

Triangle::Triangle(std::vector<glm::vec3> positions,
                   std::vector<glm::vec3> normals) : positions_(std::move(positions)), normals_(std::move(normals)) { }

bool Triangle::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  auto plane_norm = glm::normalize(cross(positions_[1] - positions_[0], positions_[2] - positions_[0]));
  auto plane_d = glm::dot(positions_[0], plane_norm);
  HitRecord plane_record;
  if (Plane(plane_norm, plane_d).Intersect(ray, t_min, plane_record) && plane_record.time < record.time) {
    auto pt = ray.At(plane_record.time);
    auto area = glm::dot(cross(positions_[1] - positions_[0], positions_[2] - positions_[0]), plane_norm);
    float ar[3];
    ar[0] = glm::dot(cross(positions_[1] - pt, positions_[2] - pt), plane_norm);
    ar[1] = glm::dot(cross(positions_[2] - pt, positions_[0] - pt), plane_norm);
    ar[2] = glm::dot(cross(positions_[0] - pt, positions_[1] - pt), plane_norm);
    if (ar[0] < 0 || ar[1] < 0 || ar[2] < 0)
      return false;
    ar[0] /= area;
    ar[1] /= area;
    ar[2] /= area;
    record.time = plane_record.time;
    record.normal = normals_[0] * ar[0] + normals_[1] * ar[1] + normals_[2] * ar[2];
    return true;
  }
  return false;
}
}  // namespace GLOO
