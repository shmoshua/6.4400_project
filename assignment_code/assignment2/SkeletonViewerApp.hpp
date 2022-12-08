#ifndef SKELETON_VIEWER_APP_H_
#define SKELETON_VIEWER_APP_H_

#include "gloo/Application.hpp"
#include "Grid.hpp"
#include "SkeletonNode.hpp"

namespace GLOO {
class SkeletonViewerApp : public Application {
 public:
  SkeletonViewerApp(const std::string& app_name,
                    glm::ivec2 window_size);
  void SetupScene() override;
 
  const float INITIAL_D = 0.1;
  const int INITIAL_X = 20;
  const int INITIAL_Y = 20;
  const int INITIAL_Z = 20;
 const float EPS = 0.4f;

 protected:
  void DrawGUI() override;

 private:
  SkeletonNode* skeletal_node_ptr_;
  Grid* grid_ptr_;
  std::vector<SkeletonNode::IntNode> dim_values_;
  std::vector<SkeletonNode::EulerAngle> slider_values_;
};
}  // namespace GLOO

#endif
