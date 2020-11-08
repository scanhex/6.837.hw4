#include "Illuminator.hpp"

#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>

#include "gloo/lights/AmbientLight.hpp"
#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/SceneNode.hpp"

namespace GLOO {
void Illuminator::GetIllumination(const LightComponent& light_component,
                                  const glm::vec3& hit_pos,
                                  glm::vec3& dir_to_light,
                                  glm::vec3& intensity,
                                  float& dist_to_light) {
  // Calculation will be done in world space.

  auto light_ptr = light_component.GetLightPtr();
  if (light_ptr->GetType() == LightType::Directional) {
    auto directional_light_ptr = dynamic_cast<DirectionalLight*>(light_ptr);
    dir_to_light = glm::normalize(-directional_light_ptr->GetDirection());
    intensity = directional_light_ptr->GetDiffuseColor();
    dist_to_light = std::numeric_limits<float>::max();
  } else if (light_ptr->GetType() == LightType::Point) {
    auto point_light_ptr = dynamic_cast<PointLight*>(light_ptr);
    auto vec_to_light = light_component.GetNodePtr()->GetTransform().GetWorldPosition() - hit_pos;
    dir_to_light = glm::normalize(vec_to_light);
    dist_to_light = glm::length(vec_to_light);
    intensity = point_light_ptr->GetDiffuseColor() / (dist_to_light * dist_to_light * point_light_ptr->GetAttenuation());
  } else {
    auto ambient_light_ptr = dynamic_cast<AmbientLight*>(light_ptr);
    intensity = ambient_light_ptr->GetAmbientColor();
  }
}
}  // namespace GLOO
