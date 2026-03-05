#pragma once
// ==========================================================
// model_loader.h — Simple OBJ Model Loader
// Parses .obj files to be used with the Renderer3D Mesh system.
// ==========================================================

#include "render/renderer3d.h"
#include "math/math3d.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

namespace ModelLoader {

    // Simple hashing for unique vertices in the OBJ file to build an indexed mesh
    struct VertexKey {
        int v, vt, vn;
        bool operator<(const VertexKey& other) const {
            if (v != other.v) return v < other.v;
            if (vt != other.vt) return vt < other.vt;
            return vn < other.vn;
        }
    };

    // Helper to parse face indices "v/vt/vn"
    inline bool parseFaceVertex(const std::string& token, VertexKey& key) {
        key.v = 0; key.vt = 0; key.vn = 0;
        std::istringstream stream(token);
        std::string part;
        
        // v
        if (std::getline(stream, part, '/')) {
            if (!part.empty()) key.v = std::stoi(part);
            else return false;
        } else return false;
        
        // vt
        if (std::getline(stream, part, '/')) {
            if (!part.empty()) key.vt = std::stoi(part);
        }
        
        // vn
        if (std::getline(stream, part, '/')) {
            if (!part.empty()) key.vn = std::stoi(part);
        }
        return true;
    }

    // Loads an OBJ file into a Mesh object
    inline Mesh loadOBJ(const std::string& filepath) {
        Mesh finalMesh;
        std::vector<Vertex3D> verticesOut;
        std::vector<unsigned int> indicesOut;

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << ">> [MODEL LOADER] Failed to open OBJ file: " << filepath << std::endl;
            return finalMesh; // Return empty mesh
        }

        std::vector<Vec3> temp_vertices;
        std::vector<Vec2> temp_uvs;
        std::vector<Vec3> temp_normals;
        
        std::map<VertexKey, unsigned int> uniqueVertices;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                Vec3 vertex;
                iss >> vertex.x >> vertex.y >> vertex.z;
                temp_vertices.push_back(vertex);
            } 
            else if (prefix == "vt") {
                Vec2 uv;
                iss >> uv.x >> uv.y;
                temp_uvs.push_back(uv);
            } 
            else if (prefix == "vn") {
                Vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                temp_normals.push_back(normal);
            } 
            else if (prefix == "f") {
                std::string vertexStr;
                std::vector<VertexKey> faceKeys;
                
                while (iss >> vertexStr) {
                    VertexKey key;
                    if (parseFaceVertex(vertexStr, key)) {
                        faceKeys.push_back(key);
                    }
                }

                if (faceKeys.size() >= 3) {
                    // Simple triangulation (assumes convex polygons, breaking into fans)
                    for (size_t i = 1; i < faceKeys.size() - 1; ++i) {
                        VertexKey keys[3] = {faceKeys[0], faceKeys[i], faceKeys[i+1]};
                        
                        for (int j = 0; j < 3; ++j) {
                            VertexKey currentKey = keys[j];
                            
                            // Check if this combined vertex has been seen before
                            auto it = uniqueVertices.find(currentKey);
                            if (it == uniqueVertices.end()) {
                                // Create new vertex
                                Vertex3D vertex;
                                
                                // OBJ indices are 1-based, convert to 0-based
                                if (currentKey.v > 0 && currentKey.v <= (int)temp_vertices.size()) {
                                    Vec3 p = temp_vertices[currentKey.v - 1];
                                    vertex.px = p.x;
                                    vertex.py = p.y;
                                    vertex.pz = p.z;
                                }
                                
                                if (currentKey.vt > 0 && currentKey.vt <= (int)temp_uvs.size()) {
                                    // Assimp or Blender might flip V
                                    Vec2 uv = temp_uvs[currentKey.vt - 1];
                                    vertex.u = uv.x;
                                    vertex.v = uv.y;
                                } else {
                                    vertex.u = 0; vertex.v = 0;
                                }
                                
                                if (currentKey.vn > 0 && currentKey.vn <= (int)temp_normals.size()) {
                                    Vec3 n = temp_normals[currentKey.vn - 1];
                                    vertex.nx = n.x;
                                    vertex.ny = n.y;
                                    vertex.nz = n.z;
                                } else {
                                    // Fallback placeholder normal
                                    vertex.nx = 0; vertex.ny = 1; vertex.nz = 0;
                                }
                                
                                // Default color (can be customized via shader uniforms later)
                                vertex.r = 1.0f; vertex.g = 1.0f; vertex.b = 1.0f; vertex.a = 1.0f;

                                unsigned int newIndex = (unsigned int)verticesOut.size();
                                uniqueVertices[currentKey] = newIndex;
                                verticesOut.push_back(vertex);
                                indicesOut.push_back(newIndex);
                            } else {
                                // Reuse existing vertex index
                                indicesOut.push_back(it->second);
                            }
                        }
                    }
                }
            }
        }

        std::cout << ">> [MODEL LOADER] Loaded OBJ: " << filepath 
                  << " (Verts: " << verticesOut.size() << ", Indices: " << indicesOut.size() << ")" << std::endl;

        finalMesh.upload(verticesOut, indicesOut);
        return finalMesh;
    }
}
