
#include <cassert>
#include <iostream>
#include <glad/glad.h>
#include <glm/gtx/string_cast.hpp>

#include "Application.hpp"
#include "Scene.hpp"
#include "utils.hpp"
#include "gl_wrapper/BindGuard.hpp"
#include "shaders/ShaderProgram.hpp"
#include "components/ShadingComponent.hpp"
#include "components/CameraComponent.hpp"
#include "debug/PrimitiveFactory.hpp"

namespace GLOO {
Renderer::Renderer(Application& application) : application_(application) {
  UNUSED(application_);
}

void Renderer::SetRenderingOptions() const {
  GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

  // Enable depth test.
  GL_CHECK(glEnable(GL_DEPTH_TEST));
  GL_CHECK(glDepthFunc(GL_LEQUAL));

  // Enable blending for multi-pass forward rendering.
  GL_CHECK(glEnable(GL_BLEND));
  GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
}

void Renderer::Render(const Scene& scene) const {
  SetRenderingOptions();
  RenderScene(scene);
}

Renderer::RenderingInfo Renderer::RetrieveRenderingInfo(
    const Scene& scene) const {
  RenderingInfo info;
  const SceneNode& root = scene.GetRootNode();

  glm::mat4 rootMatrix = root.GetTransform().GetLocalToParentMatrix();
  RecursiveRetrieve(root, info, rootMatrix);

  return info;
}

void Renderer::RecursiveRetrieve(const SceneNode& node, RenderingInfo& info, const glm::mat4& model_matrix) const {
  if (!node.IsActive()) return;
  auto rc = node.GetComponentPtr<RenderingComponent>();
  if (rc) info.emplace_back(node.GetComponentPtr<RenderingComponent>(), model_matrix);
  int c = node.GetChildrenCount();
  for (int i = 0; i < c; i++){
    const SceneNode& child = node.GetChild(i);
    glm::mat4 localTransformMatrix = child.GetTransform().GetLocalToParentMatrix();
    glm::mat4 updatedMatrix = model_matrix * localTransformMatrix;
    RecursiveRetrieve(child, info, updatedMatrix);
  }
}

void Renderer::RenderScene(const Scene& scene) const {
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  const SceneNode& root = scene.GetRootNode();
  auto rendering_info = RetrieveRenderingInfo(scene);
  auto light_ptrs = root.GetComponentPtrsInChildren<LightComponent>();
  if (light_ptrs.size() == 0) {
    // Make sure there are at least 2 passes of we don't forget to set color
    // mask back.
    return;
  }

  CameraComponent* camera = scene.GetActiveCameraPtr();

  {
    // Here we first do a depth pass (note that this has nothing to do with the
    // shadow map). The goal of this depth pass is to exclude pixels that are
    // not really visible from the camera, in later rendering passes. You can
    // safely leave this pass here without understanding/modifying it, for
    // assignment 5. If you are interested in learning more, see
    // https://www.khronos.org/opengl/wiki/Early_Fragment_Test#Optimization

    GL_CHECK(glDepthMask(GL_TRUE));
    bool color_mask = GL_FALSE;
    GL_CHECK(glColorMask(color_mask, color_mask, color_mask, color_mask));

    for (const auto& pr : rendering_info) {
      auto robj_ptr = pr.first;
      SceneNode& node = *robj_ptr->GetNodePtr();
      auto shading_ptr = node.GetComponentPtr<ShadingComponent>();
      if (shading_ptr == nullptr) {
        std::cerr << "Some mesh is not attached with a shader during rendering!"
                  << std::endl;
        continue;
      }
      ShaderProgram* shader = shading_ptr->GetShaderPtr();

      BindGuard shader_bg(shader);

      // Set various uniform variables in the shaders.
      shader->SetTargetNode(node, pr.second);
      shader->SetCamera(*camera);

      robj_ptr->Render();
    }
  }

  // The real shadow map/Phong shading passes.
  for (size_t light_id = 0; light_id < light_ptrs.size(); light_id++) {

    GL_CHECK(glDepthMask(GL_FALSE));
    bool color_mask = GL_TRUE;
    GL_CHECK(glColorMask(color_mask, color_mask, color_mask, color_mask));

    for (const auto& pr : rendering_info) {
      auto robj_ptr = pr.first;
      SceneNode& node = *robj_ptr->GetNodePtr();
      auto shading_ptr = node.GetComponentPtr<ShadingComponent>();
      if (shading_ptr == nullptr) {
        std::cerr << "Some mesh is not attached with a shader during rendering!"
                  << std::endl;
        continue;
      }
      ShaderProgram* shader = shading_ptr->GetShaderPtr();

      BindGuard shader_bg(shader);

      // Set various uniform variables in the shaders.
      shader->SetTargetNode(node, pr.second);
      shader->SetCamera(*camera);

      LightComponent& light = *light_ptrs.at(light_id);
      shader->SetLightSource(light);

      robj_ptr->Render();
    }
  }

  // Re-enable writing to depth buffer.
  GL_CHECK(glDepthMask(GL_TRUE));
}

}  // namespace GLOO
