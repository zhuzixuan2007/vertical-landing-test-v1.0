// ==========================================================
// center_calculator.cpp — Center Point Calculation Implementation
// ==========================================================

#include "center_calculator.h"

// Include Renderer to satisfy rocket_builder.h inline functions
#include "render/renderer_2d.h"
// We need access to PART_CATALOG which is defined in rocket_builder.h
#include "simulation/rocket_builder.h"

#include <cmath>

namespace CenterCalculator {

// Helper: Check if a part is an engine
bool isEngine(const PartDef& part) {
    return part.category == CAT_ENGINE || part.category == CAT_BOOSTER;
}

// Helper: Check if a part is an aerodynamic surface
bool isAerodynamicSurface(const PartDef& part) {
    // Nose cones are aerodynamic surfaces
    if (part.category == CAT_NOSE_CONE) {
        return true;
    }
    // Fin sets (id=16 in structural category) are aerodynamic surfaces
    if (part.category == CAT_STRUCTURAL && part.id == 16) {
        return true;
    }
    return false;
}

// Helper: Get effective aerodynamic area for a part
float getAerodynamicArea(const PartDef& part) {
    if (part.category == CAT_NOSE_CONE) {
        // Cross-sectional area based on diameter
        float radius = part.diameter / 2.0f;
        return 3.14159f * radius * radius;
    }
    if (part.category == CAT_STRUCTURAL && part.id == 16) {
        // Fin set: use fixed area coefficient
        // Assuming 4 fins with reasonable surface area
        return 2.0f; // m^2 effective area
    }
    return 0.0f;
}

// Calculate hash of assembly configuration
size_t hashAssembly(const RocketAssembly& assembly) {
    size_t hash = 0;
    
    // Hash based on number of parts
    hash ^= assembly.parts.size() * 0x9e3779b9;
    
    // Hash each part's properties
    for (size_t i = 0; i < assembly.parts.size(); i++) {
        const PlacedPart& pp = assembly.parts[i];
        const PartDef& def = PART_CATALOG[pp.def_id];
        
        // Combine part ID, position, and stage
        size_t part_hash = pp.def_id;
        part_hash ^= (size_t)(pp.stack_y * 1000.0f) << 8;
        part_hash ^= (size_t)pp.stage << 16;
        
        // Mix in mass and dimensions
        part_hash ^= (size_t)(def.dry_mass * 10.0f);
        part_hash ^= (size_t)(def.fuel_capacity * 0.01f) << 4;
        part_hash ^= (size_t)(def.height * 100.0f) << 12;
        
        // Combine with running hash
        hash ^= part_hash * 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    
    return hash;
}

// Calculate center of mass
Vec3 calculateCenterOfMass(const RocketAssembly& assembly) {
    if (assembly.parts.empty()) {
        return Vec3(0, 0, 0);
    }
    
    float total_mass = 0.0f;
    float weighted_y = 0.0f;
    
    for (size_t i = 0; i < assembly.parts.size(); i++) {
        const PlacedPart& pp = assembly.parts[i];
        const PartDef& def = PART_CATALOG[pp.def_id];
        
        // Part mass = dry mass + fuel capacity
        float part_mass = def.dry_mass + def.fuel_capacity;
        
        // Skip parts with zero or negative mass
        if (part_mass <= 0.0f) {
            continue;
        }
        
        // Part center position = stack_y + height/2
        float part_center_y = pp.stack_y + def.height / 2.0f;
        
        total_mass += part_mass;
        weighted_y += part_mass * part_center_y;
    }
    
    if (total_mass <= 0.0f) {
        return Vec3(0, 0, 0);
    }
    
    // Center of mass is on the rocket's central axis (x=0, z=0)
    float com_y = weighted_y / total_mass;
    return Vec3(0, com_y, 0);
}

// Calculate center of lift
Vec3 calculateCenterOfLift(const RocketAssembly& assembly) {
    if (assembly.parts.empty()) {
        return Vec3(0, 0, 0);
    }
    
    float total_area = 0.0f;
    float weighted_y = 0.0f;
    
    for (size_t i = 0; i < assembly.parts.size(); i++) {
        const PlacedPart& pp = assembly.parts[i];
        const PartDef& def = PART_CATALOG[pp.def_id];
        
        if (!isAerodynamicSurface(def)) {
            continue;
        }
        
        float area = getAerodynamicArea(def);
        if (area <= 0.0f) {
            continue;
        }
        
        // Part center position = stack_y + height/2
        float part_center_y = pp.stack_y + def.height / 2.0f;
        
        total_area += area;
        weighted_y += area * part_center_y;
    }
    
    if (total_area <= 0.0f) {
        return Vec3(0, 0, 0);
    }
    
    // Center of lift is on the rocket's central axis (x=0, z=0)
    float col_y = weighted_y / total_area;
    return Vec3(0, col_y, 0);
}

// Calculate center of thrust
Vec3 calculateCenterOfThrust(const RocketAssembly& assembly) {
    if (assembly.parts.empty()) {
        return Vec3(0, 0, 0);
    }
    
    float total_thrust = 0.0f;
    float weighted_y = 0.0f;
    
    for (size_t i = 0; i < assembly.parts.size(); i++) {
        const PlacedPart& pp = assembly.parts[i];
        const PartDef& def = PART_CATALOG[pp.def_id];
        
        if (!isEngine(def)) {
            continue;
        }
        
        float thrust = def.thrust;
        if (thrust <= 0.0f) {
            continue;
        }
        
        // Part center position = stack_y + height/2
        float part_center_y = pp.stack_y + def.height / 2.0f;
        
        total_thrust += thrust;
        weighted_y += thrust * part_center_y;
    }
    
    if (total_thrust <= 0.0f) {
        return Vec3(0, 0, 0);
    }
    
    // Center of thrust is on the rocket's central axis (x=0, z=0)
    float cot_y = weighted_y / total_thrust;
    return Vec3(0, cot_y, 0);
}

} // namespace CenterCalculator
