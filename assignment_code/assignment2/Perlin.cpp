#include "Perlin.hpp"

#include <cmath>
#include <glm/gtx/string_cast.hpp>

namespace GLOO {

Perlin::Perlin(int x, int y, int z, float d){

    for (int i = 0; i < x; i++){
        std::vector<float> a;
        for (int j = 0; j < z; j++){
            a.push_back(0.f);
        }
        arr.push_back(a);
    }

    P = makePermutation();

    for(int j = 0; j < z; j++){
        for(int i = 0; i < x; i++){
            float n = Noise2D(i*0.05, j*0.05);
            n += 1.0;
            n *= 0.5;
            float rgb = round(255*n);
            arr[i][j] = rgb;
        }
    }

    // function_ = [&arr, &minx, &minz](glm::vec3 point){
    //     std::cout << glm::to_string(point) << std::endl;
    //     float px = (point[0] + minx) ;
    //     float pz = (point[2] + minz) ;

    //     std::cout << px << ", " << pz << std::endl;

    //     int x  =  floor(px);
    //     int z  =  floor(pz);
    //     float dx = px - x;
    //     float dz = pz - z;    
    //     std::cout << x << ", " << z << std::endl;
    //     std::cout << dx << ", " << dz << std::endl;

    //     std::cout << "SIZE: " << arr.size() << std::endl;
    //     std::cout << "SIZE: " << arr[0].size() << std::endl;

    //     float cx1 = arr[x][z] + dx*(arr[x+1][z] - arr[x][z]);
        
    //     std::cout << cx1 << std::endl;

    //     float cx2 = arr[x][z+1] + dx*(arr[x+1][z+1] - arr[x][z+1]);
    //     float cz = cx1 + dz*(cx2 - cx1);
    //     return point[1] - cz ;};
}

void Perlin::shuffle(std::vector<int> &tab){
    for (int e = tab.size()- 1; e > 0; e--){
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        int index = int(r * (e-1));
        int temp = tab[e];
        tab[e] = tab[index];
        tab[index] = temp;
    }
}

std::vector<int> Perlin::makePermutation(){
    std::vector<int> P;
    for (int i = 0; i < 256; i++) P.push_back(i);
    shuffle(P);
    for (int i = 0; i < 256; i++) P.push_back(P[i]);
    return P;
}

glm::vec2 Perlin::getConstantVector(int v){
    int h = v & 3;
    if (h == 0) return glm::vec2(1.f,1.f);
    else if (h == 1) return glm::vec2(-1.f,1.f);
    else if(h == 2) return glm::vec2(-1.f,-1.f);
    else return glm::vec2(1.f,-1.f);
}

float Perlin::Fade(float t){
    return ((6*t - 15)*t + 10)*t*t*t;
}

float Perlin::Lerp(float t, float a1,float a2){
    return a1 + t*(a2-a1);
}

float Perlin::Noise2D(float x, float y){

    int X = ((int) std::floor(x)) & 255;
    int Y = ((int) std::floor(y)) & 255;

    float xf = x - floor(x);
    float yf = y - floor(y);

    glm::vec2 topRight = glm::vec2(xf-1.f, yf-1.f);
    glm::vec2 topLeft = glm::vec2(xf, yf-1.f);
    glm::vec2 bottomRight = glm::vec2(xf-1.f, yf);
    glm::vec2 bottomLeft = glm::vec2(xf, yf);
    
    // std::cout << P.size() << std::endl;
    // std::cout << X << ", " << Y << std::endl;
    // std::cout << P[X] << ",, " << P[X+1] << std::endl;
    float valueTopRight = P[P[X+1]+Y+1];
    float valueTopLeft = P[P[X]+Y+1];
    float valueBottomRight = P[P[X+1]+Y];
    float valueBottomLeft = P[P[X]+Y];
    

    float dotTopRight = glm::dot(topRight, getConstantVector(valueTopRight));
    float dotTopLeft = glm::dot(topLeft, getConstantVector(valueTopLeft));
    float dotBottomRight = glm::dot(bottomRight, getConstantVector(valueBottomRight));
    float dotBottomLeft = glm::dot(bottomLeft, getConstantVector(valueBottomLeft));

    
    float u = Fade(xf);
    float v = Fade(yf);

    return Lerp(u,Lerp(v, dotBottomLeft, dotTopLeft),Lerp(v, dotBottomRight, dotTopRight));
}



}