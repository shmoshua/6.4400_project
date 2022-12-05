#ifndef ISOSURFACE_H_
#define ISOSURFACE_H_

#include <string>
#include <vector>

namespace GLOO {
class IsoSurface {
    public:
        std::function<float(glm::vec3)> function_;
        float epsilon_;

        IsoSurface(std::function<float(glm::vec3)> function, float epsilon): function_(function), epsilon_(epsilon){};

    private:
    
};
}  // namespace GLOO

#endif
