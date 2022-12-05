#include "Grid.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"

namespace GLOO {

Grid::Grid(glm::vec3 origin, float d_, int x_, int y_, int z_, IsoSurface isosurface): origin_(origin), d(d_), x(x_), y(y_), z(z_), isosurface_(isosurface){
    computePositions();
    computeValues();
    drawGrid();
}

void Grid::computeValues(){
    values_.reserve((x+1)*(y+1)*(z+1));
    for (int i = 0; i <= x; i++){
        for (int j = 0; j <= y; j++){
            for (int k = 0; k <= z; k++){
                values_.push_back(isosurface_.function_(origin_ + d*glm::vec3(i,j,k)));
            }
        }
    }
}

//p = i*(y+1)*(z+1) + j*(z+1) + k
void Grid::computePositions(){
    positions_.reserve((x+1)*(y+1)*(z+1));
    for (int i = 0; i <= x; i++){
        for (int j = 0; j <= y; j++){
            for (int k = 0; k <= z; k++){
                positions_.push_back(origin_ + d*glm::vec3(i,j,k));
            }
        }
    }
    std::unique_ptr<PositionArray> positions = make_unique<PositionArray>(positions_);
    structure_line_->UpdatePositions(std::move(positions));
}

int Grid::getIndex(int i, int j, int k){
    return i*(y+1)*(z+1) + j*(z+1) + k;
}

void Grid::drawGrid(){
    std::unique_ptr<IndexArray> indices = make_unique<IndexArray>();

    for (int i = 0; i < x; i++){
        for (int j = 0; j < y; j++){
            for (int k = 0; k < z; k++){
                indices->push_back(getIndex(i,j,k));
                indices->push_back(getIndex(i+1,j,k));
                indices->push_back(getIndex(i,j,k));
                indices->push_back(getIndex(i,j+1,k));
                indices->push_back(getIndex(i,j,k));
                indices->push_back(getIndex(i,j,k+1));
            }
        }
    }
    structure_line_->UpdateIndices(std::move(indices));
    auto mesh_node = make_unique<SceneNode>();
    mesh_node->CreateComponent<ShadingComponent>(polyline_shader_);
    auto& rc = mesh_node->CreateComponent<RenderingComponent>(structure_line_);
    rc.SetDrawMode(DrawMode::Lines);
    AddChild(std::move(mesh_node));
}

}