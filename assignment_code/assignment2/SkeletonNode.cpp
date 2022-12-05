#include "SkeletonNode.hpp"

#include <fstream>
#include <string>
#include "gloo/utils.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/shaders/PhongShader2.hpp"

namespace GLOO {
SkeletonNode::SkeletonNode(const std::string& filename)
    : SceneNode(), draw_mode_(DrawMode::Skeleton) {
  LoadAllFiles(filename);
  DecorateTree();

  // Force initial update.
  OnJointChanged();
}

void SkeletonNode::ToggleDrawMode() {
  draw_mode_ = draw_mode_ == DrawMode::Skeleton ? DrawMode::SSD : DrawMode::Skeleton;
  // TODO: implement here toggling between skeleton mode and SSD mode.
  // The current mode is draw_mode_;
  // Hint: you may find SceneNode::SetActive convenient here as
  // inactive nodes will not be picked up by the renderer.
  skin_node_ptr_->SetActive(draw_mode_ != DrawMode::Skeleton);
  for (auto joint: joint_nodes_ptrs_){
    joint->SetActive(draw_mode_ == DrawMode::Skeleton);
  }
}

void SkeletonNode::ComputeNormals(){
  auto positions = bind_pose_mesh_->GetPositions();
  auto indices = bind_pose_mesh_->GetIndices();
  std::vector<glm::vec3> normals;

  for (int i = 0; i < n; i++){
    normals.push_back(glm::vec3(0.f, 0.f,0.f));
  }

  int c = indices.size();
  if (c%3 != 0) std::cout << "NOT 3" << std::endl;

  for (int i = 0; i < c/3; i++){
    int x1 = indices[3*i];
    int x2 = indices[3*i+1];
    int x3 = indices[3*i+2];

    glm::vec3 pos1 = positions[x1];
    glm::vec3 pos2 = positions[x2];
    glm::vec3 pos3 = positions[x3];

    glm::vec3 normal = glm::cross(pos2-pos1, pos3-pos1);
    float area = 0.5f* glm::length(normal);

    normals[x1] += area*normal;
    normals[x2] += area*normal;
    normals[x3] += area*normal;
  }

  for (int i = 0; i < n; i++){
    normals[i] = glm::normalize(normals[i]);
  }

  bind_pose_mesh_->UpdateNormals(make_unique<NormalArray>(normals));
}

void SkeletonNode::ComputePositions(){
  std::vector<glm::vec3> positions;

  for (int i = 0; i < n; i++){
    positions.push_back(glm::vec3(0.f, 0.f,0.f));
  }

  for (int i = 0; i < n; i++){
    for (int j = 1; j < m; j++){
      glm::vec4 toAdd = attachment_weights_[i][j-1] * T_[j] * B_[j] * p_[i];
      positions[i] += glm::vec3(toAdd.x, toAdd.y, toAdd.z);
    }
  }

  bind_pose_mesh_->UpdatePositions(make_unique<PositionArray>(positions));
}

void SkeletonNode::DecorateTree() {

  std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(0.025f, 25, 25);
  std::shared_ptr<VertexObject> cylinder_mesh_ = PrimitiveFactory::CreateCylinder(0.015f, 1.f, 25);
  std::shared_ptr<ShaderProgram> shader_ = std::make_shared<PhongShader>();
  std::vector<SceneNode*> sphere_nodes_ptrs_;

  for (auto joint: joint_nodes_ptrs_){
    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader_);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    sphere_nodes_ptrs_.push_back(sphere_node.get());
    joint->AddChild(std::move(sphere_node));

    int c = joint->GetChildrenCount();
    for (int i = 0; i < c; i++){
      SceneNode& child = joint->GetChild(i);
      glm::vec3 pos = child.GetTransform().GetPosition();
      glm::vec3 axis = glm::normalize(glm::cross(pos, glm::vec3(0.f, 1.f, 0.f)));
      float angle = glm::orientedAngle(glm::vec3(0.f, 1.f, 0.f), glm::normalize(pos), axis);

      auto cylinder_node = make_unique<SceneNode>();
      cylinder_node->CreateComponent<ShadingComponent>(shader_);
      cylinder_node->CreateComponent<RenderingComponent>(cylinder_mesh_);
      cylinder_node->GetTransform().SetScale(glm::vec3(1.0f, glm::length(pos), 1.0f));
      cylinder_node->GetTransform().SetRotation(axis, angle);
      cylinder_nodes_ptrs_.push_back(cylinder_node.get());
      joint->AddChild(std::move(cylinder_node));
    }
  }

  ComputePositions();
  ComputeNormals();
  
  auto skin_node = make_unique<SceneNode>();
  skin_node->CreateComponent<ShadingComponent>(shader_);
  skin_node->CreateComponent<RenderingComponent>(bind_pose_mesh_);
  skin_node->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
  skin_node->GetComponentPtr<MaterialComponent>()->GetMaterial().SetAmbientColor(glm::vec3(0.5f, 0.5f, 0.5f));
  skin_node->GetComponentPtr<MaterialComponent>()->GetMaterial().SetDiffuseColor(glm::vec3(0.5f, 0.5f, 0.5f));
  skin_node_ptr_ = skin_node.get();
  AddChild(std::move(skin_node));
  skin_node_ptr_->SetActive(false);
}

void SkeletonNode::Update(double delta_time) {
  // Prevent multiple toggle.
  static bool prev_released = true;
  if (InputManager::GetInstance().IsKeyPressed('S')) {
    if (prev_released) {
      ToggleDrawMode();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyReleased('S')) {
    prev_released = true;
  }
}

void SkeletonNode::OnJointChanged() {
  if (linked_angles_.size() > 0){

    for (int i = 0; i < m; i++){
      SceneNode* joint = joint_nodes_ptrs_[i];
      EulerAngle* angle = linked_angles_[i];

      joint->GetTransform().SetRotation(glm::quat(glm::vec3(angle->rx, angle->ry, angle->rz)));
      T_[i] = joint->GetTransform().GetLocalToWorldMatrix();
    }

    ComputePositions();
    ComputeNormals();
  }
}

void SkeletonNode::LinkRotationControl(const std::vector<EulerAngle*>& angles) {
  linked_angles_ = angles;
}

float readFloat(std::stringstream& fs){
  std::string val;
  std::getline(fs, val, ' ');
  return std::stof(val);
}

int readInt(std::stringstream& fs){
  std::string val;
  std::getline(fs, val, ' ');
  return std::stoi(val);
}

void SkeletonNode::LoadSkeletonFile(const std::string& path) {
  std::fstream fs(path);
  if (!fs) {
    std::cerr << "ERROR: Unable to open file!" << std::endl;
    return;
  }

  int i = 0;
  for (std::string line; std::getline(fs, line); ){
    std::stringstream ss;
    ss << line;

    auto joint = make_unique<SceneNode>();
    joint->GetTransform().SetPosition(glm::vec3(readFloat(ss), readFloat(ss), readFloat(ss)));
    int parent = readInt(ss);
    joint_nodes_ptrs_.push_back(joint.get());

    if (parent == -1) AddChild(std::move(joint));
    else joint_nodes_ptrs_[parent]->AddChild(std::move(joint));
    
    glm::mat4 T = joint_nodes_ptrs_[i]->GetTransform().GetLocalToWorldMatrix();
    glm::mat4 B = glm::inverse(T);
    B_.push_back(B);
    T_.push_back(T);
    i++;
  }

  m = joint_nodes_ptrs_.size();
}

void SkeletonNode::LoadMeshFile(const std::string& filename) {
  bind_pose_mesh_ = MeshLoader::Import(filename).vertex_obj;
  PositionArray positions = bind_pose_mesh_->GetPositions();
  for (glm::vec3 p : positions){
    p_.push_back(glm::vec4(p.x, p.y, p.z, 1.f));
  }
}

void SkeletonNode::LoadAttachmentWeights(const std::string& path) {
  std::fstream fs(path);
  if (!fs) {
    std::cerr << "ERROR: Unable to open file!" << std::endl;
    return;
  }

  for (std::string line; std::getline(fs, line); ){
    std::stringstream ss;
    ss << line;

    std::vector<float> row;
    for (int i = 1; i < m; i++){
      row.push_back(readFloat(ss));
    }
    attachment_weights_.push_back(row);
  }

  n = attachment_weights_.size();
}

void SkeletonNode::LoadAllFiles(const std::string& prefix) {
  std::string prefix_full = GetAssetDir() + prefix;
  LoadSkeletonFile(prefix_full + ".skel");
  LoadMeshFile(prefix + ".obj");
  LoadAttachmentWeights(prefix_full + ".attach");
}
}  // namespace GLOO
