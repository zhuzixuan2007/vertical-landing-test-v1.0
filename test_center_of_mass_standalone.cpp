// ==========================================================
// test_center_of_mass_standalone.cpp — Standalone Test for Center of Mass
// ==========================================================

#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>

// Minimal definitions needed for testing
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

enum PartCategory {
    CAT_NOSE_CONE = 0,
    CAT_FUEL_TANK = 1,
    CAT_ENGINE = 2,
    CAT_STRUCTURAL = 3,
    CAT_BOOSTER = 4
};

struct PartDef {
    int id;
    const char* name;
    PartCategory category;
    float height;
    float diameter;
    float dry_mass;
    float fuel_capacity;
    float thrust;
};

struct PlacedPart {
    int def_id;
    int stage;
    float stack_y;
};

struct RocketAssembly {
    std::vector<PlacedPart> parts;
    void recalculate() {}
};

// Minimal part catalog for testing
static PartDef TEST_PART_CATALOG[20] = {
    {0, "Standard Fairing", CAT_NOSE_CONE, 8.0f, 3.7f, 500.0f, 0.0f, 0.0f},
    {1, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {2, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {3, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {4, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {5, "Fuel Tank", CAT_FUEL_TANK, 25.0f, 3.7f, 4000.0f, 36000.0f, 0.0f},
    {6, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {7, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {8, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {9, "Mainsail", CAT_ENGINE, 2.5f, 3.7f, 6000.0f, 0.0f, 1500.0f},
    {10, "Poodle", CAT_ENGINE, 2.0f, 2.5f, 1750.0f, 0.0f, 250.0f},
    {11, "Thumper SRB", CAT_BOOSTER, 7.5f, 1.25f, 7650.0f, 8200.0f, 650.0f},
    {12, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {13, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {14, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {15, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {16, "Fin Set", CAT_STRUCTURAL, 2.0f, 0.0f, 50.0f, 0.0f, 0.0f},
    {17, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {18, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {19, "Empty", CAT_STRUCTURAL, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

#define PART_CATALOG TEST_PART_CATALOG

// Include the actual calculation logic
namespace CenterCalculator {
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
        
        float com_y = weighted_y / total_mass;
        return Vec3(0, com_y, 0);
    }
}

// Helper function to compare floats with tolerance
bool floatEquals(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

// Test 1: Empty rocket (no parts)
void test_empty_rocket() {
    std::cout << "Test 1: Empty rocket" << std::endl;
    RocketAssembly assembly;
    Vec3 com = CenterCalculator::calculateCenterOfMass(assembly);
    
    if (floatEquals(com.x, 0.0f) && floatEquals(com.y, 0.0f) && floatEquals(com.z, 0.0f)) {
        std::cout << "  PASS: CoM = (0, 0, 0)" << std::endl;
    } else {
        std::cout << "  FAIL: Expected (0, 0, 0), got (" << com.x << ", " << com.y << ", " << com.z << ")" << std::endl;
    }
    std::cout << std::endl;
}

// Test 2: Single part
void test_single_part() {
    std::cout << "Test 2: Single part (nose cone)" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart part;
    part.def_id = 0;  // Standard Fairing
    part.stage = 0;
    part.stack_y = 0.0f;
    assembly.parts.push_back(part);
    
    Vec3 com = CenterCalculator::calculateCenterOfMass(assembly);
    
    // Expected: CoM at center of part = stack_y + height/2 = 0 + 8/2 = 4.0m
    float expected_y = 4.0f;
    if (floatEquals(com.y, expected_y)) {
        std::cout << "  PASS: CoM.y = " << com.y << " (expected " << expected_y << ")" << std::endl;
    } else {
        std::cout << "  FAIL: Expected y=" << expected_y << ", got y=" << com.y << std::endl;
    }
    std::cout << std::endl;
}

// Test 3: Two identical parts
void test_two_identical_parts() {
    std::cout << "Test 3: Two identical parts" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart part1;
    part1.def_id = 0;
    part1.stage = 0;
    part1.stack_y = 0.0f;
    assembly.parts.push_back(part1);
    
    PlacedPart part2;
    part2.def_id = 0;
    part2.stage = 0;
    part2.stack_y = 10.0f;
    assembly.parts.push_back(part2);
    
    Vec3 com = CenterCalculator::calculateCenterOfMass(assembly);
    
    // Expected: CoM at midpoint = (4.0 + 14.0) / 2 = 9.0m
    float expected_y = 9.0f;
    
    std::cout << "  Expected CoM.y: " << expected_y << " m" << std::endl;
    std::cout << "  Calculated CoM.y: " << com.y << " m" << std::endl;
    
    if (floatEquals(com.y, expected_y)) {
        std::cout << "  PASS" << std::endl;
    } else {
        std::cout << "  FAIL: Difference = " << std::fabs(com.y - expected_y) << " m" << std::endl;
    }
    std::cout << std::endl;
}

// Test 4: Parts with different masses
void test_different_masses() {
    std::cout << "Test 4: Parts with different masses" << std::endl;
    RocketAssembly assembly;
    
    // Nose cone: mass = 500kg, center = 4.0m
    PlacedPart nosecone;
    nosecone.def_id = 0;
    nosecone.stage = 0;
    nosecone.stack_y = 0.0f;
    assembly.parts.push_back(nosecone);
    
    // Fuel tank: mass = 4000 + 36000 = 40000kg, center = 8 + 25/2 = 20.5m
    PlacedPart tank;
    tank.def_id = 5;
    tank.stage = 0;
    tank.stack_y = 8.0f;
    assembly.parts.push_back(tank);
    
    Vec3 com = CenterCalculator::calculateCenterOfMass(assembly);
    
    // Calculate expected CoM manually:
    const PartDef& nosecone_def = PART_CATALOG[0];
    const PartDef& tank_def = PART_CATALOG[5];
    
    float nosecone_mass = nosecone_def.dry_mass + nosecone_def.fuel_capacity;
    float nosecone_center = 0.0f + nosecone_def.height / 2.0f;
    float tank_mass = tank_def.dry_mass + tank_def.fuel_capacity;
    float tank_center = 8.0f + tank_def.height / 2.0f;
    float expected_y = (nosecone_mass * nosecone_center + tank_mass * tank_center) / (nosecone_mass + tank_mass);
    
    std::cout << "  Nose cone: mass=" << std::fixed << std::setprecision(0) << nosecone_mass 
              << " kg, center=" << std::setprecision(2) << nosecone_center << " m" << std::endl;
    std::cout << "  Fuel tank: mass=" << std::setprecision(0) << tank_mass 
              << " kg, center=" << std::setprecision(2) << tank_center << " m" << std::endl;
    std::cout << "  Expected CoM.y: " << expected_y << " m" << std::endl;
    std::cout << "  Calculated CoM.y: " << com.y << " m" << std::endl;
    
    if (floatEquals(com.y, expected_y, 0.01f)) {
        std::cout << "  PASS" << std::endl;
    } else {
        std::cout << "  FAIL: Difference = " << std::fabs(com.y - expected_y) << " m" << std::endl;
    }
    std::cout << std::endl;
}

// Test 5: Complete rocket
void test_complete_rocket() {
    std::cout << "Test 5: Complete rocket (nose + tank + engine)" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart nosecone;
    nosecone.def_id = 0;
    nosecone.stage = 0;
    nosecone.stack_y = 0.0f;
    assembly.parts.push_back(nosecone);
    
    PlacedPart tank;
    tank.def_id = 5;
    tank.stage = 0;
    tank.stack_y = 8.0f;
    assembly.parts.push_back(tank);
    
    PlacedPart engine;
    engine.def_id = 9;
    engine.stage = 0;
    engine.stack_y = 33.0f;
    assembly.parts.push_back(engine);
    
    Vec3 com = CenterCalculator::calculateCenterOfMass(assembly);
    
    // Calculate expected CoM manually
    const PartDef& nosecone_def = PART_CATALOG[0];
    const PartDef& tank_def = PART_CATALOG[5];
    const PartDef& engine_def = PART_CATALOG[9];
    
    float nosecone_mass = nosecone_def.dry_mass + nosecone_def.fuel_capacity;
    float nosecone_center = 0.0f + nosecone_def.height / 2.0f;
    float tank_mass = tank_def.dry_mass + tank_def.fuel_capacity;
    float tank_center = 8.0f + tank_def.height / 2.0f;
    float engine_mass = engine_def.dry_mass + engine_def.fuel_capacity;
    float engine_center = 33.0f + engine_def.height / 2.0f;
    
    float total_mass = nosecone_mass + tank_mass + engine_mass;
    float expected_y = (nosecone_mass * nosecone_center + tank_mass * tank_center + engine_mass * engine_center) / total_mass;
    
    std::cout << "  Nose cone: mass=" << std::fixed << std::setprecision(0) << nosecone_mass 
              << " kg, center=" << std::setprecision(2) << nosecone_center << " m" << std::endl;
    std::cout << "  Fuel tank: mass=" << std::setprecision(0) << tank_mass 
              << " kg, center=" << std::setprecision(2) << tank_center << " m" << std::endl;
    std::cout << "  Engine: mass=" << std::setprecision(0) << engine_mass 
              << " kg, center=" << std::setprecision(2) << engine_center << " m" << std::endl;
    std::cout << "  Expected CoM.y: " << expected_y << " m" << std::endl;
    std::cout << "  Calculated CoM.y: " << com.y << " m" << std::endl;
    
    if (floatEquals(com.y, expected_y, 0.01f)) {
        std::cout << "  PASS" << std::endl;
    } else {
        std::cout << "  FAIL: Difference = " << std::fabs(com.y - expected_y) << " m" << std::endl;
    }
    std::cout << std::endl;
}

// Test 6: Zero mass parts
void test_zero_mass_parts() {
    std::cout << "Test 6: Zero mass parts (should be ignored)" << std::endl;
    RocketAssembly assembly;
    
    // Add a valid part
    PlacedPart nosecone;
    nosecone.def_id = 0;
    nosecone.stage = 0;
    nosecone.stack_y = 0.0f;
    assembly.parts.push_back(nosecone);
    
    // Add zero mass parts (should be ignored)
    PlacedPart empty1;
    empty1.def_id = 1;
    empty1.stage = 0;
    empty1.stack_y = 10.0f;
    assembly.parts.push_back(empty1);
    
    PlacedPart empty2;
    empty2.def_id = 2;
    empty2.stage = 0;
    empty2.stack_y = 20.0f;
    assembly.parts.push_back(empty2);
    
    Vec3 com = CenterCalculator::calculateCenterOfMass(assembly);
    
    // Expected: CoM should be at nose cone center only (4.0m)
    float expected_y = 4.0f;
    if (floatEquals(com.y, expected_y)) {
        std::cout << "  PASS: CoM.y = " << com.y << " (zero mass parts ignored)" << std::endl;
    } else {
        std::cout << "  FAIL: Expected y=" << expected_y << ", got y=" << com.y << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Center of Mass Calculation Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    test_empty_rocket();
    test_single_part();
    test_two_identical_parts();
    test_different_masses();
    test_complete_rocket();
    test_zero_mass_parts();
    
    std::cout << "========================================" << std::endl;
    std::cout << "All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
