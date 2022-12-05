#ifndef GLOO_PHONG_SHADER_2_H_
#define GLOO_PHONG_SHADER_2_H_

#include "ShaderProgram.hpp"

namespace GLOO {
class PhongShader2 : public ShaderProgram {
 public:
  PhongShader2();
  void SetTargetNode(const SceneNode& node,
                     const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;
  void SetLightSource(const LightComponent& componentt) const override;


 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
};
}  // namespace GLOO

#endif
