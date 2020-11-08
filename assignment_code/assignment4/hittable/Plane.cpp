#include "Plane.hpp"

namespace GLOO {
Plane::Plane(const glm::vec3& normal, float d) : normal_(glm::normalize(normal)), d_(d) { }

bool Plane::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  float speed = glm::dot(ray.GetDirection(), normal_);
  if (abs(speed) < 1e-7)
    return false;
  float dist = d_ - glm::dot(ray.GetOrigin(), normal_);
  float t = dist / speed;
  if (t < t_min)
    return false;
  if (t < record.time) {
    record.time = t;
    record.normal = -normal_ * glm::sign(speed);
    return true;
  }
  return false;
}
}  // namespace GLOO
