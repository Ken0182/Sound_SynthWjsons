#include "normalization.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <fstream>

namespace aiaudio {

// PresetNormalizer implementation
PresetNormalizer::NormalizedPreset PresetNormalizer::normalize(const DSPGraph& graph, Role role) const {
    NormalizedPreset preset;
    preset.version = "1.0";
    preset.timestamp = std::chrono::system_clock::now();
    
    // Extract parameters from graph
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            auto paramNames = stage->getParameterNames();
            for (const auto& paramName : paramNames) {
                std::string fullParamName = stageName + "." + paramName;
                auto paramValue = stage->getParameter(paramName);
                
                if (std::holds_alternative<double>(paramValue)) {
                    double value = std::get<double>(paramValue);
                    
                    // Apply role-specific normalization
                    if (paramName.find("frequency") != std::string::npos || 
                        paramName.find("cutoff") != std::string::npos) {
                        value = mapFrequency(value, role);
                    } else if (paramName.find("amplitude") != std::string::npos ||
                               paramName.find("gain") != std::string::npos) {
                        value = mapAmplitude(value, role);
                    } else if (paramName.find("attack") != std::string::npos ||
                               paramName.find("decay") != std::string::npos ||
                               paramName.find("release") != std::string::npos) {
                        value = mapTime(value, role);
                    } else if (paramName.find("resonance") != std::string::npos ||
                               paramName.find("ratio") != std::string::npos) {
                        value = mapRatio(value, role);
                    }
                    
                    preset.parameters[fullParamName] = value;
                }
            }
        }
    }
    
    // Apply perceptual mappings
    applyPerceptualMappings(preset, role);
    
    // Ensure stability
    ensureStability(preset);
    
    return preset;
}

std::vector<std::string> PresetNormalizer::validate(const NormalizedPreset& preset) const {
    std::vector<std::string> issues;
    
    // Validate individual parameters
    for (const auto& [name, value] : preset.parameters) {
        if (name.find("frequency") != std::string::npos || 
            name.find("cutoff") != std::string::npos) {
            if (!isValidFrequency(value)) {
                issues.push_back("Invalid frequency parameter " + name + ": " + std::to_string(value));
            }
        } else if (name.find("amplitude") != std::string::npos ||
                   name.find("gain") != std::string::npos) {
            if (!isValidAmplitude(value)) {
                issues.push_back("Invalid amplitude parameter " + name + ": " + std::to_string(value));
            }
        } else if (name.find("attack") != std::string::npos ||
                   name.find("decay") != std::string::npos ||
                   name.find("release") != std::string::npos) {
            if (!isValidTime(value)) {
                issues.push_back("Invalid time parameter " + name + ": " + std::to_string(value));
            }
        } else if (name.find("resonance") != std::string::npos ||
                   name.find("ratio") != std::string::npos) {
            if (!isValidRatio(value)) {
                issues.push_back("Invalid ratio parameter " + name + ": " + std::to_string(value));
            }
        }
    }
    
    // Check stability
    if (!checkFeedbackStability(preset)) {
        issues.push_back("Potential feedback instability detected");
    }
    
    if (!checkGainStability(preset)) {
        issues.push_back("Gain stability issues detected");
    }
    
    if (!checkPhaseStability(preset)) {
        issues.push_back("Phase stability issues detected");
    }
    
    return issues;
}

void PresetNormalizer::loadRoleRules(Role role, const std::map<std::string, double>& rules) {
    roleRules_[role] = rules;
}

void PresetNormalizer::applyPerceptualMappings(NormalizedPreset& preset, Role role) const {
    // Apply role-specific perceptual mappings
    auto it = roleRules_.find(role);
    if (it != roleRules_.end()) {
        const auto& rules = it->second;
        
        for (auto& [name, value] : preset.parameters) {
            if (rules.find(name) != rules.end()) {
                // Apply perceptual scaling
                double scale = rules.at(name);
                value = value * scale;
            }
        }
    }
}

void PresetNormalizer::ensureStability(NormalizedPreset& preset) const {
    // Clamp all parameters to safe ranges
    for (auto& [name, value] : preset.parameters) {
        if (name.find("frequency") != std::string::npos || 
            name.find("cutoff") != std::string::npos) {
            value = std::clamp(value, 20.0, 20000.0);
        } else if (name.find("amplitude") != std::string::npos ||
                   name.find("gain") != std::string::npos) {
            value = std::clamp(value, 0.0, 1.0);
        } else if (name.find("resonance") != std::string::npos) {
            value = std::clamp(value, 0.0, 0.99);
        } else if (name.find("attack") != std::string::npos ||
                   name.find("decay") != std::string::npos ||
                   name.find("release") != std::string::npos) {
            value = std::clamp(value, 0.001, 10.0);
        }
    }
}

double PresetNormalizer::mapFrequency(double freq, Role role) const {
    // Role-specific frequency mapping
    switch (role) {
        case Role::BASS:
            return std::clamp(freq, 20.0, 500.0);
        case Role::PAD:
            return std::clamp(freq, 100.0, 2000.0);
        case Role::LEAD:
            return std::clamp(freq, 200.0, 4000.0);
        case Role::PERCUSSION:
            return std::clamp(freq, 50.0, 8000.0);
        default:
            return std::clamp(freq, 20.0, 20000.0);
    }
}

double PresetNormalizer::mapAmplitude(double amp, Role role) const {
    // Role-specific amplitude mapping
    switch (role) {
        case Role::BASS:
            return std::clamp(amp, 0.3, 0.8);
        case Role::PAD:
            return std::clamp(amp, 0.2, 0.6);
        case Role::LEAD:
            return std::clamp(amp, 0.4, 0.9);
        case Role::PERCUSSION:
            return std::clamp(amp, 0.5, 1.0);
        default:
            return std::clamp(amp, 0.0, 1.0);
    }
}

double PresetNormalizer::mapTime(double time, Role role) const {
    // Role-specific time mapping
    switch (role) {
        case Role::BASS:
            return std::clamp(time, 0.001, 0.5);
        case Role::PAD:
            return std::clamp(time, 0.1, 3.0);
        case Role::LEAD:
            return std::clamp(time, 0.01, 1.0);
        case Role::PERCUSSION:
            return std::clamp(time, 0.001, 0.2);
        default:
            return std::clamp(time, 0.001, 10.0);
    }
}

double PresetNormalizer::mapRatio(double ratio, Role role) const {
    // Role-specific ratio mapping
    switch (role) {
        case Role::BASS:
            return std::clamp(ratio, 0.0, 0.7);
        case Role::PAD:
            return std::clamp(ratio, 0.0, 0.5);
        case Role::LEAD:
            return std::clamp(ratio, 0.0, 0.8);
        case Role::PERCUSSION:
            return std::clamp(ratio, 0.0, 0.9);
        default:
            return std::clamp(ratio, 0.0, 0.99);
    }
}

bool PresetNormalizer::isValidFrequency(double freq) const {
    return freq >= 20.0 && freq <= 20000.0 && std::isfinite(freq);
}

bool PresetNormalizer::isValidAmplitude(double amp) const {
    return amp >= 0.0 && amp <= 1.0 && std::isfinite(amp);
}

bool PresetNormalizer::isValidTime(double time) const {
    return time >= 0.001 && time <= 10.0 && std::isfinite(time);
}

bool PresetNormalizer::isValidRatio(double ratio) const {
    return ratio >= 0.0 && ratio <= 0.99 && std::isfinite(ratio);
}

bool PresetNormalizer::checkFeedbackStability(const NormalizedPreset& preset) const {
    // Check for potential feedback loops
    double totalGain = 1.0;
    
    for (const auto& [name, value] : preset.parameters) {
        if (name.find("amplitude") != std::string::npos ||
            name.find("gain") != std::string::npos) {
            totalGain *= value;
        }
    }
    
    return totalGain < 1.0;
}

bool PresetNormalizer::checkGainStability(const NormalizedPreset& preset) const {
    // Check for excessive gain values
    for (const auto& [name, value] : preset.parameters) {
        if (name.find("amplitude") != std::string::npos ||
            name.find("gain") != std::string::npos) {
            if (value > 1.0) {
                return false;
            }
        }
    }
    
    return true;
}

bool PresetNormalizer::checkPhaseStability(const NormalizedPreset& preset) const {
    // Check for phase-related stability issues
    // This is a simplified check - in practice would be more complex
    
    for (const auto& [name, value] : preset.parameters) {
        if (name.find("phase") != std::string::npos) {
            if (value < 0.0 || value > 1.0) {
                return false;
            }
        }
    }
    
    return true;
}

// SnapshotTester implementation
SnapshotTester::Snapshot SnapshotTester::createSnapshot(const NormalizedPreset& preset) const {
    Snapshot snapshot;
    snapshot.parameters = preset.parameters;
    snapshot.timestamp = std::chrono::system_clock::now();
    snapshot.hash = generateHash(preset.parameters);
    return snapshot;
}

bool SnapshotTester::compareSnapshots(const Snapshot& a, const Snapshot& b, double tolerance) const {
    if (a.parameters.size() != b.parameters.size()) {
        return false;
    }
    
    for (const auto& [name, valueA] : a.parameters) {
        auto it = b.parameters.find(name);
        if (it == b.parameters.end()) {
            return false;
        }
        
        double valueB = it->second;
        if (std::abs(valueA - valueB) > tolerance) {
            return false;
        }
    }
    
    return true;
}

void SnapshotTester::loadReferenceSnapshots(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw AIAudioException("Could not open reference snapshots: " + path);
    }
    
    // Load snapshots from file (simplified JSON format)
    // Implementation would parse JSON and populate referenceSnapshots_
}

void SnapshotTester::saveSnapshots(const std::vector<Snapshot>& snapshots, const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw AIAudioException("Could not save snapshots: " + path);
    }
    
    // Save snapshots to file (simplified JSON format)
    // Implementation would serialize to JSON
}

std::string SnapshotTester::generateHash(const std::map<std::string, double>& parameters) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6);
    
    for (const auto& [name, value] : parameters) {
        ss << name << ":" << value << ";";
    }
    
    // Simple hash (in practice would use proper hash function)
    std::hash<std::string> hasher;
    return std::to_string(hasher(ss.str()));
}

} // namespace aiaudio