#ifndef TERRAIN_H_
#define TERRAIN_H_

#include "IsoSurface.hpp"

namespace GLOO {
class Terrain : public IsoSurface {
    std::function<float(glm::vec3)> function_ = [](glm::vec3 point) {
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
};
}  // namespace GLOO

#endif
