// ==========================================================
// test_center_of_lift.cpp — Test for Center of Lift Calculation
// ==========================================================

#include "src/simulation/center_calculator.h"
#include "src/simulation/rocket_builder.h"
#include <iostream>
#include <cmath>
#include <iomanip>

// Helper function to compare floats with tolerance
bool floatEquals(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

// Test 1: Empty rocket (no parts)
void test_empty_rocket() {
    std::cout << "Test 1: Empty rocket" << std::endl;
    RocketAssembly assembly;
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    // Expected: (0, 0, 0) for empty rocket
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
    
    // Add a standard fairing (id=0, height=8m)
    PlacedPart part;
    part.def_id = 0;  // Standard Fairing
    part.stage = 0;
    part.stack_y = 0.0f;
    assembly.parts.push_back(part);
    assembly.recalculate();
    
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
    
    // Add a fin set (id=16, height=2m)
    PlacedPart part;
    part.def_id = 16;  // Fin Set
    part.stage = 0;
    part.stack_y = 10.0f;
    assembly.parts.push_back(part);
    assembly.recalculate();
    
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
    
    // Add standard fairing (id=0, height=8m, diameter=3.7m)
    PlacedPart nosecone;
    nosecone.def_id = 0;
    nosecone.stage = 0;
    nosecone.stack_y = 0.0f;
    assembly.parts.push_back(nosecone);
    
    // Add fin set (id=16, height=2m, fixed area=2.0m²)
    PlacedPart finset;
    finset.def_id = 16;
    finset.stage = 0;
    finset.stack_y = 20.0f;
    assembly.parts.push_back(finset);
    
    assembly.recalculate();
    
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    // Calculate expected CoL manually:
    // Nose cone: area = π * (3.7/2)² ≈ 10.75 m², center_y = 0 + 8/2 = 4.0m
    // Fin set: area = 2.0 m², center_y = 20 + 2/2 = 21.0m
    // CoL = (10.75 * 4.0 + 2.0 * 21.0) / (10.75 + 2.0)
    //     = (43.0 + 42.0) / 12.75
    //     = 85.0 / 12.75 ≈ 6.67m
    
    float nosecone_area = 3.14159f * (3.7f / 2.0f) * (3.7f / 2.0f);
    float nosecone_center = 4.0f;
    float finset_area = 2.0f;
    float finset_center = 21.0f;
    float expected_y = (nosecone_area * nosecone_center + finset_area * finset_center) / (nosecone_area + finset_area);
    
    std::cout << "  Nose cone area: " << nosecone_area << " m², center: " << nosecone_center << " m" << std::endl;
    std::cout << "  Fin set area: " << finset_area << " m², center: " << finset_center << " m" << std::endl;
    std::cout << "  Expected CoL.y: " << expected_y << " m" << std::endl;
    std::cout << "  Calculated CoL.y: " << col.y << " m" << std::endl;
    
    if (floatEquals(col.y, expected_y, 0.01f)) {
        std::cout << "  PASS" << std::endl;
    } else {
        std::cout << "  FAIL: Difference = " << std::fabs(col.y - expected_y) << " m" << std::endl;
    }
    std::cout << std::endl;
}

// Test 5: No aerodynamic surfaces (only fuel tank and engine)
void test_no_aero_surfaces() {
    std::cout << "Test 5: No aerodynamic surfaces" << std::endl;
    RocketAssembly assembly;
    
    // Add fuel tank (id=5, not aerodynamic)
    PlacedPart tank;
    tank.def_id = 5;
    tank.stage = 0;
    tank.stack_y = 0.0f;
    assembly.parts.push_back(tank);
    
    // Add engine (id=9, not aerodynamic)
    PlacedPart engine;
    engine.def_id = 9;
    engine.stage = 0;
    engine.stack_y = 25.0f;
    assembly.parts.push_back(engine);
    
    assembly.recalculate();
    
    Vec3 col = CenterCalculator::calculateCenterOfLift(assembly);
    
    // Expected: (0, 0, 0) since no aerodynamic surfaces
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
