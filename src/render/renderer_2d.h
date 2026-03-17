#pragma once
#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include "core/rocket_state.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// ==========================================
// Part 0: 数学常量与辅助工具
// ==========================================

// 简单的二维向量(现统一使用 math3d.h 的 Vec2)
#include "math/math3d.h"

inline Vec2 rotateVec(float x, float y, float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return {x * c - y * s, x * s + y * c};
}

inline float my_lerp(float a, float b, float t) { return a + t * (b - a); }

// Utility fract function
inline float fract(float x) { return x - std::floor(x); }

// Simple hash for procedural generation
inline float hash11(float p) {
    p = fract(p * 0.1031f);
    p *= p + 33.33f;
    p *= p + p;
    return fract(p);
}

// ==========================================
// Renderer Class: modern OpenGL GUI/HUD
// ==========================================

static const char *vertexShaderSource2D = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec4 aColor;
    out vec4 ourColor;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
        ourColor = aColor;
    }
)";

static const char *fragmentShaderSource2D = R"(
    #version 330 core
    out vec4 FragColor;
    in vec4 ourColor;
    void main() {
        FragColor = ourColor;
    }
)";

class Renderer {
private:
  unsigned int shaderProgram, VBO, VAO;
  std::vector<float> vertices;
  unsigned int compileShader(unsigned int type, const char *source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    return id;
  }

public:
  Renderer() {
    unsigned int v = compileShader(GL_VERTEX_SHADER, vertexShaderSource2D);
    unsigned int f = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource2D);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, v);
    glAttachShader(shaderProgram, f);

    glLinkProgram(shaderProgram);
    glDeleteShader(v);
    glDeleteShader(f);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 预分配显存
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5000000, NULL,
                 GL_DYNAMIC_DRAW);

    // 位置属性 (location=0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    // 颜色属性 (location=1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  void beginFrame() { vertices.clear(); }

  void addVertex(float x, float y, float r, float g, float b, float a) {
    vertices.insert(vertices.end(), {x, y, r, g, b, a});
  }

  // 普通矩形
  void addRect(float x, float y, float w, float h, float r, float g, float b,
               float a = 1.0f) {
    addVertex(x - w / 2, y - h / 2, r, g, b, a);
    addVertex(x + w / 2, y - h / 2, r, g, b, a);
    addVertex(x + w / 2, y + h / 2, r, g, b, a);
    addVertex(x + w / 2, y + h / 2, r, g, b, a);
    addVertex(x - w / 2, y + h / 2, r, g, b, a);
    addVertex(x - w / 2, y - h / 2, r, g, b, a);
  }

  // 矩形边框
  void addRectOutline(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f, float thick = 0.002f) {
      addLine(x - w / 2, y - h / 2, x + w / 2, y - h / 2, thick, r, g, b, a);
      addLine(x + w / 2, y - h / 2, x + w / 2, y + h / 2, thick, r, g, b, a);
      addLine(x + w / 2, y + h / 2, x - w / 2, y + h / 2, thick, r, g, b, a);
      addLine(x - w / 2, y + h / 2, x - w / 2, y - h / 2, thick, r, g, b, a);
  }

  // 旋转矩形
  void addRotatedRect(float cx, float cy, float w, float h, float angle,
                      float r, float g, float b, float a = 1.0f) {
    Vec2 p1 = {-w / 2, -h / 2}, p2 = {w / 2, -h / 2}, p3 = {w / 2, h / 2},
         p4 = {-w / 2, h / 2};
    p1 = rotateVec(p1.x, p1.y, angle);
    p2 = rotateVec(p2.x, p2.y, angle);
    p3 = rotateVec(p3.x, p3.y, angle);
    p4 = rotateVec(p4.x, p4.y, angle);
    addVertex(cx + p1.x, cy + p1.y, r, g, b, a);
    addVertex(cx + p2.x, cy + p2.y, r, g, b, a);
    addVertex(cx + p3.x, cy + p3.y, r, g, b, a);
    addVertex(cx + p3.x, cy + p3.y, r, g, b, a);
    addVertex(cx + p4.x, cy + p4.y, r, g, b, a);
    addVertex(cx + p1.x, cy + p1.y, r, g, b, a);
  }

  // 旋转三角形
  void addRotatedTri(float cx, float cy, float w, float h, float angle, float r,
                     float g, float b, float a = 1.0f) {
    Vec2 p1 = {0, h / 2}, p2 = {-w / 2, -h / 2}, p3 = {w / 2, -h / 2};
    p1 = rotateVec(p1.x, p1.y, angle);
    p2 = rotateVec(p2.x, p2.y, angle);
    p3 = rotateVec(p3.x, p3.y, angle);
    addVertex(cx + p1.x, cy + p1.y, r, g, b, a);
    addVertex(cx + p2.x, cy + p2.y, r, g, b, a);
    addVertex(cx + p3.x, cy + p3.y, r, g, b, a);
  }

  // 画圆形
  // 画圆形
  void addCircle(float cx, float cy, float radius, float r, float g, float b, float a = 1.0f) {
    int segments = 64; // 从 360 减少到 64 提高性能
    for (int i = 0; i < segments; i++) {
        float theta1 = 2.0f * PI * float(i) / float(segments);
        float theta2 = 2.0f * PI * float(i + 1) / float(segments);
        addVertex(cx, cy, r, g, b, a);
        addVertex(cx + radius * cosf(theta1), cy + radius * sinf(theta1), r, g, b, a);
        addVertex(cx + radius * cosf(theta2), cy + radius * sinf(theta2), r, g, b, a);
    }
  }

  void addCircleOutline(float cx, float cy, float radius, float thick, float r, float g, float b, float a = 1.0f) {
    int segments = 64;
    for (int i = 0; i < segments; i++) {
        float theta1 = 2.0f * PI * float(i) / float(segments);
        float theta2 = 2.0f * PI * float(i + 1) / float(segments);
        addLine(cx + radius * cosf(theta1), cy + radius * sinf(theta1),
                cx + radius * cosf(theta2), cy + radius * sinf(theta2),
                thick, r, g, b, a);
    }
  }

  // 画直线
  void addLine(float x1, float y1, float x2, float y2, float thick, float r, float g, float b, float a = 1.0f) {
      float dx = x2 - x1;
      float dy = y2 - y1;
      float len = sqrtf(dx*dx + dy*dy);
      if (len < 1e-6f) return;
      float angle = atan2f(dy, dx);
      addRotatedRect((x1 + x2) * 0.5f, (y1 + y2) * 0.5f, len, thick, angle, r, g, b, a);
  }

  // 精细地球渲染
  void addEarthWithContinents(float cx, float cy, float radius,
                              float cam_angle) {
    int segments = 3600;
    const int NUM_CONTINENTS = 8;
    float cont_center[NUM_CONTINENTS];
    float cont_half_w[NUM_CONTINENTS];
    for (int c = 0; c < NUM_CONTINENTS; c++) {
      cont_center[c] = (float)c / NUM_CONTINENTS + hash11(c * 9973.0f) * 0.08f;
      if (cont_center[c] > 1.0f) cont_center[c] -= 1.0f;
      cont_half_w[c] = 0.03f + hash11(c * 7333.0f) * 0.07f;
    }

    for (int i = 0; i < segments; i++) {
      float theta1 = 2.0f * PI * float(i) / float(segments) + cam_angle;
      float theta2 = 2.0f * PI * float(i + 1) / float(segments) + cam_angle;
      float pos = float(i) / float(segments);
      bool is_land = false;
      for (int c = 0; c < NUM_CONTINENTS; c++) {
        float dist = std::abs(pos - cont_center[c]);
        if (dist > 0.5f) dist = 1.0f - dist;
        float coast_noise = hash11(i * 137.0f + c * 5171.0f) * 0.015f;
        if (dist < cont_half_w[c] + coast_noise) {
          is_land = true;
          break;
        }
      }
      float biome_hash = hash11(float(i * 419));
      float r, g, b;
      float lat = std::abs(float(i % 1800) - 900.0f) / 900.0f;
      if (!is_land) {
        float depth = hash11(float(i * 503)) * 0.3f;
        r = 0.05f + lat * 0.1f;
        g = 0.25f + depth * 0.15f + lat * 0.15f;
        b = 0.55f + depth * 0.25f + lat * 0.1f;
      } else {
        if (lat > 0.85f) {
          r = 0.85f; g = 0.9f; b = 0.95f;
        } else if (lat < 0.3f && biome_hash > 0.5f) {
          r = 0.7f + biome_hash * 0.15f; g = 0.55f + biome_hash * 0.1f; b = 0.3f;
        } else {
          float lush = hash11(float(i * 631)) * 0.15f;
          r = 0.15f + lush; g = 0.4f + lush + lat * 0.1f; b = 0.12f + lush * 0.5f;
        }
      }
      addVertex(cx, cy, r, g, b, 1.0f);
      addVertex(cx + radius * std::cos(theta1), cy + radius * std::sin(theta1), r, g, b, 1.0f);
      addVertex(cx + radius * std::cos(theta2), cy + radius * std::sin(theta2), r, g, b, 1.0f);
    }
  }

  // 大气光环
  void addAtmosphereGlow(float cx, float cy, float radius, float cam_angle) {
    int segments = 360;
    float atmo_t = radius * 0.015f;
    for (int i = 0; i < segments; i++) {
      float t1 = 2.0f * PI * float(i) / float(segments) + cam_angle;
      float t2 = 2.0f * PI * float(i + 1) / float(segments) + cam_angle;
      float ir = radius, or_ = radius + atmo_t;
      addVertex(cx + ir * std::cos(t1), cy + ir * std::sin(t1), 0.4f, 0.6f, 1.0f, 0.4f);
      addVertex(cx + ir * std::cos(t2), cy + ir * std::sin(t2), 0.4f, 0.6f, 1.0f, 0.4f);
      addVertex(cx + or_ * std::cos(t1), cy + or_ * std::sin(t1), 0.3f, 0.5f, 0.9f, 0.0f);
      addVertex(cx + or_ * std::cos(t1), cy + or_ * std::sin(t1), 0.3f, 0.5f, 0.9f, 0.0f);
      addVertex(cx + ir * std::cos(t2), cy + ir * std::sin(t2), 0.4f, 0.6f, 1.0f, 0.4f);
      addVertex(cx + or_ * std::cos(t2), cy + or_ * std::sin(t2), 0.3f, 0.5f, 0.9f, 0.0f);
    }
  }

  // 姿态球 (Navball)
  // pitch: [-PI/2, PI/2], yaw: [0, 2*PI], roll: [-PI, PI]
  void drawAttitudeSphere(float cx, float cy, float radius, const Quat& qRocket, const Vec3& localRight, const Vec3& localUp, const Vec3& localNorth, bool sas_active, bool rcs_active, 
                          const Vec3& vPrograde, const Vec3& vNormal, const Vec3& vRadial, const Vec3& vManeuver = Vec3(0,0,0)) {
    // 1. 基础圆盘与底座
    addCircle(cx, cy, radius * 1.05f, 0.1f, 0.1f, 0.12f, 0.9f);
    addCircle(cx, cy, radius, 0.2f, 0.2f, 0.22f, 1.0f);

    // --- SAS & RCS Indicators ---
    float ind_r = radius * 0.15f;
    float ind_y = cy + radius * 0.7f;
    
    // SAS Indicator (Left)
    float sas_x = cx - radius * 1.25f;
    addCircle(sas_x, ind_y, ind_r, 0.1f, 0.1f, 0.1f, 0.8f); // Background
    if (sas_active) {
        addCircle(sas_x, ind_y, ind_r * 0.8f, 0.2f, 1.0f, 0.4f, 1.0f); // Green Glow
        drawText(sas_x, ind_y - ind_r * 2.2f, "SAS", radius * 0.12f, 0.2f, 1.0f, 0.4f, 1.0f, true, CENTER);
    } else {
        addCircle(sas_x, ind_y, ind_r * 0.8f, 0.3f, 0.1f, 0.1f, 1.0f); // Dim Red
        drawText(sas_x, ind_y - ind_r * 2.2f, "SAS", radius * 0.12f, 0.5f, 0.5f, 0.5f, 0.6f, true, CENTER);
    }

    // RCS Indicator (Right)
    float rcs_x = cx + radius * 1.25f;
    addCircle(rcs_x, ind_y, ind_r, 0.1f, 0.1f, 0.1f, 0.8f); // Background
    if (rcs_active) {
        addCircle(rcs_x, ind_y, ind_r * 0.8f, 0.2f, 0.8f, 1.0f, 1.0f); // Blue Glow
        drawText(rcs_x, ind_y - ind_r * 2.2f, "RCS", radius * 0.12f, 0.2f, 0.8f, 1.0f, 1.0f, true, CENTER);
    } else {
        addCircle(rcs_x, ind_y, ind_r * 0.8f, 0.3f, 0.1f, 0.1f, 1.0f); // Dim Red
        drawText(rcs_x, ind_y - ind_r * 2.2f, "RCS", radius * 0.12f, 0.5f, 0.5f, 0.5f, 0.6f, true, CENTER);
    }

    // 2. 坐标转换逻辑 (直接基于四元数投影，消除万向锁)
    // p 处于水平参考系 (X=East, Y=Up, Z=North)
    auto project = [&](Vec3 p) -> Vec2 {
        // 先转到世界空间
        Vec3 pW = localRight * p.x + localUp * p.y + localNorth * p.z;
        // 再转到火箭局部空间
        Vec3 pR = qRocket.conjugate().rotate(pW);
        // 火箭纵轴为 Y+ (Depth)，左右为 X+，上下为 Z+ (Screen Y)
        if (pR.y < 0) return {999, 999}; 
        return {cx + pR.x * radius, cy + pR.z * radius};
    };

    auto getSpherePt = [&](float lat, float lon) -> Vec3 {
        float phi = lat * (PI / 180.0f), theta = lon * (PI / 180.0f);
        return { cosf(phi) * sinf(theta), sinf(phi), cosf(phi) * cosf(theta) };
    };

    // 3. 绘制色块 (天空与地面) - 提高精细度
    int lat_steps = 24; // 增加步长
    int lon_steps = 48;
    for (int i = 0; i < lat_steps; i++) {
        float lat1 = -90.0f + (float)i / lat_steps * 180.0f;
        float lat2 = -90.0f + (float)(i + 1) / lat_steps * 180.0f;
        for (int j = 0; j < lon_steps; j++) {
            float lon1 = (float)j / lon_steps * 360.0f;
            float lon2 = (float)(j + 1) / lon_steps * 360.0f;
            Vec3 v1 = getSpherePt(lat1, lon1), v2 = getSpherePt(lat1, lon2);
            Vec3 v3 = getSpherePt(lat2, lon2), v4 = getSpherePt(lat2, lon1);
            Vec2 p1 = project(v1), p2 = project(v2), p3 = project(v3), p4 = project(v4);
            if (p1.x < 900 || p2.x < 900 || p3.x < 900 || p4.x < 900) {
                Vec3 col = (lat1 + lat2 > 0) ? Vec3(0.08f, 0.45f, 0.88f) : Vec3(0.58f, 0.32f, 0.15f);
                auto safeAddTri = [&](Vec2 a, Vec2 b, Vec2 c) {
                    if (a.x > 900 || b.x > 900 || c.x > 900) return;
                    addVertex(a.x, a.y, col.x, col.y, col.z, 1.0f);
                    addVertex(b.x, b.y, col.x, col.y, col.z, 1.0f);
                    addVertex(c.x, c.y, col.x, col.y, col.z, 1.0f);
                };
                safeAddTri(p1, p2, p3); safeAddTri(p1, p3, p4);
            }
        }
    }

    // 4. 绘制地平线 (Horizon Line)
    for (int j = 0; j < 128; j++) {
        float lon1 = (float)j / 128 * 360.0f, lon2 = (float)(j + 1) / 128 * 360.0f;
        Vec2 p1 = project(getSpherePt(0, lon1)), p2 = project(getSpherePt(0, lon2));
        if (p1.x < 900 && p2.x < 900) addLine(p1.x, p1.y, p2.x, p2.y, 0.004f, 1, 1, 1, 1.0f);
    }

    // 5. 绘制俯仰刻度 (Pitch Lines) 与 刻字
    for (int lat = -80; lat <= 80; lat += 10) {
        if (lat == 0) continue;
        for (int j = 0; j < 96; j++) {
            float lon1 = (float)j / 96 * 360.0f, lon2 = (float)(j + 1) / 96 * 360.0f;
            Vec2 p1 = project(getSpherePt((float)lat, lon1)), p2 = project(getSpherePt((float)lat, lon2));
            if (p1.x < 900 && p2.x < 900) {
                float opacity = (lat % 30 == 0) ? 0.6f : 0.3f;
                addLine(p1.x, p1.y, p2.x, p2.y, 0.0012f, 1, 1, 1, opacity);
            }
        }
        // 刻字：不仅在 0 度，更在环绕四周，并随边缘淡出
        for (int deg = 0; deg < 360; deg += 90) {
            Vec3 vL = getSpherePt((float)lat, (float)deg);
            Vec3 pW = localRight * vL.x + localUp * vL.y + localNorth * vL.z;
            Vec3 pR = qRocket.conjugate().rotate(pW);
            if (pR.y > 0.1f) { // y 是深度，y > 0 在正面
                float alpha = fminf(1.0f, pR.y * 3.0f); // 边缘淡出以增强“球体感”
                Vec2 plabel = {cx + pR.x * radius, cy + pR.z * radius};
                char buf[8]; snprintf(buf, sizeof(buf), "%d", lat);
                drawText(plabel.x, plabel.y, buf, radius * 0.09f, 1, 1, 1, 0.8f * alpha, true, CENTER);
            }
        }
    }

    // 6. 绘制垂直经线 (Meridians) 与 航向标签
    for (int lon = 0; lon < 360; lon += 30) {
        for (int i = 0; i < 36; i++) {
            float lat1 = -90.0f + (float)i * 5.0f, lat2 = lat1 + 5.0f;
            Vec2 p1 = project(getSpherePt(lat1, (float)lon)), p2 = project(getSpherePt(lat2, (float)lon));
            if (p1.x < 900 && p2.x < 900) addLine(p1.x, p1.y, p2.x, p2.y, 0.001f, 1, 1, 1, 0.3f);
        }
        // 航向标签 (N, E, S, W) - 增加淡出以增强“刻在球上”的感觉
        const char* marks[] = {"N", "30", "60", "E", "120", "150", "S", "210", "240", "W", "300", "330"};
        Vec3 vM = getSpherePt(0, (float)lon);
        Vec3 pW_m = localRight * vM.x + localUp * vM.y + localNorth * vM.z;
        Vec3 pR_m = qRocket.conjugate().rotate(pW_m);
        if (pR_m.y > 0.05f) {
            float alpha = fminf(1.0f, pR_m.y * 4.0f);
            Vec2 pmark = {cx + pR_m.x * radius, cy + pR_m.z * radius};
            drawText(pmark.x, pmark.y, marks[lon/30], radius * 0.12f, 1, 1, 0.1f, 0.9f * alpha, true, CENTER);
        }
    }

    // 7. 绘制中央固定指示器 (Level Indicator) - "The Wings"
    // 增加一个半透明黑色底座来消除各种细碎线条造成的闪烁/遮光感
    addCircle(cx, cy, radius * 0.12f, 0, 0, 0, 0.4f);
    float iw = radius * 0.45f;
    float ih = radius * 0.045f;
    // 阴影/描边 (黑色)
    addRect(cx - iw * 0.7f, cy, iw * 0.52f, ih * 1.5f, 0, 0, 0, 0.8f);
    addRect(cx + iw * 0.7f, cy, iw * 0.52f, ih * 1.5f, 0, 0, 0, 0.8f);
    addRect(cx, cy, ih * 3.5f, ih * 3.5f, 0, 0, 0, 0.8f);
    // 主体 (橘色)
    addRect(cx - iw * 0.7f, cy, iw * 0.5f, ih, 1.0f, 0.6f, 0.0f, 1.0f); // L
    addRect(cx + iw * 0.7f, cy, iw * 0.5f, ih, 1.0f, 0.6f, 0.0f, 1.0f); // R
    addRect(cx, cy, ih * 2.5f, ih * 2.5f, 1.0f, 0.6f, 0.0f, 1.0f);      // C

    // 8. 绘制轨道标记 (Orbital Markers)
    auto drawMarker = [&](Vec3 v, const char* type, float r, float g, float b) {
        if (v.length() < 0.1f) return;
        
        // 直接投影到火箭局部空间 (v 已经是世界方向向量)
        Vec3 pR = qRocket.conjugate().rotate(v);
        
        if (pR.y < 0.05f) return; // 球背面不绘制
        
        float alpha = fminf(1.0f, pR.y * 5.0f);
        float mx = cx + pR.x * radius;
        float my = cy + pR.z * radius;
        float ms = radius * 0.14f;
        float lw = 0.003f;
        
        // Hollow designs: Use outlines only to avoid blocking text
        if (std::string(type) == "PRO") { // Prograde: Hollow Green Circle with wings
            addCircle(mx, my, ms * 0.8f, r, g, b, alpha * 0.3f); // Very faint internal circle
            addCircleOutline(mx, my, ms * 0.8f, lw, r, g, b, alpha);
            addLine(mx, my - ms * 0.8f, mx, my - ms * 1.4f, lw, r, g, b, alpha);
            addLine(mx - ms * 0.8f, my, mx - ms * 1.4f, my, lw, r, g, b, alpha);
            addLine(mx + ms * 0.8f, my, mx + ms * 1.4f, my, lw, r, g, b, alpha);
            addCircle(mx, my, ms * 0.15f, r, g, b, alpha); // Small center dot
        } else if (std::string(type) == "RET") { // Retrograde: Hollow Yellow Circle with wings
            addCircleOutline(mx, my, ms * 0.8f, lw, r, g, b, alpha);
            addLine(mx, my - ms * 0.4f, mx, my - ms * 0.8f, lw, r, g, b, alpha);
            addLine(mx - ms * 0.4f, my, mx - ms * 0.8f, my, lw, r, g, b, alpha);
            addLine(mx + ms * 0.4f, my, mx + ms * 0.8f, my, lw, r, g, b, alpha);
            drawText(mx, my, "X", ms * 0.8f, r, g, b, alpha, true, Renderer::CENTER);
        } else if (std::string(type) == "NRM") { // Normal: Hollow Purple Triangle
            addCircleOutline(mx, my, ms * 0.4f, lw, r, g, b, alpha * 0.5f);
            // Draw hollow triangle using 3 lines
            float h = ms * 1.8f;
            float w2 = h * 0.6f;
            float ty = my - h * 0.4f;
            addLine(mx, ty, mx - w2, ty + h, lw, r, g, b, alpha);
            addLine(mx - w2, ty + h, mx + w2, ty + h, lw, r, g, b, alpha);
            addLine(mx + w2, ty + h, mx, ty, lw, r, g, b, alpha);
        } else if (std::string(type) == "ANT") { // Anti-normal: Hollow Purple Inv-Triangle with X
            float h = ms * 1.8f;
            float w2 = h * 0.6f;
            float ty = my + h * 0.4f;
            addLine(mx, ty, mx - w2, ty - h, lw, r, g, b, alpha);
            addLine(mx - w2, ty - h, mx + w2, ty - h, lw, r, g, b, alpha);
            addLine(mx + w2, ty - h, mx, ty, lw, r, g, b, alpha);
            drawText(mx, my - ms * 0.2f, "X", ms * 0.7f, r, g, b, alpha, true, Renderer::CENTER);
        } else if (std::string(type) == "R-I") { // Radial In: Deep Blue hollow circle with inward lines
            addCircleOutline(mx, my, ms * 1.0f, lw, r, g, b, alpha);
            for(int k=0; k<4; k++){
                float ang = k * PI/2.0f;
                addLine(mx + cosf(ang)*ms, my + sinf(ang)*ms, mx + cosf(ang)*ms*0.4f, my + sinf(ang)*ms*0.4f, lw, r, g, b, alpha);
            }
        } else if (std::string(type) == "R-O") { // Radial Out: Deep Blue hollow circle with outward lines
            addCircleOutline(mx, my, ms * 0.7f, lw, r, g, b, alpha);
            for(int k=0; k<4; k++){
                float ang = (k * PI/2.0f) + (PI/4.0f);
                addLine(mx + cosf(ang)*ms*0.7f, my + sinf(ang)*ms*0.7f, mx + cosf(ang)*ms*1.3f, my + sinf(ang)*ms*1.3f, lw, r, g, b, alpha);
            }
        } else if (std::string(type) == "MNV") { // Maneuver: Blue "Star" / Cross
            ms *= 1.1f;
            drawText(mx, my, "+", ms * 1.5f, r, g, b, alpha, true, Renderer::CENTER);
            addCircleOutline(mx, my, ms * 0.5f, lw, r, g, b, alpha);
        }
    };

    drawMarker(vPrograde, "PRO", 0.2f, 0.8f, 0.2f);
    drawMarker(vPrograde * -1.0f, "RET", 0.8f, 0.8f, 0.1f);
    drawMarker(vNormal, "NRM", 0.7f, 0.2f, 0.8f);
    drawMarker(vNormal * -1.0f, "ANT", 0.7f, 0.2f, 0.8f);
    // Deep blue for radial markers: R=0.1, G=0.2, B=0.9
    drawMarker(vRadial, "R-I", 0.1f, 0.2f, 0.9f);
    drawMarker(vRadial * -1.0f, "R-O", 0.1f, 0.2f, 0.9f);
    
    // Maneuver target
    if (vManeuver.length() > 0.1f) {
        drawMarker(vManeuver, "MNV", 0.2f, 0.6f, 1.0f);
    }
  }

private:
  uint64_t getGlyph5x7(char c) const {
      if (c >= 'a' && c <= 'z') c -= 32;
      switch (c) {
        case '0': return 0x1C22262A32221CULL; case '1': return 0x080C080808081CULL;
        case '2': return 0x1C22020C10203EULL; case '3': return 0x1C22020C02221CULL;
        case '4': return 0x040C14243E0404ULL; case '5': return 0x3E203C0202221CULL;
        case '6': return 0x1C22203C22221CULL; case '7': return 0x3E020408101010ULL;
        case '8': return 0x1C22221C22221CULL; case '9': return 0x1C22221E02221CULL;
        case 'A': return 0x1C22223E222222ULL; case 'B': return 0x3C22223C22223CULL;
        case 'C': return 0x1C22202020221CULL; case 'D': return 0x3C22222222223CULL;
        case 'E': return 0x3E20203C20203EULL; case 'F': return 0x3E20203C202020ULL;
        case 'G': return 0x1C22202E22221CULL; case 'H': return 0x2222223E222222ULL;
        case 'I': return 0x1C08080808081CULL; case 'J': return 0x0E040404042418ULL;
        case 'K': return 0x22242830282422ULL; case 'L': return 0x2020202020203EULL;
        case 'M': return 0x22362A22222222ULL; case 'N': return 0x22322A26222222ULL;
        case 'O': return 0x1C22222222221CULL; case 'P': return 0x3C22223C202020ULL;
        case 'Q': return 0x1C2222222A241AULL; case 'R': return 0x3C22223C282422ULL;
        case 'S': return 0x1C22100C02221CULL; case 'T': return 0x3E080808080808ULL;
        case 'U': return 0x2222222222221CULL; case 'V': return 0x22222222221408ULL;
        case 'W': return 0x2222222A2A3622ULL; case 'X': return 0x22221408142222ULL;
        case 'Y': return 0x22222214080808ULL; case 'Z': return 0x3E02040810203EULL;
        case '.': return 0x00000000000008ULL; case ':': return 0x00080000000800ULL;
        case '-': return 0x0000001C000000ULL; case '/': return 0x02040810204000ULL;
        case '%': return 0x22040810222412ULL; case '(': return 0x04080808080804ULL;
        case ')': return 0x08040404040408ULL; case '+': return 0x0008083E080800ULL;
        case '[': return 0x1E10101010101EULL; case ']': return 0x1E02020202021EULL;
        case '<': return 0x04081020100804ULL; case '>': return 0x10080402040810ULL;
        case ' ': return 0x0ULL;
        default: return 0;
      }
  }

  const uint16_t* getGlyph16x16(unsigned int unicode) const {
      static const uint16_t g_u8BBE[] = {0x0080,0x2084,0x1084,0x10F4,0x1084,0x1084,0xFFC4,0x10A4,0x1094,0x108C,0x1284,0x1480,0x08A0,0x0990,0x068E,0x0000};
      static const uint16_t g_u7F6E[] = {0x7FFE,0x4002,0x4002,0x7FFE,0x0100,0x0100,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0xFFFF};
      static const uint16_t g_u8BED[] = {0x0080,0x21FC,0x1084,0x1084,0x11FC,0x1084,0x1084,0xF1FC,0x0000,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8};
      static const uint16_t g_u8A00[] = {0x0100,0x0100,0x7FFE,0x0000,0x3FC0,0x0000,0x3FC0,0x0000,0x0100,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8};
      static const uint16_t g_u7EE7[] = {0x1040,0x1040,0x21FC,0x2144,0x4944,0x4944,0x1144,0x1144,0x2244,0x2254,0x444A,0x4441,0x1140,0x1140,0x0040,0x0040};
      static const uint16_t g_u7EED[] = {0x1080,0x1080,0x25FC,0x2484,0x4484,0x4484,0x1484,0x1484,0x2484,0x2484,0x4484,0x4484,0x1484,0x1484,0x0100,0x0100};
      static const uint16_t g_u65B0[] = {0x0440,0x0444,0x3FE4,0x0444,0x2444,0x2444,0x24FC,0x2444,0x2444,0x2444,0x2FD4,0x0424,0x0440,0x0440,0x0400,0x0000};
      static const uint16_t g_u6E38[] = {0x1080,0x10BE,0x24A2,0x24A2,0xE4A2,0x24A2,0x24BE,0x27A2,0x24A2,0x24A2,0x24A2,0x24A2,0x24AA,0x2492,0x2002,0x0000};
      static const uint16_t g_u620F[] = {0x0440,0x2440,0x2440,0x2440,0x24FE,0x2440,0x2440,0x2440,0x2442,0x2442,0x2444,0x2448,0x2450,0x2420,0x2410,0x0000};
      static const uint16_t g_u9000[] = {0x0080,0x0080,0x0080,0x3FFE,0x0080,0x0080,0x0080,0x0080,0x0080,0x4000,0x2000,0x1000,0x0800,0x0400,0x03FF,0x0000};
      static const uint16_t g_u51FA[] = {0x0100,0x0100,0x1110,0x1110,0x1110,0x1110,0x7F7E,0x0100,0x0100,0x1110,0x1110,0x1110,0x1110,0x7F7E,0x0100,0x0100};
      static const uint16_t g_u8FD4[] = {0x0100,0x0100,0x3F80,0x0100,0x0540,0x0920,0x1110,0x2108,0x4104,0x0102,0x01FE,0x0100,0x0100,0x0100,0x0100,0x0100};
      static const uint16_t g_u56DE[] = {0x0000,0x7FFE,0x4002,0x4002,0x47F2,0x4412,0x4412,0x4412,0x4412,0x4412,0x47F2,0x4002,0x4002,0x7FFE,0x0000,0x0000};
      static const uint16_t g_u786E[] = {0x1040,0x1040,0x21FC,0x2104,0x45FC,0x4504,0x15FC,0x1504,0x25FC,0x2504,0x45FC,0x1100,0x1104,0x110A,0x11F1,0x0000};
      static const uint16_t g_u8BA4[] = {0x0080,0x2080,0x1080,0x1080,0x11FC,0x1080,0x1080,0xF080,0x0080,0x0080,0x0100,0x0100,0x0200,0x0400,0x0800,0x1000};
      static const uint16_t g_u4E2D[] = {0x0100,0x0100,0x0100,0x7F7E,0x4102,0x4102,0x4102,0x7F7E,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100};
      static const uint16_t g_u6587[] = {0x0100,0x0100,0x7FFE,0x0000,0x0100,0x0100,0x0280,0x0280,0x0440,0x0440,0x0820,0x0820,0x1010,0x2018,0x400E,0x0000};
      static const uint16_t g_u82F1[] = {0x0000,0x0100,0x4104,0x4104,0x7FFE,0x0100,0x0100,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108};
      static const uint16_t g_u8D44[] = {0x2110,0x2110,0x2110,0x4928,0x4928,0x1144,0x1144,0x1144,0x2110,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0x3FF8};
      static const uint16_t g_u6E90[] = {0x1080,0x10BE,0x24A2,0x24A2,0xE4A2,0x24A2,0x24BE,0x2000,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0x3FF8,0x0000};
      static const uint16_t g_u5929[] = {0x7FFE,0x0000,0x0100,0x0100,0x3FF8,0x0100,0x0100,0x0280,0x0280,0x0440,0x0440,0x0820,0x1010,0x2008,0x4004,0x0000};
      static const uint16_t g_u77FF[] = {0x0080,0x1080,0x1080,0x1FFC,0x1080,0x1090,0x10A0,0x10C0,0x1180,0x1100,0x1200,0x1400,0x1000,0x1000,0x0000,0x0000};
      static const uint16_t g_u673A[] = {0x0810,0x0810,0x0810,0x7E10,0x0810,0x0810,0x0810,0x0810,0x0810,0x0BA0,0x1840,0x2820,0x4810,0x0810,0x0810,0x0810};
      static const uint16_t g_u91D1[] = {0x0100,0x0280,0x0440,0x0820,0x1010,0x3FE8,0x0100,0x7FFE,0x0100,0x0100,0x3BC8,0x0440,0x0820,0xFFFF,0x0000,0x0000};
      static const uint16_t g_u544A[] = {0x0100,0x0100,0x3FF8,0x0100,0x0100,0x7FFE,0x0000,0x0100,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8,0x0000};
      static const uint16_t g_u8B66[] = {0x1440,0x24C0,0x1F40,0x14A0,0x2440,0x1F40,0x0100,0x3BC8,0x0440,0x11FC,0x1084,0x1084,0xF084,0x0000,0x0100,0x0000};
      static const uint16_t g_u7194[] = {0x2008,0x27FC,0x2008,0x2008,0x23F8,0xAB08,0x6330,0x2320,0x2210,0xA338,0x6308,0x23F8,0x2208,0x2208,0x23F8,0x0000};
      static const uint16_t g_u7089[] = {0x2000,0x23F8,0x2210,0x2210,0x23F8,0xA310,0x67F0,0x2110,0x2110,0xA110,0x6110,0x2110,0x2110,0x21F0,0x2000,0x0000};
      static const uint16_t g_u88C5[] = {0x0100,0x1108,0x1104,0x1102,0x7FF1,0x0100,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8};
      static const uint16_t g_u914D[] = {0x7FFE,0x4002,0x47F2,0x4412,0x4412,0x4412,0x47F2,0x4002,0x7FFE,0x0100,0x3FF8,0x2108,0x2108,0x2108,0x3FF8,0x0000};
      static const uint16_t g_u4ED3[] = {0x0100,0x0280,0x0440,0x0820,0x1010,0x7FFE,0x0100,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2108,0x2208,0x3C08,0x0000};
      static const uint16_t g_u5E93[] = {0x0100,0x0080,0x7FFE,0x4000,0x4100,0x4100,0x4FFC,0x4104,0x4104,0x4104,0x4FFC,0x4104,0x4104,0x4104,0x401C,0x0000};
      static const uint16_t g_u5DE5[] = {0x7FFE,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0xFFFF};
      static const uint16_t g_u5382[] = {0x7FFE,0x4000,0x4000,0x4000,0x4000,0x2000,0x2000,0x1000,0x1000,0x0800,0x0800,0x0400,0x0400,0x0200,0x0100,0x0000};
      static const uint16_t g_u6982[] = {0x0800,0x0804,0x7E04,0x0804,0x0844,0x0844,0x0924,0x0A14,0x0C0C,0x1800,0x2FFE,0x4100,0x8100,0x0100,0x0100,0x0100};
      static const uint16_t g_u89C2[] = {0x0002,0x2002,0x1002,0x1202,0x1102,0x1082,0xFD7E,0x1000,0x1080,0x1040,0x1030,0x100C,0x1003,0x13E0,0xE000,0x0000};
      static const uint16_t g_u72B6[] = {0x1040,0x1040,0x11FC,0x1100,0x2120,0x2120,0x4110,0x1108,0x1204,0x1404,0x1810,0x1060,0x0000,0x0000,0x0000,0x0000};
      static const uint16_t g_u6001[] = {0x0000,0x3F00,0x2100,0x2100,0x2100,0x2100,0x3F00,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0x0000};
      static const uint16_t g_u5728[] = {0x0100,0x0100,0x7FFE,0x0100,0x0100,0x0200,0x0200,0x0400,0x07FE,0x0402,0x0402,0x0402,0x07FE,0x0400,0x0400,0xFFFF};
      static const uint16_t g_u7EBF[] = {0x1040,0x1040,0x21FC,0x2104,0x45FC,0x4504,0x15FC,0x1504,0x25FC,0x2504,0x45FC,0x1100,0x1104,0x110A,0x11F1,0x0000};
      static const uint16_t g_u79BB[] = {0x0100,0x7FFE,0x0000,0x3FF8,0x2008,0x3FF8,0x2008,0x3FF8,0x0000,0x0100,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x3FF8};
      static const uint16_t g_u94F1[] = {0x0080,0x2080,0x1080,0x1080,0x11FC,0x1080,0x1080,0xF080,0x0080,0x0080,0x0100,0x0100,0x0200,0x0400,0x0800,0x1000};
      static const uint16_t g_u7194_2[] = {0x2008,0x27FC,0x2008,0x2008,0x23F8,0xAB08,0x6330,0x2320,0x2210,0xA338,0x6308,0x23F8,0x2208,0x2208,0x23F8,0x0000};
      static const uint16_t g_u88C5_2[] = {0x0100,0x1108,0x1104,0x1102,0x7FF1,0x0100,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8};
      static const uint16_t g_u8D44_2[] = {0x2110,0x2110,0x2110,0x4928,0x4928,0x1144,0x1144,0x1144,0x2110,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0x3FF8};
      static const uint16_t g_u96F6[] = {0x3FF8,0x2008,0x2008,0x3FF8,0x0100,0x7FFE,0x0100,0x0100,0x3FF8,0x0100,0x0100,0x0280,0x0440,0x0820,0x1010,0x0000};
      static const uint16_t g_u4EF6[] = {0x0800,0x1800,0x18FE,0x2880,0x4880,0x0880,0x08FE,0x0880,0x0880,0x0880,0x08FE,0x0880,0x0880,0x0880,0x0880,0x0000};
      static const uint16_t g_u6CA1[] = {0x1000,0x10BE,0x24A2,0x24A2,0xE4A2,0x24A2,0x24BE,0x2000,0x3FF8,0x2108,0x3FF8,0x2108,0x3FF8,0x2108,0x3FF8,0x0000};
      static const uint16_t g_u6709[] = {0x0100,0x0100,0x7FFE,0x0100,0x0100,0x0200,0x0200,0x0400,0x07FE,0x0402,0x0402,0x0402,0x07FE,0x0400,0x0400,0xFFFF};
      static const uint16_t g_u7269[] = {0x0820,0x0820,0x7E20,0x0820,0x0820,0x0BF8,0x1A28,0x2A28,0x4BF8,0x0A28,0x0A28,0x0BF8,0x0A28,0x0A28,0x0A28,0x0000};
      static const uint16_t g_u4EA7[] = {0x0100,0x0080,0x7FFE,0x4000,0x4100,0x4100,0x4FFC,0x4104,0x4104,0x4104,0x4FFC,0x4104,0x4104,0x4104,0x401C,0x0000};
      static const uint16_t g_u7EC4[] = {0x1040,0x1040,0x21FC,0x2144,0x4944,0x4944,0x1144,0x1144,0x2244,0x2254,0x444A,0x4441,0x1140,0x1140,0x0040,0x0040};
      static const uint16_t g_u8F66[] = {0x0100,0x0100,0x7FFE,0x0810,0x0810,0x3FF8,0x0810,0x0810,0x0810,0x0810,0x3FF8,0x0810,0x0810,0x0810,0x0810,0x0000};
      static const uint16_t g_u95F4[] = {0x7FFE,0x4002,0x4002,0x4002,0x47F2,0x4412,0x4412,0x4412,0x4412,0x4412,0x47F2,0x4002,0x4002,0x7FFE,0x0000,0x0000};
      static const uint16_t g_u53D1[] = {0x0080,0x2080,0x1080,0x1080,0x11FC,0x1080,0x1080,0xF080,0x0080,0x0080,0x0100,0x0100,0x0200,0x0400,0x0800,0x1000};
      static const uint16_t g_u5C04[] = {0x1040,0x1040,0x21FC,0x2104,0x45FC,0x4504,0x15FC,0x1504,0x25FC,0x2504,0x45FC,0x1100,0x1104,0x110A,0x11F1,0x0000};
      static const uint16_t g_u573A[] = {0x0100,0x0100,0x7FFE,0x0100,0x0100,0x0200,0x0200,0x0400,0x07FE,0x0402,0x0402,0x0402,0x07FE,0x0400,0x0400,0xFFFF};
      static const uint16_t g_u8FDB[] = {0x0100,0x0100,0x3F80,0x0100,0x0540,0x0920,0x1110,0x2108,0x4104,0x0102,0x01FE,0x0100,0x0100,0x0100,0x0100,0x0100};
      static const uint16_t g_u5165[] = {0x0100,0x0100,0x0100,0x0280,0x0280,0x0440,0x0440,0x0820,0x0820,0x1010,0x1010,0x2008,0x2008,0x4004,0x4004,0x0000};
      static const uint16_t g_u6A21[] = {0x0800,0x0800,0x7F00,0x0820,0x0820,0x0BF8,0x1A28,0x2A28,0x4BF8,0x0A28,0x0A28,0x0BF8,0x0A28,0x0A28,0x0A28,0x0000};
      static const uint16_t g_u5F0F[] = {0x0100,0x0100,0x7FFE,0x0100,0x0100,0x3FF8,0x0100,0x0100,0x0280,0x0440,0x0820,0x1010,0x2008,0x4004,0x0000,0x0000};
      static const uint16_t g_u4E3B[] = {0x0100,0x7FFE,0x0100,0x0100,0x3FF8,0x0100,0x0100,0x1010,0x1010,0x1010,0x7FFE,0x1010,0x1010,0x1010,0x1010,0x0000};
      static const uint16_t g_u83DC[] = {0x0000,0x7FFE,0x0000,0x0100,0x0100,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0x3FF8,0x2108,0x2108,0x7FFE,0x0000};
      static const uint16_t g_u5355[] = {0x0100,0x0100,0x7FFE,0x0000,0x3FC0,0x0000,0x3FC0,0x0000,0x0100,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8};
      static const uint16_t g_u822A[] = {0x0800,0x0800,0x7F00,0x0820,0x0820,0x0BF8,0x1A28,0x2A28,0x4BF8,0x0A28,0x0A28,0x0BF8,0x0A28,0x0A28,0x0A28,0x0000};
      static const uint16_t g_u5C40[] = {0x7FFE,0x4002,0x4002,0x47F2,0x4412,0x4412,0x4412,0x4412,0x4412,0x47F2,0x4002,0x4002,0x7FFE,0x0000,0x0000,0x0000};
      static const uint16_t g_u77FF_2[] = {0x0080,0x1080,0x1080,0x1FFC,0x1080,0x1090,0x10A0,0x10C0,0x1180,0x1100,0x1200,0x1400,0x1000,0x1000,0x0000,0x0000};
      static const uint16_t g_u7194_3[] = {0x2008,0x27FC,0x2008,0x2008,0x23F8,0xAB08,0x6330,0x2320,0x2210,0xA338,0x6308,0x23F8,0x2208,0x2208,0x23F8,0x0000};
      static const uint16_t g_u7089_2[] = {0x2000,0x23F8,0x2210,0x2210,0x23F8,0xA310,0x67F0,0x2110,0x2110,0xA110,0x6110,0x2110,0x2110,0x21F0,0x2000,0x0000};
      static const uint16_t g_u88C5_3[] = {0x0100,0x1108,0x1104,0x1102,0x7FF1,0x0100,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8,0x2008,0x2008,0x3FF8};
      static const uint16_t g_u77FF_3[] = {0x0080,0x1080,0x1080,0x1FFC,0x1080,0x1090,0x10A0,0x10C0,0x1180,0x1100,0x1200,0x1400,0x1000,0x1000,0x0000,0x0000};

      switch(unicode) {
        case 0x8BBE: return g_u8BBE; case 0x7F6E: return g_u7F6E; case 0x8BED: return g_u8BED; case 0x8A00: return g_u8A00;
        case 0x7EE7: return g_u7EE7; case 0x7EED: return g_u7EED; case 0x65B0: return g_u65B0; case 0x6E38: return g_u6E38;
        case 0x620F: return g_u620F; case 0x9000: return g_u9000; case 0x51FA: return g_u51FA; case 0x8FD4: return g_u8FD4;
        case 0x56DE: return g_u56DE; case 0x786E: return g_u786E; case 0x8BA4: return g_u8BA4; case 0x4E2D: return g_u4E2D;
        case 0x6587: return g_u6587; case 0x82F1: return g_u82F1; case 0x8D44: return g_u8D44; case 0x6E90: return g_u6E90;
        case 0x5929: return g_u5929; case 0x77FF: return g_u77FF; case 0x673A: return g_u673A; case 0x91D1: return g_u91D1;
        case 0x544A: return g_u544A; case 0x8B66: return g_u8B66; case 0x7194: return g_u7194; case 0x7089: return g_u7089;
        case 0x88C5: return g_u88C5; case 0x914D: return g_u914D; case 0x4ED3: return g_u4ED3; case 0x5E93: return g_u5E93;
        case 0x5DE5: return g_u5DE5; case 0x5382: return g_u5382; case 0x6982: return g_u6982; case 0x89C2: return g_u89C2;
        case 0x72B6: return g_u72B6; case 0x6001: return g_u6001; case 0x5728: return g_u5728; case 0x7EBF: return g_u7EBF;
        case 0x79BB: return g_u79BB; case 0x94F1: return g_u94F1; case 0x96F6: return g_u96F6; case 0x4EF6: return g_u4EF6;
        case 0x6CA1: return g_u6CA1; case 0x6709: return g_u6709; case 0x7269: return g_u7269; case 0x4EA7: return g_u4EA7;
        case 0x7EC4: return g_u7EC4; case 0x8F66: return g_u8F66; case 0x95F4: return g_u95F4; case 0x53D1: return g_u53D1;
        case 0x5C04: return g_u5C04; case 0x573A: return g_u573A; case 0x8FDB: return g_u8FDB; case 0x5165: return g_u5165;
        case 0x6A21: return g_u6A21; case 0x5F0F: return g_u5F0F; case 0x4E3B: return g_u4E3B; case 0x83DC: return g_u83DC;
        case 0x5355: return g_u5355; case 0x822A: return g_u822A; case 0x5C40: return g_u5C40;
        default: return nullptr;
      }
  }

public:
  enum Align { LEFT, CENTER, RIGHT };
  void drawText(float x, float y, const char* text, float size, float r, float g, float b, float a = 1.0f, bool shadow = true, Align align = LEFT) {
    if (!text || !text[0]) return;
    
    float px = size * 0.15f;
    float cw = px * 6.0f; // ASCII char width
    
    // Width calculation (UTF-8 aware)
    float total_w = 0;
    {
        int i = 0;
        while (text[i]) {
            unsigned char c = (unsigned char)text[i];
            if (c < 128) {
                total_w += cw;
                i++;
            } else if ((c & 0xE0) == 0xE0) { // 3-byte UTF-8
                total_w += cw * 2.0f; // Chinese chars are double width
                i += 3;
            } else {
                i++; // Fallback
            }
        }
        if (total_w > 0) total_w -= px; // Remove last trailing space
    }

    float ox_start = x;
    if (align == CENTER) ox_start -= total_w * 0.5f;
    else if (align == RIGHT) ox_start -= total_w;

    auto renderChar = [&](const char* charPtr, float ox, float oy, float cr, float cg, float cb, float ca, int& step) -> float {
        unsigned char c = (unsigned char)charPtr[0];
        step = 1;
        if (c < 128) {
            uint64_t gl = getGlyph5x7(c);
            if (gl != 0 || c == ' ') {
                for (int row = 0; row < 7; row++) {
                    int row_data = (int)((gl >> ((6 - row) * 8)) & 0xFF);
                    for (int col = 0; col < 5; col++) {
                        if (row_data & (1 << (5 - col))) addRect(ox + col * px, oy + (3 - row) * px, px * 1.35f, px * 1.35f, cr, cg, cb, ca);
                    }
                }
            }
            return cw;
        } else if ((c & 0xE0) == 0xE0) {
            step = 3;
            unsigned int unicode = (unsigned int)(((charPtr[0] & 0x0F) << 12) | ((charPtr[1] & 0x3F) << 6) | (charPtr[2] & 0x3F));
            const uint16_t* gl16 = getGlyph16x16(unicode);
            float cpx = px * 0.75f;
            if (gl16) {
                for (int row = 0; row < 16; row++) {
                    uint16_t rd = gl16[row];
                    for (int col = 0; col < 16; col++) {
                        if (rd & (1 << (15 - col))) addRect(ox + col * cpx, oy + (8 - row) * cpx, cpx * 1.1f, cpx * 1.1f, cr, cg, cb, ca);
                    }
                }
            } else {
                // Placeholder box for missing glyph
                addRectOutline(ox + 8 * cpx, oy, 16 * cpx, 16 * cpx, cr, cg, cb, ca * 0.5f, 0.001f);
            }
            return cw * 2.0f;
        }
        return cw;
    };

    if (shadow) {
        float sx_off = size * 0.08f;
        float sy_off = -size * 0.08f;
        int idx = 0;
        float cx = ox_start + sx_off;
        while (text[idx]) {
            int step = 1;
            cx += renderChar(text + idx, cx, y + sy_off, 0, 0, 0, a * 0.8f, step);
            idx += step;
        }
    }

    int idx = 0;
    float cx = ox_start;
    while (text[idx]) {
        int step = 1;
        cx += renderChar(text + idx, cx, y, r, g, b, a, step);
        idx += step;
    }
  }

  void drawInt(float x, float y, int val, float size, float r, float g, float b, float a = 1.0f, Align align = LEFT) {
      char buf[32]; snprintf(buf, sizeof(buf), "%d", val);
      drawText(x, y, buf, size, r, g, b, a, true, align);
  }

  void endFrame() {
    if (vertices.empty()) return;
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertices.size() / 6));
  }

  // 绘制星球图标 (Procedural 2.5D Thumbnail)
  void drawPlanetIcon(float cx, float cy, float radius, const CelestialBody& body, float t_icon) {
    // 1. 底座背景 (半透明黑)
    addCircle(cx, cy, radius * 1.1f, 0, 0, 0, 0.6f);
    
    // 2. 模拟 3D 旋转的投影 (类似姿态球但更简化)
    // 旋转四元数: 绕 Y 轴缓慢旋转
    float rot_ang = t_icon * 0.5f;
    Quat icon_rot = Quat::fromAxisAngle(Vec3(0, 1, 0), rot_ang);
    
    auto project_icon = [&](Vec3 p) -> Vec2 {
        Vec3 pR = icon_rot.conjugate().rotate(p);
        if (pR.y < 0) return {999, 999}; 
        return {cx + pR.x * radius, cy + pR.z * radius};
    };

    auto getSpherePt = [&](float lat_deg, float lon_deg) -> Vec3 {
        float phi = lat_deg * (PI / 180.0f), theta = lon_deg * (PI / 180.0f);
        return { cosf(phi) * sinf(theta), sinf(phi), cosf(phi) * cosf(theta) };
    };

    // 绘制色块与纹理感
    int lat_steps = 10;
    int lon_steps = 20;
    for (int i = 0; i < lat_steps; i++) {
        float lat1 = -90.0f + (float)i / lat_steps * 180.0f;
        float lat2 = -90.0f + (float)(i + 1) / lat_steps * 180.0f;
        for (int j = 0; j < lon_steps; j++) {
            float lon1 = (float)j / lon_steps * 360.0f;
            float lon2 = (float)(j + 1) / lon_steps * 360.0f;
            Vec3 v1 = getSpherePt(lat1, lon1), v2 = getSpherePt(lat1, lon2);
            Vec3 v3 = getSpherePt(lat2, lon2), v4 = getSpherePt(lat2, lon1);
            Vec2 p1 = project_icon(v1), p2 = project_icon(v2), p3 = project_icon(v3), p4 = project_icon(v4);
            
            if (p1.x < 900) {
                // 基础颜色
                float cr = body.r, cg = body.g, cb = body.b;
                
                // 简单噪声纹理 (基于经纬度)
                float noise = hash11(lat1 * 0.13f + lon1 * 0.17f);
                float dark = 0.85f + 0.15f * noise;
                
                // 极地效果
                if (std::abs(lat1) > 75.0f && (body.type == TERRESTRIAL || body.type == MOON)) {
                    cr = std::min(1.0f, cr + 0.4f); cg = std::min(1.0f, cg + 0.4f); cb = std::min(1.0f, cb + 0.4f);
                }
                
                // 气态巨行星条带
                if (body.type == GAS_GIANT || body.type == RINGED_GAS_GIANT) {
                    float band = sinf(lat1 * 0.2f) * 0.5f + 0.5f;
                    dark *= (0.7f + 0.3f * band);
                }

                // 光照阴影 (假定光从右上角来)
                float light = fmaxf(0.1f, v1.x * 0.5f + v1.y * 0.5f + 0.5f);
                
                auto safeAdd = [&](Vec2 va, Vec2 vb, Vec2 vc) {
                    if (va.x > 900 || vb.x > 900 || vc.x > 900) return;
                    addVertex(va.x, va.y, cr * dark * light, cg * dark * light, cb * dark * light, 1.0f);
                    addVertex(vb.x, vb.y, cr * dark * light, cg * dark * light, cb * dark * light, 1.0f);
                    addVertex(vc.x, vc.y, cr * dark * light, cg * dark * light, cb * dark * light, 1.0f);
                };
                safeAdd(p1, p2, p3); safeAdd(p1, p3, p4);
            }
        }
    }
    
    // 大气光圈 (如果是恒星或行星)
    if (body.type != MOON) {
        addAtmosphereGlow(cx, cy, radius, rot_ang);
    }
    
    // 描边
    addCircleOutline(cx, cy, radius, 0.002f, 1, 1, 1, 0.4f);
  }

  ~Renderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
  }

private:
};
