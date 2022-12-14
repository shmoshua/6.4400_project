#ifndef PERLIN_H_
#define PERLIN_H_

#include <string>
#include <vector>
#include <stdlib.h>
#include "gloo/SceneNode.hpp"

namespace GLOO {
class Perlin {
    public:
        Perlin(int x, int y, int z, float d);

        std::vector<std::vector<float>> arr;
        std::function<float(glm::vec3)> function_;

        std::vector<int> P;

        void shuffle(std::vector<int> &tab);
        std::vector<int> makePermutation();
        glm::vec2 getConstantVector(int v);

        float Fade(float t);
        float Lerp(float t, float a1,float a2);
        float Noise2D(float x, float y);








    private:
    
};
}  // namespace GLOO

#endif
