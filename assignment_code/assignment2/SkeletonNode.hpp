#ifndef SKELETON_NODE_H_
#define SKELETON_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

#include <string>
#include <vector>

namespace GLOO {
class SkeletonNode : public SceneNode {
 public:
  enum class DrawMode { Skeleton, SSD };
  struct EulerAngle {
    float rx, ry, rz;
  };

  int n; //number of vertices;
  int m; //number of joints;

  SkeletonNode(const std::string& filename);
  void LinkRotationControl(const std::vector<EulerAngle*>& angles);
  void Update(double delta_time) override;
  void OnJointChanged();

 private:
  void LoadAllFiles(const std::string& prefix);
  void LoadSkeletonFile(const std::string& path);
  void LoadMeshFile(const std::string& filename);
  void LoadAttachmentWeights(const std::string& path);

  void ToggleDrawMode();
  void DecorateTree();
  void ComputePositions();
  void ComputeNormals();
  void SetJointColors();

  DrawMode draw_mode_;
  
  std::vector<SceneNode*> cylinder_nodes_ptrs_;
  std::vector<SceneNode*> joint_nodes_ptrs_;
  std::vector<glm::mat4> B_;
  std::vector<glm::mat4> T_;
  SceneNode* skin_node_ptr_;

  std::vector<glm::vec4> colors_;
  
  std::shared_ptr<VertexObject> bind_pose_mesh_;
  std::vector<glm::vec4> p_;
  std::vector<glm::vec3> normals_;

  std::vector<std::vector<float>> attachment_weights_;

  // Euler angles of the UI sliders.
  std::vector<EulerAngle*> linked_angles_;
};
}  // namespace GLOO

#endif
