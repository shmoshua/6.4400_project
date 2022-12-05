#include "Grid.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include <glm/gtx/string_cast.hpp>

namespace GLOO {

Grid::Grid(glm::vec3 origin, float d_, int x_, int y_, int z_, IsoSurface isosurface): origin_(origin), d(d_), x(x_), y(y_), z(z_), isosurface_(isosurface){
    computePositions();
    computeValues();
    drawGrid();
    computeCubeIndex();
    std::cout << "Values range between [" << minVal << ", " << maxVal << "]" << std::endl;

    
    // for (auto c : values_){
    //     std::cout << c << std::endl;
    // }
    // for (auto c : cubeIndices_){
    //     std::cout << c << std::endl;
    // }
    addInterpolationPoints();
}

void Grid::computeValues(){
    values_.reserve((x+1)*(y+1)*(z+1));
    for (int i = 0; i <= x; i++){
        for (int j = 0; j <= y; j++){
            for (int k = 0; k <= z; k++){
                float val = isosurface_.function_(origin_ + d*glm::vec3(i,j,k));
                values_.push_back(val);
                minVal = minVal < val ? minVal : val;
                maxVal = maxVal > val ? maxVal : val;
            }
        }
    }
}

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

int Grid::getIndex(int i, int j, int k, int vertexIndex){
    int d = vertexTable[vertexIndex];
    int ans = 0;
    ans += d & 4 ? (i+1)*(y+1)*(z+1) : i * (y+1)*(z+1);
    ans += d & 2 ? (j+1)*(z+1) : j * (z+1);
    ans += d & 1 ? (k+1) : k ;

    return ans;
}

int Grid::getCubeIndex(int i, int j, int k){
    return i*y*z + j*z + k;
}

std::pair<int, int> Grid::getAdjacentVertexIndices(int edgeIndex){
    if (edgeIndex == 3 || edgeIndex == 7){
        return std::make_pair(edgeIndex, edgeIndex - 3);
    }
    if (edgeIndex <= 7){
        return std::make_pair(edgeIndex, edgeIndex + 1);
    }
    if (edgeIndex <= 11){
        return std::make_pair(edgeIndex - 4, edgeIndex - 8);
    }
    return std::make_pair(-1,-1);
}


void Grid::drawGrid(){
    std::unique_ptr<IndexArray> indices = make_unique<IndexArray>();

    for (int i = 0; i <= x; i++){
        for (int j = 0; j <= y; j++){
            for (int k = 0; k <= z; k++){
                if (i+1 <= x){
                    indices->push_back(getIndex(i,j,k));
                    indices->push_back(getIndex(i+1,j,k));
                }
                if (j+1 <= y){
                    indices->push_back(getIndex(i,j,k));
                    indices->push_back(getIndex(i,j+1,k));
                }
                if (k+1 <= z){
                    indices->push_back(getIndex(i,j,k));
                    indices->push_back(getIndex(i,j,k+1));
                }
            }
        }
    }
    structure_line_->UpdateIndices(std::move(indices));
    auto mesh_node = make_unique<SceneNode>();
    mesh_node->CreateComponent<ShadingComponent>(polyline_shader_);
    auto& rc = mesh_node->CreateComponent<RenderingComponent>(structure_line_);
    rc.SetDrawMode(DrawMode::Lines);
    grid = mesh_node.get();
    AddChild(std::move(mesh_node));
}

void Grid::computeCubeIndex(){
    cubeIndices_.reserve(x*y*z);
    edgeVectors_.reserve(x*y*z);
    for (int i = 0; i < x; i++){
        for (int j = 0; j < y; j++){
            for (int k = 0; k < z; k++){
                int cubeIndex = 0;
                if (values_[getIndex(i,j,k)] < isosurface_.epsilon_) cubeIndex |= 1;  //0
                if (values_[getIndex(i,j,k+1)] < isosurface_.epsilon_) cubeIndex |= 2; //1
                if (values_[getIndex(i,j+1,k+1)] < isosurface_.epsilon_) cubeIndex |= 4; //2
                if (values_[getIndex(i,j+1,k)] < isosurface_.epsilon_) cubeIndex |= 8; //3
                if (values_[getIndex(i+1,j,k)] < isosurface_.epsilon_) cubeIndex |= 16; //4
                if (values_[getIndex(i+1,j,k+1)] < isosurface_.epsilon_) cubeIndex |= 32; //5
                if (values_[getIndex(i+1,j+1,k+1)] < isosurface_.epsilon_) cubeIndex |= 64; //6
                if (values_[getIndex(i+1,j+1,k)] < isosurface_.epsilon_) cubeIndex |= 128; //7
                cubeIndices_.push_back(cubeIndex);
                edgeVectors_.push_back(edgeTable[cubeIndex]);
            }
        }
    }
}

glm::vec3 Grid::interpolate(int p1, int p2){
    if (abs(isosurface_.epsilon_ - values_[p1]) < 0.00001)
        return positions_[p1];
    if (abs(isosurface_.epsilon_ - values_[p2]) < 0.00001)
        return positions_[p2];
    if (abs(values_[p1] - values_[p2]) < 0.00001)
        return positions_[p1];
    float lambda = (isosurface_.epsilon_ - values_[p1]) / (values_[p2] - values_[p1]);
    return positions_[p1] + lambda * (positions_[p2] - positions_[p1]);
}

void Grid::addInterpolationPoints(){

    std::unique_ptr<PositionArray> surface_points = make_unique<PositionArray>();
    std::unique_ptr<IndexArray> surface_indices = make_unique<IndexArray>();

    for (int i = 0; i < x; i++){
        for (int j = 0; j < y; j++){
            for (int k = 0; k < z; k++){
                //Add the interpolated points
                int edgeVector = edgeVectors_[getCubeIndex(i,j,k)];
                int l = 1;
                for (int m = 0; m < 12; m++){
                    if (edgeVector & l){
                        std::pair<int, int> neighbors = getAdjacentVertexIndices(m);
                        glm::vec3 point = interpolate(getIndex(i,j,k,neighbors.first), getIndex(i,j,k,neighbors.second));
                        surface_points->push_back(point);
                    }
                    else {
                        surface_points->push_back(glm::vec3(0,0,0));
                    }
                    l <<= 1;
                }

                //Add the indices
                auto triangles = triangleTable[cubeIndices_[getCubeIndex(i,j,k)]];
                int m = 0;
                int offset = 12 * getCubeIndex(i,j,k);
                while (triangles[m] != -1){
                    surface_indices->push_back(offset + triangles[m]);
                    m += 1;
                }

            }
        }
    }

    // for (int i = 0; i < (*surface_points).size(); i++){
    //     std::cout << glm::to_string(surface_points->at(i)) << std::endl;
    // }

    // for (int i = 0; i < (*surface_indices).size(); i++){
    //     std::cout << surface_indices->at(i) << std::endl;
    // }

    surface_line_->UpdatePositions(std::move(surface_points));
    surface_line_->UpdateIndices(std::move(surface_indices));
    auto surface_node = make_unique<SceneNode>();
    surface_node->CreateComponent<ShadingComponent>(polyline_shader_);
    auto& rc = surface_node->CreateComponent<RenderingComponent>(surface_line_);
    surface_node->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
    rc.SetDrawMode(DrawMode::Triangles);
    AddChild(std::move(surface_node));
}



}