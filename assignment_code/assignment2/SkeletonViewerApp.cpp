#include "SkeletonViewerApp.hpp"

#include "gloo/external.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/debug/AxisNode.hpp"
#include "Grid.hpp"
#include "Perlin.hpp"

namespace {
const std::vector<std::string> kJointNames = {"Threshold", "Size D", "Size X", "Size Y", "Size Z"};
}

namespace GLOO {

SkeletonViewerApp::SkeletonViewerApp(const std::string& app_name,
                                     glm::ivec2 window_size)
    : Application(app_name, window_size),
    dim_values_(3, {5, 5, 5}), slider_values_(2, {0.f, 0.f, 0.f}) {}

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
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 100.0f, 100.f));
  //root.AddChild(std::move(point_light_node));

  std::function<float(glm::vec3)> sphere = [](glm::vec3 point) {return point[0]* point[0] + point[1]*point[1] +point[2] *point[2];};
  std::function<float(glm::vec3)> y_cylinder = [](glm::vec3 point) {return point[0]* point[0] + point[2] *point[2];};
  std::function<float(glm::vec3)> x_cylinder = [](glm::vec3 point) {return point[1]* point[1] + point[2] *point[2];};
  std::function<float(glm::vec3)> simple = [](glm::vec3 point) {return point[0]*point[1]*point[2];};
  std::function<float(glm::vec3)> cube1 = [](glm::vec3 point) {return glm::sin(glm::length(10.f * point))* 10.f * point[0] + glm::sin(glm::length(10.f * point))* 10.f * point[1];};
  std::function<float(glm::vec3)> cube2 = [](glm::vec3 point) {return glm::sin(point[0]*point[1] + point[1]*point[2] + point[0]*point[2]) + glm::sin(point[0]*point[1]) + glm::sin(point[0]*point[2]) + glm::sin(point[2]*point[1]);};
  std::function<float(glm::vec3)> func = [](glm::vec3 point) {return 0;};
  std::function<float(glm::vec3)> bump = [](glm::vec3 point) {
                                                              point[0] = 2.0f * (point[0] - std::floor(point[0]) - 0.5f);
                                                              point[1] = 2.0f * (point[1] - std::floor(point[1]) - 0.5f);
                                                              point[2] = 2.0f * (point[2] - std::floor(point[2]) - 0.5f);
                                                              float rx = point[0] * point[0];
                                                              float ry = point[1] * point[1];
                                                              float rz = point[2] * point[2];
                                                              if (point[1] < 0.0f) return 0.0f;
                                                              if (rx > 0.9999f  || ry > 0.9999f || rz > 0.9999f) return 0.01f;
                                                              rx = -1.0f / (1.0f - rx);
                                                              ry = -1.0f / (1.0f - ry);
                                                              rz = -1.0f / (1.0f - rz);
                                                              return 10 * glm::exp(rx + ry + rz); };
  std::function<float(glm::vec3)> heart = [](glm::vec3 point) {
                                                              float x = point[0]/3.f;
                                                              float y = point[1]/3.f;
                                                              float z = point[2]/3.f;
                                                              float r1 = x*x+ 9.f*y*y/4.f + z*z-1;
                                                              float r2 = r1*r1*r1 -x*x*z*z*z;
                                                              return r2 - 9.f*y*y*z*z*z/200.f;
                                                              };
  
  std::function<float(glm::vec3)> cos = [](glm::vec3 point) {
                                                              return std::cos(point[0]) + std::cos(point[1]) + std::cos(point[2]);
                                                              };
  std::function<float(glm::vec3)> quad = [](glm::vec3 point) {
                                                              float x = point[0];
                                                              float y = point[1];
                                                              float z = point[2];
                                                              return x*x*x*x+y*y*y*y+z*z*z*z-x*x-y*y-z*z;
                                                              };
  std::function<float(glm::vec3)> rods = [](glm::vec3 point) {
                                                              float x = point[0];
                                                              float y = point[1];
                                                              float z = point[2];
                                                              float rods = pow((x-.5)*6,4)+pow((y-.5)*6,4)+pow((z-.5)*6,4)-(pow((x-.5)*6,2))-(pow((y-.5)*6,2))-(pow((z-.5)*6,2))-(pow((x-.5)*6,2))*(pow((y-.5)*6,2))-(pow((x-.5)*6,2))*(pow((z-.5)*6,2))-(pow((y-.5)*6,2))*(pow((z-.5)*6,2))+1;
                                                              float cube =  std::max(abs(x-.5)-.495, std::max(abs(y-.5)-.495,abs(z-.5)-.495));
                                                              return std::max(rods,cube);
                                                              };
  std::function<float(glm::vec3)> hourglass = [](glm::vec3 point) {
                                                              float x = point[0];
                                                              float y = point[1];
                                                              float z = point[2];
                                                              return pow((y-.5)*2,4) - (pow((y-.5)*2,2)) + (pow((x-.5)*2,2)) + (pow((z-.5)*2,2));
                                                              };
  std::function<float(glm::vec3)> tangle = [](glm::vec3 point) {
                                                              float x = point[0];
                                                              float y = point[1];
                                                              float z = point[2];
                                                              return pow((x-.5)*4.6,4) - 5*(pow((x-.5)*4.6,2)) + pow((y-.5)*4.6,4) - 5*(pow((y-.5)*4.6,2)) + pow((z-.5)*4.6,4) - 5*(pow((z-.5)*4.6,2)) + 11.8;
                                                              };


  Perlin perlin = Perlin(100,100,100, 0.1);
  IsoSurface perlin_isosurface = IsoSurface(perlin.function_, 0.4f);
  IsoSurface isosurface = IsoSurface(sphere, 0.4f);
  auto grid_node = make_unique<Grid>(0.1, 100, 100, 100, isosurface);
  
  grid_ptr_ = grid_node.get();

  std::vector<SkeletonNode::EulerAngle*> angles;
  std::vector<SkeletonNode::IntNode*> dims;
  slider_values_[1].rx = 0.15;
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
