#pragma once
// ==========================================================
// center_visualizer.h — Center Point Visualization Rendering
// Renders visual markers for center of mass, lift, and thrust
// ==========================================================

#include "math/math3d.h"

// Forward declarations
struct Renderer3D;
struct Renderer;

namespace CenterVisualizer {
    // Marker types for different center points
    enum MarkerType {
        MARKER_COM,  // Center of Mass: Blue sphere
        MARKER_COL,  // Center of Lift: Yellow cone
        MARKER_COT   // Center of Thrust: Red cylinder
    };
    
    // Marker color constants (RGB)
    constexpr float COM_COLOR[3] = {0.2f, 0.5f, 1.0f};  // Blue
    constexpr float COL_COLOR[3] = {1.0f, 0.8f, 0.2f};  // Yellow
    constexpr float COT_COLOR[3] = {1.0f, 0.2f, 0.2f};  // Red
    
    // Marker size constants (meters) - increased for better visibility
    constexpr float COM_RADIUS = 0.8f;      // Sphere radius (increased from 0.3)
    constexpr float COL_RADIUS = 0.6f;      // Cone base radius (increased from 0.25)
    constexpr float COL_HEIGHT = 1.2f;      // Cone height (increased from 0.5)
    constexpr float COT_RADIUS = 0.5f;      // Cylinder radius (increased from 0.2)
    constexpr float COT_HEIGHT = 1.0f;      // Cylinder height (increased from 0.4)
    
    // Render a center point marker
    // Input: 3D renderer, position, marker type, rocket transform matrix
    void renderMarker(Renderer3D* renderer, const Vec3& pos, 
                     MarkerType type, const Mat4& rocketTransform);
    
    // Render label text (using 2D overlay)
    // Input: 2D renderer, 3D world position, text, view-projection matrix
    void renderLabel(Renderer* renderer2d, const Vec3& worldPos, 
                    const char* text, const Mat4& viewProj);
}
