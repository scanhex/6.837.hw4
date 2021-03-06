#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"

namespace GLOO {
void Tracer::Render(const Scene& scene,
                    const std::string& output_file) {
  scene_ptr_ = &scene;

  auto& root = scene_ptr_->GetRootNode();
  tracing_components_ = root.GetComponentPtrsInChildren<TracingComponent>();
  light_components_ = root.GetComponentPtrsInChildren<LightComponent>();


  Image image(image_size_.x, image_size_.y);

  for (int y = 0; y < image_size_.y; y++) {
    for (int x = 0; x < image_size_.x; x++) {
      auto ray = camera_.GenerateRay({2.0f * x / image_size_.x - 1, 2.0f * y/image_size_.y - 1});
      HitRecord record;
      image.SetPixel(x, y, TraceRay(ray, 0, record));
    }
  }

  if (output_file.size())
    image.SavePNG(output_file);
}


glm::vec3 Tracer::TraceRay(const Ray& ray,
                           size_t bounces,
                           HitRecord& record) const {
  glm::vec3 res = GetBackgroundColor(ray.GetDirection());
  if (bounces > max_bounces_)
    return res;
  TracingComponent *hit = nullptr;
  for (const auto &object : tracing_components_) {
    auto transf = object->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
    auto ray_local = ray;
    ray_local.ApplyTransform(glm::inverse(transf));
    if (object->GetHittable().Intersect(ray_local, 0, record)) {
      record.normal = glm::normalize(glm::vec3(glm::inverse(glm::transpose(transf)) * glm::vec4(record.normal, 0.0f)));
      hit = object;
    }
  }
  if (!hit)
    return res;
  const auto &object = hit;
  auto hit_pos = ray.At(record.time);
  auto material =
      object->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial();
  res = {0, 0, 0};
  auto eye = glm::normalize(ray.GetDirection());
  auto R = glm::reflect(eye, record.normal);
  constexpr float eps = 1e-3;
  for (const auto &x : light_components_) {
    glm::vec3 dir_to_light;
    glm::vec3 intensity;
    float dist_to_light;
    Illuminator::GetIllumination(*x, hit_pos, dir_to_light, intensity,
                                 dist_to_light);
    if (x->GetLightPtr()->GetType() == LightType::Ambient) {
      res += intensity * material.GetAmbientColor();
    } else {
      if (shadows_enabled_) {
        HitRecord shadow_record;
        Tracer::TraceRay(Ray(hit_pos + dir_to_light * eps, dir_to_light), max_bounces_, shadow_record);
        if (shadow_record.time < dist_to_light)
          continue;
      }
      glm::vec3 I_diffuse =
          std::max(0.0f, glm::dot(dir_to_light, record.normal)) * intensity;
      res += I_diffuse * material.GetDiffuseColor();

      glm::vec3 I_specular =
          (float)pow(std::max(0.0f, glm::dot(dir_to_light, R)),
                     material.GetShininess()) *
          intensity;
      res += I_specular * material.GetSpecularColor();
    }
  }
  if (glm::length(material.GetSpecularColor()) > eps && bounces + 1 < max_bounces_)
  {
    HitRecord new_record;
    res += Tracer::TraceRay(Ray(hit_pos + R * eps, R), bounces + 1, new_record) * material.GetSpecularColor();
  }
  return res;
}


glm::vec3 Tracer::GetBackgroundColor(const glm::vec3& direction) const {
  if (cube_map_ != nullptr) {
    return cube_map_->GetTexel(direction);
  } else
    return background_color_;
}
}  // namespace GLOO
