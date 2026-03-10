// ==========================================================
// test_center_of_thrust.cpp — Test for Center of Thrust Calculation
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
    {0, "Nose Cone", CAT_NOSE_CONE, 8.0f, 3.7f, 500.0f, 0.0f, 0.0f},
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
    bool isEngine(const PartDef& part) {
        return part.category == CAT_ENGINE || part.category == CAT_BOOSTER;
    }

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
            
            float part_center_y = pp.stack_y + def.height / 2.0f;
            
            total_thrust += thrust;
            weighted_y += thrust * part_center_y;
        }
        
        if (total_thrust <= 0.0f) {
            return Vec3(0, 0, 0);
        }
        
        float cot_y = weighted_y / total_thrust;
        return Vec3(0, cot_y, 0);
    }
}

// Helper function to compare floats with tolerance
bool floatEquals(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

// Test 1: Empty rocket (no parts)
void test_empty_rocket() {
    std::cout << "Test 1: Empty rocket (no engines)" << std::endl;
    RocketAssembly assembly;
    Vec3 cot = CenterCalculator::calculateCenterOfThrust(assembly);
    
    if (floatEquals(cot.x, 0.0f) && floatEquals(cot.y, 0.0f) && floatEquals(cot.z, 0.0f)) {
        std::cout << "  PASS: CoT = (0, 0, 0)" << std::endl;
    } else {
        std::cout << "  FAIL: Expected (0, 0, 0), got (" << cot.x << ", " << cot.y << ", " << cot.z << ")" << std::endl;
    }
    std::cout << std::endl;
}

// Test 2: Single engine
void test_single_engine() {
    std::cout << "Test 2: Single engine" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart engine;
    engine.def_id = 9;  // Mainsail
    engine.stage = 0;
    engine.stack_y = 10.0f;
    assembly.parts.push_back(engine);
    
    Vec3 cot = CenterCalculator::calculateCenterOfThrust(assembly);
    
    // Expected: CoT at center of engine = stack_y + height/2 = 10 + 2.5/2 = 11.25m
    float expected_y = 11.25f;
    if (floatEquals(cot.y, expected_y)) {
        std::cout << "  PASS: CoT.y = " << cot.y << " (expected " << expected_y << ")" << std::endl;
    } else {
        std::cout << "  FAIL: Expected y=" << expected_y << ", got y=" << cot.y << std::endl;
    }
    std::cout << std::endl;
}

// Test 3: Two identical engines
void test_two_identical_engines() {
    std::cout << "Test 3: Two identical engines" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart engine1;
    engine1.def_id = 9;
    engine1.stage = 0;
    engine1.stack_y = 0.0f;
    assembly.parts.push_back(engine1);
    
    PlacedPart engine2;
    engine2.def_id = 9;
    engine2.stage = 0;
    engine2.stack_y = 10.0f;
    assembly.parts.push_back(engine2);
    
    Vec3 cot = CenterCalculator::calculateCenterOfThrust(assembly);
    
    // Expected: CoT at midpoint = 6.25m
    float expected_y = 6.25f;
    
    std::cout << "  Expected CoT.y: " << expected_y << " m" << std::endl;
    std::cout << "  Calculated CoT.y: " << cot.y << " m" << std::endl;
    
    if (floatEquals(cot.y, expected_y)) {
        std::cout << "  PASS" << std::endl;
    } else {
        std::cout << "  FAIL: Difference = " << std::fabs(cot.y - expected_y) << " m" << std::endl;
    }
    std::cout << std::endl;
}

// Test 4: Two engines with different thrust
void test_different_thrust_engines() {
    std::cout << "Test 4: Two engines with different thrust" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart mainsail;
    mainsail.def_id = 9;
    mainsail.stage = 0;
    mainsail.stack_y = 0.0f;
    assembly.parts.push_back(mainsail);
    
    PlacedPart poodle;
    poodle.def_id = 10;
    poodle.stage = 0;
    poodle.stack_y = 10.0f;
    assembly.parts.push_back(poodle);
    
    Vec3 cot = CenterCalculator::calculateCenterOfThrust(assembly);
    
    // Calculate expected CoT manually:
    // Mainsail: thrust = 1500kN, center_y = 0 + 2.5/2 = 1.25m
    // Poodle: thrust = 250kN, center_y = 10 + 2.0/2 = 11.0m
    // CoT = (1500 * 1.25 + 250 * 11.0) / (1500 + 250) = 2.64m
    
    const PartDef& mainsail_def = PART_CATALOG[9];
    const PartDef& poodle_def = PART_CATALOG[10];
    
    float mainsail_thrust = mainsail_def.thrust;
    float mainsail_center = 0.0f + mainsail_def.height / 2.0f;
    float poodle_thrust = poodle_def.thrust;
    float poodle_center = 10.0f + poodle_def.height / 2.0f;
    float expected_y = (mainsail_thrust * mainsail_center + poodle_thrust * poodle_center) / (mainsail_thrust + poodle_thrust);
    
    std::cout << "  Mainsail: thrust=" << std::fixed << std::setprecision(0) << mainsail_thrust 
              << " kN, center=" << std::setprecision(2) << mainsail_center << " m" << std::endl;
    std::cout << "  Poodle: thrust=" << std::setprecision(0) << poodle_thrust 
              << " kN, center=" << std::setprecision(2) << poodle_center << " m" << std::endl;
    std::cout << "  Expected CoT.y: " << expected_y << " m" << std::endl;
    std::cout << "  Calculated CoT.y: " << cot.y << " m" << std::endl;
    
    if (floatEquals(cot.y, expected_y, 0.01f)) {
        std::cout << "  PASS" << std::endl;
    } else {
        std::cout << "  FAIL: Difference = " << std::fabs(cot.y - expected_y) << " m" << std::endl;
    }
    std::cout << std::endl;
}

// Test 5: Booster
void test_booster() {
    std::cout << "Test 5: Solid rocket booster" << std::endl;
    RocketAssembly assembly;
    
    PlacedPart booster;
    booster.def_id = 11;  // Thumper SRB
    booster.stage = 0;
    booster.stack_y = 5.0f;
    assembly.parts.push_back(booster);
    
    Vec3 cot = CenterCalculator::calculateCenterOfThrust(assembly);
    
    // Expected: CoT at center of booster = 5 + 7.5/2 = 8.75m
    float expected_y = 8.75f;
    if (floatEquals(cot.y, expected_y)) {
        std::cout << "  PASS: CoT.y = " << cot.y << " (expected " << expected_y << ")" << std::endl;
    } else {
        std::cout << "  FAIL: Expected y=" << expected_y << ", got y=" << cot.y << std::endl;
    }
    std::cout << std::endl;
}

// Test 6: No engines
void test_no_engines() {
    std::cout << "Test 6: No engines (only structural parts)" << std::endl;
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
    
    Vec3 cot = CenterCalculator::calculateCenterOfThrust(assembly);
    
    if (floatEquals(cot.x, 0.0f) && floatEquals(cot.y, 0.0f) && floatEquals(cot.z, 0.0f)) {
        std::cout << "  PASS: CoT = (0, 0, 0) for non-engine parts" << std::endl;
    } else {
        std::cout << "  FAIL: Expected (0, 0, 0), got (" << cot.x << ", " << cot.y << ", " << cot.z << ")" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Center of Thrust Calculation Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    test_empty_rocket();
    test_single_engine();
    test_two_identical_engines();
    test_different_thrust_engines();
    test_booster();
    test_no_engines();
    
    std::cout << "========================================" << std::endl;
    std::cout << "All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
