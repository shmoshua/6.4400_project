#include "SkeletonViewerApp.hpp"

#include "gloo/external.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/debug/AxisNode.hpp"
#include "Grid.hpp"

namespace {
const std::vector<std::string> kJointNames = {"Threshold", "Size D", "Size X", "Size Y", "Size Z"};
}

namespace GLOO {

SkeletonViewerApp::SkeletonViewerApp(const std::string& app_name,
                                     glm::ivec2 window_size)
    : Application(app_name, window_size),
      slider_values_(2, {0.f, 0.f, 0.f}),
    dim_values_(3, {INITIAL_X, INITIAL_Y, INITIAL_Z}) {}

void SkeletonViewerApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 3.0f);
  camera_node->GetTransform().SetPosition(glm::vec3(0.f,-1.f,0.f));
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.2f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto sun_light = std::make_shared<DirectionalLight>();
  sun_light->SetDiffuseColor(glm::vec3(0.6f, 0.6f, 0.6f));
  sun_light->SetSpecularColor(glm::vec3(0.6f, 0.6f, 0.6f));
  sun_light->SetDirection(glm::vec3(-1.0f, -0.8f, -1.0f));
  auto sun_light_node = make_unique<SceneNode>();
  sun_light_node->CreateComponent<LightComponent>(sun_light);
  root.AddChild(std::move(sun_light_node));

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.9f, 0.9f, 0.9f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 10.0f, 10.f));
  root.AddChild(std::move(point_light_node));

  std::function<float(glm::vec3)> sphere = [](glm::vec3 point) {return point[0]* point[0] + point[1]*point[1] +point[2] *point[2];};
  std::function<float(glm::vec3)> y_cylinder = [](glm::vec3 point) {return point[0]* point[0] + point[2] *point[2];};
  std::function<float(glm::vec3)> x_cylinder = [](glm::vec3 point) {return point[1]* point[1] + point[2] *point[2];};
  std::function<float(glm::vec3)> simple = [](glm::vec3 point) {return point[0]*point[1]*point[2];};
  std::function<float(glm::vec3)> cube1 = [](glm::vec3 point) {return glm::sin(glm::length(10.f * point))* 10.f * point[0] + glm::sin(glm::length(10.f * point))* 10.f * point[1];};
  std::function<float(glm::vec3)> cube2 = [](glm::vec3 point) {return glm::sin(point[0]*point[1] + point[1]*point[2] + point[0]*point[2]) + glm::sin(point[0]*point[1]) + glm::sin(point[0]*point[2]) + glm::sin(point[2]*point[1]);};
  std::function<float(glm::vec3)> func = [](glm::vec3 point) {return 0;};

  IsoSurface isosurface = IsoSurface(cube1, 0.4f);
  auto grid_node = make_unique<Grid>(INITIAL_D, INITIAL_X, INITIAL_Y, INITIAL_Z, isosurface);
  
  grid_ptr_ = grid_node.get();

  std::vector<SkeletonNode::EulerAngle*> angles;
  std::vector<SkeletonNode::IntNode*> dims;
  slider_values_[1].rx = INITIAL_D;
  for (size_t i = 0; i < slider_values_.size(); i++) {
      angles.push_back(&slider_values_[i]);
  }
  for (int i = 0; i < dim_values_.size(); ++i) {
      dims.push_back(&dim_values_[i]);
  }
  grid_ptr_->LinkControl(angles, dims);
  
  root.AddChild(std::move(grid_node));
}

void SkeletonViewerApp::DrawGUI() {
  bool modifiedval = false;
  bool modifieddim = false;
  ImGui::Begin("Control Panel");
  for (size_t i = 0; i < 5; i++) {
      ImGui::Text("%s", kJointNames[i].c_str());
      ImGui::PushID((int)i);
      //std::cout << i << std::endl;
      if (i == 0) {
          modifiedval |= ImGui::SliderFloat("", &slider_values_[0].rx, 0.0f, 1.0f);
      }
      else if (i == 1) {
          modifieddim |= ImGui::SliderFloat("", &slider_values_[1].rx, 0.05f, 0.25f);
      }
      else {
          modifieddim |= ImGui::SliderInt("", &dim_values_[i - 2].x, 5, 100);
      }
      ImGui::PopID();
  }
  ImGui::End();

  if (modifiedval) {
    grid_ptr_->OnChangedValue();
  }
  if (modifieddim) {
    grid_ptr_->OnChangedDimension();
  }
}
}  // namespace GLOO
