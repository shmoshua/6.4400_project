#ifndef GRID_H_
#define GRID_H_

#include "gloo/SceneNode.hpp"
#include "IsoSurface.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"

#include "gloo/shaders/SimpleShader.hpp"

#include <string>
#include <vector>

namespace GLOO {
class Grid : public SceneNode {
    public:

        glm::vec3 origin_;
        //unit gridcube size
        float d;

        // number of cubes into x,y,z axes
        int x;
        int y;
        int z;

        IsoSurface isosurface_;

        std::vector<glm::vec3> positions_;
        std::vector<float> values_;
        std::shared_ptr<SimpleShader> polyline_shader_ = std::make_shared<SimpleShader>();

        std::shared_ptr<VertexObject> structure_line_ = std::make_shared<VertexObject>();

        Grid(glm::vec3 origin, float d_, int x_, int y_, int z_, IsoSurface isosurface);
        
        void computePositions();
        void computeValues();
        void drawGrid();

        int getIndex(int i, int j, int k);



    private:
    

};
}  // namespace GLOO

#endif
