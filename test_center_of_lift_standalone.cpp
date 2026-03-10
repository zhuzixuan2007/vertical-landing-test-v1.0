// ==========================================================
// test_center_of_lift_standalone.cpp — Standalone Test for Center of Lift
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
    bool isAerodynamicSurface(const PartDef& part) {
        if (part.category == CAT_NOSE_CONE) {
            return true;
        }
        if (part.category == CAT_STRUCTURAL && part.id == 16) {
            return true;
        }
        return false;
    }

    float getAerodynamicArea(const PartDef& part) {
        if (part.category == CAT_NOSE_CONE) {
            float radius = part.diameter / 2.0f;
            return 3.14159f * radius * radius;
        }
        if (part.category == CAT_STRUCTURAL && part.id == 16) {
            return 2.0f; // m^2 effective area for fin set
        }
        return 0.0f;
    }

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
            
            float part_center_y = pp.stack_y + def.height / 2.0f;
            
            total_area += area;
            weighted_y += area * part_center_y;
        }
        
        if (total_area <= 0.0f) {
            return Vec3(0, 0, 0);
        }
        
        float col_y = weighted_y / total_area;
        return Vec3(0, col_y, 0);
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
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    if (floatEquals(col.x, 0.0f) && floatEquals(col.y, 0.0f) && floatEquals(col.z, 0.0f)) {
        std::cout << "  PASS: CoL = (0, 0, 0)" << std::endl;
    } else {
        std::cout << "  FAIL: Expected (0, 0, 0), got (" << col.x << ", " << col.y << ", " << col.z << ")" << std::endl;
    }
    std::cout << std::endl;
}

// Test 2: Single nose cone
void test_single_nosecone() {
    std::cout << "Test 2: Single nose cone" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart part;
    part.def_id = 0;  // Standard Fairing
    part.stage = 0;
    part.stack_y = 0.0f;
    assembly.parts.push_back(part);
    
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    // Expected: CoL at center of nose cone = stack_y + height/2 = 0 + 8/2 = 4.0m
    float expected_y = 4.0f;
    if (floatEquals(col.y, expected_y)) {
        std::cout << "  PASS: CoL.y = " << col.y << " (expected " << expected_y << ")" << std::endl;
    } else {
        std::cout << "  FAIL: Expected y=" << expected_y << ", got y=" << col.y << std::endl;
    }
    std::cout << std::endl;
}

// Test 3: Single fin set
void test_single_finset() {
    std::cout << "Test 3: Single fin set" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart part;
    part.def_id = 16;  // Fin Set
    part.stage = 0;
    part.stack_y = 10.0f;
    assembly.parts.push_back(part);
    
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    // Expected: CoL at center of fin set = stack_y + height/2 = 10 + 2/2 = 11.0m
    float expected_y = 11.0f;
    if (floatEquals(col.y, expected_y)) {
        std::cout << "  PASS: CoL.y = " << col.y << " (expected " << expected_y << ")" << std::endl;
    } else {
        std::cout << "  FAIL: Expected y=" << expected_y << ", got y=" << col.y << std::endl;
    }
    std::cout << std::endl;
}

// Test 4: Nose cone + fin set (weighted average)
void test_nosecone_and_finset() {
    std::cout << "Test 4: Nose cone + fin set (weighted average)" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart nosecone;
    nosecone.def_id = 0;
    nosecone.stage = 0;
    nosecone.stack_y = 0.0f;
    assembly.parts.push_back(nosecone);
    
    PlacedPart finset;
    finset.def_id = 16;
    finset.stage = 0;
    finset.stack_y = 20.0f;
    assembly.parts.push_back(finset);
    
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    // Calculate expected CoL manually:
    // Nose cone: area = π * (3.7/2)² ≈ 10.75 m², center_y = 0 + 8/2 = 4.0m
    // Fin set: area = 2.0 m², center_y = 20 + 2/2 = 21.0m
    // CoL = (10.75 * 4.0 + 2.0 * 21.0) / (10.75 + 2.0)
    
    const PartDef& nosecone_def = PART_CATALOG[0];
    const PartDef& finset_def = PART_CATALOG[16];
    
    float nosecone_area = 3.14159f * (nosecone_def.diameter / 2.0f) * (nosecone_def.diameter / 2.0f);
    float nosecone_center = 0.0f + nosecone_def.height / 2.0f;
    float finset_area = 2.0f;
    float finset_center = 20.0f + finset_def.height / 2.0f;
    float expected_y = (nosecone_area * nosecone_center + finset_area * finset_center) / (nosecone_area + finset_area);
    
    std::cout << "  Nose cone: area=" << std::fixed << std::setprecision(2) << nosecone_area 
              << " m², center=" << nosecone_center << " m" << std::endl;
    std::cout << "  Fin set: area=" << finset_area << " m², center=" << finset_center << " m" << std::endl;
    std::cout << "  Expected CoL.y: " << expected_y << " m" << std::endl;
    std::cout << "  Calculated CoL.y: " << col.y << " m" << std::endl;
    
    if (floatEquals(col.y, expected_y, 0.01f)) {
        std::cout << "  PASS" << std::endl;
    } else {
        std::cout << "  FAIL: Difference = " << std::fabs(col.y - expected_y) << " m" << std::endl;
    }
    std::cout << std::endl;
}

// Test 5: No aerodynamic surfaces
void test_no_aero_surfaces() {
    std::cout << "Test 5: No aerodynamic surfaces" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart tank;
    tank.def_id = 5;
    tank.stage = 0;
    tank.stack_y = 0.0f;
    assembly.parts.push_back(tank);
    
    PlacedPart engine;
    engine.def_id = 9;
    engine.stage = 0;
    engine.stack_y = 25.0f;
    assembly.parts.push_back(engine);
    
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    if (floatEquals(col.x, 0.0f) && floatEquals(col.y, 0.0f) && floatEquals(col.z, 0.0f)) {
        std::cout << "  PASS: CoL = (0, 0, 0) for non-aerodynamic parts" << std::endl;
    } else {
        std::cout << "  FAIL: Expected (0, 0, 0), got (" << col.x << ", " << col.y << ", " << col.z << ")" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Center of Lift Calculation Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    test_empty_rocket();
    test_single_nosecone();
    test_single_finset();
    test_nosecone_and_finset();
    test_no_aero_surfaces();
    
    std::cout << "========================================" << std::endl;
    std::cout << "All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
