#pragma once
// ==========================================================
// stage_manager.h — Multi-Stage Rocket Manager
// Handles stage building, separation events, and active
// stage synchronization.
// ==========================================================

#include "core/rocket_state.h"
#include <vector>

struct RocketAssembly; // Forward declaration

namespace StageManager {

// -------------------------------------------------------
// BuildStages — Scan the assembly for Decouplers and
// partition parts into stages.
// -------------------------------------------------------
void BuildStages(const RocketAssembly& assembly, RocketConfig& config);

// -------------------------------------------------------
// SyncActiveConfig — Copy active stage's parameters into
// the top-level RocketConfig shorthand fields.
// -------------------------------------------------------
void SyncActiveConfig(RocketConfig& config, int stage_index);

// -------------------------------------------------------
// SeparateStage — Jettison the current (bottom) stage
// and advance to the next one.
// -------------------------------------------------------
bool SeparateStage(RocketState& state, RocketConfig& config);

// -------------------------------------------------------
// IsCurrentStageEmpty — Check if the current stage's
// fuel is depleted.
// -------------------------------------------------------
bool IsCurrentStageEmpty(const RocketState& state);

// -------------------------------------------------------
// GetActiveStage — Get the StageConfig for the given
// stage index.
// -------------------------------------------------------
const StageConfig& GetActiveStage(const RocketConfig& config, int stage_index);

// -------------------------------------------------------
// GetTotalMassFromStage — Calculate total mass from a
// given stage upward (including fuel).
// -------------------------------------------------------
double GetTotalMassFromStage(const RocketConfig& config, const RocketState& state, int from_stage);

} // namespace StageManager
