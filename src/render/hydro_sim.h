#pragma once
#include <vector>
#include <queue>
#include <algorithm>
#include "../math/math3d.h"

namespace Hydro {

struct HydroData {
    std::vector<float> accumulation; // Flow accumulation (River volume)
    std::vector<int> flowDir;       // D8 Flow direction index (0-7)
    std::vector<int> strahler;      // Strahler Stream Order
    std::vector<float> waterTable;   // Surface water / Lake depth
    std::vector<float> filledHeight; // Terrain height after depression filling
    std::vector<float> sedimentLoad; // Current sediment carrier load
};

class HydroSimulator {
public:
    int width, height;
    HydroData data;

    HydroSimulator(int w, int h) : width(w), height(h) {
        data.accumulation.assign(w * h, 0.0f);
        data.flowDir.assign(w * h, -1);
        data.strahler.assign(w * h, 0);
        data.waterTable.assign(w * h, 0.0f);
        data.filledHeight.assign(w * h, 0.0f);
        data.sedimentLoad.assign(w * h, 0.0f);
    }

    // Main Entry Point: Disabling coarse global erosion to prevent 78km-wide riverbed artifacts.
    // Carving will now be handled at the high-precision LOD level in the vertex shader.
    void simulate(const std::vector<float>& heightMap, const std::vector<float>& precipitation, const std::vector<float>& temperature) {
        fillDepressions(heightMap);
        calculateFlowDirections();
        calculateAccumulation(precipitation, temperature, heightMap);
        
        // --- Refined Sink Classification (Water Balance: Inflow vs Evaporation) ---
        for (int i = 0; i < width * height; i++) {
            float lakeDepth = data.filledHeight[i] - heightMap[i];
            if (lakeDepth > 0.0001f) { 
                // Local potential evaporation scale
                float pEvap = std::max(0.0f, temperature[i] + 15.0f) * 0.05f;
                float inflow = data.accumulation[i]; 

                if (inflow > pEvap * 1.5f) {
                    // Humid: Permanent lake with outlet.
                    // Boosted Breaching: High flow or proximity to coast carves deeper
                    float breachFactor = (inflow > 50.0f) ? 0.005f : 0.001f;
                    data.filledHeight[i] -= std::max(breachFactor, lakeDepth * 0.3f * (inflow / 200.0f));
                    data.filledHeight[i] = std::max(heightMap[i] + 0.0001f, data.filledHeight[i]);
                } else if (inflow > pEvap * 0.2f) {
                    // Semi-Arid: Seasonal/Ephemeral lake. Partial fill.
                    data.filledHeight[i] = heightMap[i] + lakeDepth * 0.2f;
                } else {
                    // Arid: Endorheic Basin / Salt Flat. Drain the "water" but keep the depression.
                    data.filledHeight[i] = heightMap[i];
                }
            }
        }
        
        calculateStrahlerOrder();
    }

private:
    void fillDepressions(const std::vector<float>& heightMap) {
        data.filledHeight = heightMap;
        
        struct Node {
            int x, y;
            float h;
            bool operator>(const Node& other) const { return h > other.h; }
        };
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::vector<bool> visited(width * height, false);

        float seaLevel = 0.45f;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                if (heightMap[idx] <= seaLevel) {
                    pq.push({x, y, heightMap[idx]});
                    visited[idx] = true;
                }
            }
        }

        int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

        while (!pq.empty()) {
            Node curr = pq.top(); pq.pop();

            for (int i = 0; i < 8; i++) {
                int nx = (curr.x + dx[i] + width) % width;
                int ny = std::clamp(curr.y + dy[i], 0, height - 1);
                int nIdx = ny * width + nx;

                if (!visited[nIdx]) {
                    visited[nIdx] = true;
                    // Add a slightly more significant epsilon (1e-5) to force a drainage slope
                    // This prevents massive flat "blobs" by giving water a direction
                    if (data.filledHeight[nIdx] < curr.h + 1e-5f) {
                        data.filledHeight[nIdx] = curr.h + 1e-5f;
                    }
                    pq.push({nx, ny, data.filledHeight[nIdx]});
                }
            }
        }
    }

    void calculateFlowDirections() {
        int dx[] = {1, 1, 0, -1, -1, -1, 0, 1}; 
        int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                float maxSlope = -1.0f; 
                int bestDir = -1;

                for (int i = 0; i < 8; i++) {
                    int nx = (x + dx[i] + width) % width;
                    int ny = std::clamp(y + dy[i], 0, height-1);
                    int nIdx = ny * width + nx;

                    float drop = data.filledHeight[idx] - data.filledHeight[nIdx];
                    if (drop > maxSlope) {
                        maxSlope = drop;
                        bestDir = i;
                    }
                }
                data.flowDir[idx] = bestDir;
            }
        }
    }

    void calculateAccumulation(const std::vector<float>& precip, const std::vector<float>& temp, const std::vector<float>& heightMap) {
        struct Cell { int idx; float h; };
        std::vector<Cell> sortedCells;
        sortedCells.reserve(width * height);
        for (int i = 0; i < width * height; i++) sortedCells.push_back({i, data.filledHeight[i]});
        
        std::sort(sortedCells.begin(), sortedCells.end(), [](const Cell& a, const Cell& b) {
            return a.h > b.h;
        });

        for (int i = 0; i < width * height; i++) {
            float evap = std::max(0.0f, temp[i] + 10.0f) * 0.009f; 
            float netWater = std::max(0.0f, precip[i] - evap); 
            if (heightMap[i] <= 0.45f) netWater = 0.0f;
            data.accumulation[i] = netWater;
            data.sedimentLoad[i] = 0.0f;
        }

        int dx[] = {1, 1, 0, -1, -1, -1, 0, 1}; 
        int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};

        for (auto& cell : sortedCells) {
            int idx = cell.idx;
            int dir = data.flowDir[idx];
            if (dir != -1) {
                int nx = (idx % width + dx[dir] + width) % width;
                int ny = std::clamp(idx / width + dy[dir], 0, height - 1);
                int nIdx = ny * width + nx;

                // --- Erosion & Sedimentation Logic ---
                if (heightMap[idx] > 0.45f) { // On Land
                    float slope = std::max(0.0001f, (data.filledHeight[idx] - data.filledHeight[nIdx]));
                    // Transport Capacity C = f(Volume, Slope)
                    float capacity = powf(data.accumulation[idx], 0.7f) * slope * 0.05f;
                    float diff = capacity - data.sedimentLoad[idx];
                    
                    float k_er = 0.15f; // Erosion/Deposition coefficient
                    float deltaH = diff * k_er;
                    
                    // Clamp deltaH to prevent unrealistic spikes/pits
                    deltaH = std::clamp(deltaH, -0.01f, 0.01f);

                    data.filledHeight[idx] -= deltaH;
                    data.sedimentLoad[idx] += deltaH;

                    // Propagate to downstream neighbor
                    data.accumulation[nIdx] += data.accumulation[idx];
                    data.sedimentLoad[nIdx] += std::max(0.0f, data.sedimentLoad[idx]);
                } else {
                    // Entering Ocean: Drop all sediment (Delta Formation)
                    if (data.sedimentLoad[idx] > 0.0f) {
                        data.filledHeight[idx] += data.sedimentLoad[idx] * 0.5f; // Deposit at coast
                        data.sedimentLoad[idx] = 0.0f;
                    }
                }
            }
        }
    }

    void calculateStrahlerOrder() {
        data.strahler.assign(width * height, 0);
        
        std::vector<std::vector<int>> contributors(width * height);
        int dx[] = {1, 1, 0, -1, -1, -1, 0, 1}; 
        int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};

        for (int i = 0; i < width * height; i++) {
            int dir = data.flowDir[i];
            if (dir != -1) {
                int nx = (i % width + dx[dir] + width) % width;
                int ny = std::clamp(i / width + dy[dir], 0, height - 1);
                contributors[ny * width + nx].push_back(i);
            }
        }

        struct Cell { int idx; float h; };
        std::vector<Cell> sortedCells;
        sortedCells.reserve(width * height);
        for (int i = 0; i < width * height; i++) sortedCells.push_back({i, data.filledHeight[i]});
        
        std::sort(sortedCells.begin(), sortedCells.end(), [](const Cell& a, const Cell& b) {
            return a.h > b.h;
        });

        for (auto& cell : sortedCells) {
            int idx = cell.idx;
            if (contributors[idx].empty()) {
                // Lower threshold for river tips (0.01 instead of 0.05)
                if (data.accumulation[idx] > 0.01f) data.strahler[idx] = 1;
                else data.strahler[idx] = 0;
            } else {
                int maxOrder = 0;
                int countMax = 0;
                for (int cIdx : contributors[idx]) {
                    int cOrder = data.strahler[cIdx];
                    if (cOrder > maxOrder) {
                        maxOrder = cOrder;
                        countMax = 1;
                    } else if (cOrder == maxOrder && maxOrder > 0) {
                        countMax++;
                    }
                }
                if (countMax > 1) data.strahler[idx] = maxOrder + 1;
                else data.strahler[idx] = maxOrder;
            }
        }
    }

public:
    void bake() {
        if (textureID == 0) glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // R: FilledHeight (for Lake flattening), G: Accumulation, B: Strahler, A: Placeholder
        std::vector<float> textureData(width * height * 4, 0.0f);
        for (int i = 0; i < width * height; i++) {
            textureData[i * 4 + 0] = data.filledHeight[i];
            textureData[i * 4 + 1] = data.accumulation[i];
            textureData[i * 4 + 2] = (float)data.strahler[i];
            textureData[i * 4 + 3] = (float)data.flowDir[i];
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, textureData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    GLuint textureID = 0;

    // --- STATIC UTILITY: Local Priority Flood for Quadtree Nodes ---
    // grid: a w*w grid of heights to be filled
    // boundaries: heights of pixels on the 4 edges (must be provided from global map)
    static void fillDepressionsLocal(int res, std::vector<float>& grid, const std::vector<float>& boundaries) {
        struct Node {
            int x, y;
            float h;
            bool operator>(const Node& other) const { return h > other.h; }
        };
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::vector<bool> visited(res * res, false);

        // 1. Seed boundaries from the global map
        for (int i = 0; i < res; i++) {
            // Top (y=0)
            pq.push({i, 0, boundaries[i]});
            visited[i] = true;
            // Bottom (y=res-1)
            pq.push({i, res - 1, boundaries[res + i]});
            visited[(res - 1) * res + i] = true;
            // Left (x=0)
            if (!visited[i * res]) {
                pq.push({0, i, boundaries[2 * res + i]});
                visited[i * res] = true;
            }
            // Right (x=res-1)
            if (!visited[i * res + res - 1]) {
                pq.push({res - 1, i, boundaries[3 * res + i]});
                visited[i * res + res - 1] = true;
            }
        }

        int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

        while (!pq.empty()) {
            Node curr = pq.top(); pq.pop();
            for (int i = 0; i < 8; i++) {
                int nx = curr.x + dx[i];
                int ny = curr.y + dy[i];
                if (nx < 0 || nx >= res || ny < 0 || ny >= res) continue;
                int nIdx = ny * res + nx;

                if (!visited[nIdx]) {
                    visited[nIdx] = true;
                    // Apply the same tiny gradient for drainage
                    if (grid[nIdx] < curr.h + 0.00001f) {
                        grid[nIdx] = curr.h + 0.00001f;
                    }
                    pq.push({nx, ny, grid[nIdx]});
                }
            }
        }
    }
};

} // namespace Hydro
