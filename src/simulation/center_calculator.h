#pragma once
// ==========================================================
// center_calculator.h — Center Point Calculation Functions
// Calculates center of mass, lift, and thrust for rocket assemblies
// ==========================================================

#include "math/math3d.h"
#include <cstddef>

// Forward declarations
struct RocketAssembly;
struct PartDef;

namespace CenterCalculator {
    // Calculate center of mass
    // Input: RocketAssembly (contains all parts with positions and masses)
    // Output: Center of mass position (rocket local coordinate system)
    Vec3 calculateCenterOfMass(const RocketAssembly& assembly);
    
    // Calculate center of lift
    // Input: RocketAssembly (contains aerodynamic surface parts)
    // Output: Center of lift position (rocket local coordinate system)
    Vec3 calculateCenterOfLift(const RocketAssembly& assembly);
    
    // Calculate center of thrust
    // Input: RocketAssembly (contains engine parts)
    // Output: Center of thrust position (rocket local coordinate system)
    Vec3 calculateCenterOfThrust(const RocketAssembly& assembly);
    
    // Calculate assembly hash (used to detect configuration changes)
    size_t hashAssembly(const RocketAssembly& assembly);
    
    // Helper functions for part type identification
    bool isEngine(const PartDef& part);
    bool isAerodynamicSurface(const PartDef& part);
    float getAerodynamicArea(const PartDef& part);
}
