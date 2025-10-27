#include "roles_policies.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

namespace aiaudio {

// PolicyCompiler implementation
RolePolicy PolicyCompiler::loadPolicy(const std::string& yamlContent, Role role) {
#ifdef YAMLCPP_FOUND
    YAML::Node root = YAML::Load(yamlContent);
    
    RolePolicy policy;
    policy.role = role;
    policy.version = root["version"].as<std::string>("1.0");
    policy.description = root["description"].as<std::string>("");
    policy.precedence = root["precedence"].as<int>(0);
    
    // Parse constraints
    if (root["constraints"]) {
        for (const auto& constraintNode : root["constraints"]) {
            std::string paramName = constraintNode.first.as<std::string>();
            PolicyConstraint constraint = parseConstraint(constraintNode.second);
            constraint.parameter = paramName;
            policy.constraints[paramName] = constraint;
        }
    }
    
    // Parse priors
    if (root["priors"]) {
        policy.priors = parsePriors(root["priors"]);
    }
    
    // Parse penalties
    if (root["penalties"]) {
        policy.penalties = parsePenalties(root["penalties"]);
    }
    
    return policy;
#else
    throw AIAudioException("YAML parsing not available - yaml-cpp not found");
#endif
}

RolePolicy PolicyCompiler::loadPolicyFromFile(const std::string& filePath, Role role) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw AIAudioException("Could not open policy file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return loadPolicy(buffer.str(), role);
}

std::vector<std::string> PolicyCompiler::validatePolicy(const RolePolicy& policy) const {
    std::vector<std::string> issues;
    
    // Validate constraints
    for (const auto& [paramName, constraint] : policy.constraints) {
        if (constraint.parameter != paramName) {
            issues.push_back("Constraint parameter mismatch: " + paramName);
        }
        
        if (constraint.type == ConstraintType::RANGE && constraint.range.size() != 2) {
            issues.push_back("Range constraint must have exactly 2 values: " + paramName);
        }
        
        if (constraint.type == ConstraintType::ENUM && constraint.options.empty()) {
            issues.push_back("Enum constraint must have options: " + paramName);
        }
        
        if (constraint.type == ConstraintType::CUSTOM && !constraint.customValidator) {
            issues.push_back("Custom constraint must have validator: " + paramName);
        }
    }
    
    // Validate priors
    for (const auto& [paramName, prior] : policy.priors) {
        if (prior < 0.0 || prior > 1.0) {
            issues.push_back("Prior must be in [0,1]: " + paramName);
        }
    }
    
    // Validate penalties
    for (const auto& [paramName, penalty] : policy.penalties) {
        if (penalty < 0.0) {
            issues.push_back("Penalty must be non-negative: " + paramName);
        }
    }
    
    return issues;
}

std::map<std::string, std::pair<double, double>> PolicyCompiler::compileConstraints(
    const RolePolicy& policy) const {
    
    std::map<std::string, std::pair<double, double>> ranges;
    
    for (const auto& [paramName, constraint] : policy.constraints) {
        if (constraint.type == ConstraintType::RANGE && constraint.range.size() >= 2) {
            ranges[paramName] = {constraint.range[0], constraint.range[1]};
        }
    }
    
    return ranges;
}

RolePolicy PolicyCompiler::resolveConflicts(const std::vector<RolePolicy>& policies) const {
    if (policies.empty()) {
        throw AIAudioException("Cannot resolve conflicts: no policies provided");
    }
    
    // Sort by precedence (higher precedence wins)
    std::vector<RolePolicy> sortedPolicies = policies;
    std::sort(sortedPolicies.begin(), sortedPolicies.end(),
              [](const RolePolicy& a, const RolePolicy& b) {
                  return a.precedence > b.precedence;
              });
    
    // Start with highest precedence policy
    RolePolicy result = sortedPolicies[0];
    
    // Merge constraints from other policies (lower precedence)
    for (size_t i = 1; i < sortedPolicies.size(); ++i) {
        const auto& policy = sortedPolicies[i];
        
        for (const auto& [paramName, constraint] : policy.constraints) {
            // Only add if not already present (higher precedence wins)
            if (result.constraints.find(paramName) == result.constraints.end()) {
                result.constraints[paramName] = constraint;
            }
        }
        
        // Merge priors (weighted average)
        for (const auto& [paramName, prior] : policy.priors) {
            if (result.priors.find(paramName) != result.priors.end()) {
                result.priors[paramName] = (result.priors[paramName] + prior) / 2.0;
            } else {
                result.priors[paramName] = prior;
            }
        }
        
        // Merge penalties (maximum)
        for (const auto& [paramName, penalty] : policy.penalties) {
            if (result.penalties.find(paramName) != result.penalties.end()) {
                result.penalties[paramName] = std::max(result.penalties[paramName], penalty);
            } else {
                result.penalties[paramName] = penalty;
            }
        }
    }
    
    return result;
}

#ifdef YAMLCPP_FOUND
PolicyConstraint PolicyCompiler::parseConstraint(const YAML::Node& node) const {
    PolicyConstraint constraint;
    
    std::string typeStr = node["type"].as<std::string>("range");
    if (typeStr == "range") {
        constraint.type = ConstraintType::RANGE;
        if (node["min"] && node["max"]) {
            constraint.range = {node["min"].as<double>(), node["max"].as<double>()};
        }
    } else if (typeStr == "enum") {
        constraint.type = ConstraintType::ENUM;
        if (node["options"]) {
            for (const auto& option : node["options"]) {
                constraint.options.push_back(option.as<std::string>());
            }
        }
    } else if (typeStr == "boolean") {
        constraint.type = ConstraintType::BOOLEAN;
        constraint.defaultValue = node["default"].as<bool>(false);
    } else if (typeStr == "custom") {
        constraint.type = ConstraintType::CUSTOM;
        // Custom validator would be set elsewhere
    }
    
    constraint.weight = node["weight"].as<double>(1.0);
    
    return constraint;
}
#endif

#ifdef YAMLCPP_FOUND
std::map<std::string, double> PolicyCompiler::parsePriors(const YAML::Node& node) const {
    std::map<std::string, double> priors;
    
    for (const auto& priorNode : node) {
        std::string paramName = priorNode.first.as<std::string>();
        double priorValue = priorNode.second.as<double>();
        priors[paramName] = priorValue;
    }
    
    return priors;
}
#endif

#ifdef YAMLCPP_FOUND
std::map<std::string, double> PolicyCompiler::parsePenalties(const YAML::Node& node) const {
    std::map<std::string, double> penalties;
    
    for (const auto& penaltyNode : node) {
        std::string paramName = penaltyNode.first.as<std::string>();
        double penaltyValue = penaltyNode.second.as<double>();
        penalties[paramName] = penaltyValue;
    }
    
    return penalties;
}
#endif

// PolicyEngine implementation
void PolicyEngine::applyPolicy(DSPGraph& graph, const RolePolicy& policy, 
                              const MusicalContext& context) const {
    // Apply constraints to all stages
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            for (const auto& [paramName, constraint] : policy.constraints) {
                applyConstraint(*stage, constraint);
            }
        }
    }
    
    // Apply role-specific transformations
    applyRoleTransformations(graph, policy.role, context);
    
    // Apply context-aware adjustments
    adjustForTempo(graph, context);
    adjustForKey(graph, context);
    adjustForScale(graph, context);
}

std::vector<std::string> PolicyEngine::checkCompliance(const DSPGraph& graph, 
                                                      const RolePolicy& policy) const {
    std::vector<std::string> violations;
    
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            for (const auto& [paramName, constraint] : policy.constraints) {
                auto paramValue = stage->getParameter(paramName);
                if (std::holds_alternative<double>(paramValue)) {
                    double value = std::get<double>(paramValue);
                    if (!validateConstraint(constraint, value)) {
                        violations.push_back("Stage " + stageName + " parameter " + 
                                           paramName + " violates constraint");
                    }
                }
            }
        }
    }
    
    return violations;
}

void PolicyEngine::applyRoleTransformations(DSPGraph& graph, Role role, 
                                           const MusicalContext& context) const {
    switch (role) {
        case Role::PAD:
            applyPadTransformations(graph, context);
            break;
        case Role::BASS:
            applyBassTransformations(graph, context);
            break;
        case Role::LEAD:
            applyLeadTransformations(graph, context);
            break;
        case Role::DRUM:
            applyDrumTransformations(graph, context);
            break;
        case Role::PERCUSSION:
            applyPercussionTransformations(graph, context);
            break;
        case Role::AMBIENT:
            applyAmbientTransformations(graph, context);
            break;
        case Role::TEXTURE:
            applyTextureTransformations(graph, context);
            break;
        default:
            break;
    }
}

double PolicyEngine::computePolicyScore(const DSPGraph& graph, const RolePolicy& policy) const {
    double score = 1.0;
    
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            for (const auto& [paramName, constraint] : policy.constraints) {
                auto paramValue = stage->getParameter(paramName);
                if (std::holds_alternative<double>(paramValue)) {
                    double value = std::get<double>(paramValue);
                    double penalty = computeViolationPenalty(constraint, value);
                    score *= (1.0 - penalty * constraint.weight);
                }
            }
        }
    }
    
    return std::max(0.0, score);
}

std::vector<std::string> PolicyEngine::getRecommendations(const DSPGraph& graph, 
                                                         const RolePolicy& policy) const {
    std::vector<std::string> recommendations;
    
    auto violations = checkCompliance(graph, policy);
    for (const auto& violation : violations) {
        recommendations.push_back("Fix: " + violation);
    }
    
    // Add role-specific recommendations
    switch (policy.role) {
        case Role::PAD:
            recommendations.push_back("Consider adding reverb for atmospheric depth");
            recommendations.push_back("Use slow attack and release for smooth transitions");
            break;
        case Role::BASS:
            recommendations.push_back("Ensure low-frequency content is prominent");
            recommendations.push_back("Use tight envelope for punch");
            break;
        case Role::LEAD:
            recommendations.push_back("Add modulation for expressiveness");
            recommendations.push_back("Use bright filter settings");
            break;
        default:
            break;
    }
    
    return recommendations;
}

void PolicyEngine::applyPadTransformations(DSPGraph& graph, const MusicalContext& context) const {
    // Apply pad-specific transformations
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            // Set pad-appropriate parameters
            if (stage->getType() == StageType::ENVELOPE) {
                stage->setParameter("attack", 0.2);
                stage->setParameter("decay", 0.5);
                stage->setParameter("sustain", 0.7);
                stage->setParameter("release", 2.0);
            }
        }
    }
}

void PolicyEngine::applyBassTransformations(DSPGraph& graph, const MusicalContext& context) const {
    // Apply bass-specific transformations
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            // Set bass-appropriate parameters
            if (stage->getType() == StageType::OSCILLATOR) {
                stage->setParameter("frequency", 100.0);
                stage->setParameter("amplitude", 0.8);
            }
            if (stage->getType() == StageType::FILTER) {
                stage->setParameter("cutoff", 200.0);
                stage->setParameter("resonance", 0.3);
            }
        }
    }
}

void PolicyEngine::applyLeadTransformations(DSPGraph& graph, const MusicalContext& context) const {
    // Apply lead-specific transformations
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            // Set lead-appropriate parameters
            if (stage->getType() == StageType::OSCILLATOR) {
                stage->setParameter("frequency", 1000.0);
                stage->setParameter("amplitude", 0.9);
            }
        }
    }
}

void PolicyEngine::applyDrumTransformations(DSPGraph& graph, const MusicalContext& context) const {
    // Apply drum-specific transformations
    // Implementation would be similar to other roles
}

void PolicyEngine::applyPercussionTransformations(DSPGraph& graph, const MusicalContext& context) const {
    // Apply percussion-specific transformations
    // Implementation would be similar to other roles
}

void PolicyEngine::applyAmbientTransformations(DSPGraph& graph, const MusicalContext& context) const {
    // Apply ambient-specific transformations
    // Implementation would be similar to other roles
}

void PolicyEngine::applyTextureTransformations(DSPGraph& graph, const MusicalContext& context) const {
    // Apply texture-specific transformations
    // Implementation would be similar to other roles
}

void PolicyEngine::applyConstraint(DSPStage& stage, const PolicyConstraint& constraint) const {
    switch (constraint.type) {
        case ConstraintType::RANGE:
            if (constraint.range.size() >= 2) {
                applyRangeConstraint(stage, constraint.parameter, 
                                   constraint.range[0], constraint.range[1]);
            }
            break;
        case ConstraintType::ENUM:
            applyEnumConstraint(stage, constraint.parameter, constraint.options);
            break;
        case ConstraintType::BOOLEAN:
            applyBooleanConstraint(stage, constraint.parameter, constraint.defaultValue);
            break;
        case ConstraintType::CUSTOM:
            applyCustomConstraint(stage, constraint);
            break;
    }
}

void PolicyEngine::applyRangeConstraint(DSPStage& stage, const std::string& param, 
                                       double minVal, double maxVal) const {
    auto currentValue = stage.getParameter(param);
    if (std::holds_alternative<double>(currentValue)) {
        double value = std::get<double>(currentValue);
        double clampedValue = std::clamp(value, minVal, maxVal);
        if (clampedValue != value) {
            stage.setParameter(param, clampedValue);
        }
    }
}

void PolicyEngine::applyEnumConstraint(DSPStage& stage, const std::string& param, 
                                      const std::vector<std::string>& options) const {
    auto currentValue = stage.getParameter(param);
    if (std::holds_alternative<std::string>(currentValue)) {
        std::string value = std::get<std::string>(currentValue);
        if (std::find(options.begin(), options.end(), value) == options.end()) {
            // Set to first valid option
            stage.setParameter(param, options[0]);
        }
    }
}

void PolicyEngine::applyBooleanConstraint(DSPStage& stage, const std::string& param, 
                                         bool defaultValue) const {
    auto currentValue = stage.getParameter(param);
    if (std::holds_alternative<bool>(currentValue)) {
        // Boolean value is already valid
    } else {
        stage.setParameter(param, defaultValue);
    }
}

void PolicyEngine::applyCustomConstraint(DSPStage& stage, const PolicyConstraint& constraint) const {
    if (constraint.customValidator) {
        auto currentValue = stage.getParameter(constraint.parameter);
        if (std::holds_alternative<double>(currentValue)) {
            double value = std::get<double>(currentValue);
            if (!constraint.customValidator(value)) {
                // Apply correction based on constraint type
                // This would be implemented based on specific constraint requirements
            }
        }
    }
}

void PolicyEngine::adjustForTempo(DSPGraph& graph, const MusicalContext& context) const {
    // Adjust time-based parameters for tempo
    double tempoFactor = context.tempo / 120.0; // Normalize to 120 BPM
    
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            // Adjust envelope times
            if (stage->getType() == StageType::ENVELOPE) {
                auto attack = stage->getParameter("attack");
                auto decay = stage->getParameter("decay");
                auto release = stage->getParameter("release");
                
                if (std::holds_alternative<double>(attack)) {
                    double newAttack = std::get<double>(attack) / tempoFactor;
                    stage->setParameter("attack", newAttack);
                }
                if (std::holds_alternative<double>(decay)) {
                    double newDecay = std::get<double>(decay) / tempoFactor;
                    stage->setParameter("decay", newDecay);
                }
                if (std::holds_alternative<double>(release)) {
                    double newRelease = std::get<double>(release) / tempoFactor;
                    stage->setParameter("release", newRelease);
                }
            }
        }
    }
}

void PolicyEngine::adjustForKey(DSPGraph& graph, const MusicalContext& context) const {
    // Adjust frequency parameters for key
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            if (stage->getType() == StageType::OSCILLATOR) {
                auto freq = stage->getParameter("frequency");
                if (std::holds_alternative<double>(freq)) {
                    double currentFreq = std::get<double>(freq);
                    // Adjust frequency based on key (simplified)
                    double keyAdjustment = std::pow(2.0, context.key / 12.0);
                    double newFreq = currentFreq * keyAdjustment;
                    stage->setParameter("frequency", newFreq);
                }
            }
        }
    }
}

void PolicyEngine::adjustForScale(DSPGraph& graph, const MusicalContext& context) const {
    // Adjust parameters based on scale
    // This would involve more complex musical theory calculations
}

bool PolicyEngine::validateConstraint(const PolicyConstraint& constraint, double value) const {
    switch (constraint.type) {
        case ConstraintType::RANGE:
            if (constraint.range.size() >= 2) {
                return value >= constraint.range[0] && value <= constraint.range[1];
            }
            return true;
        case ConstraintType::CUSTOM:
            return constraint.customValidator ? constraint.customValidator(value) : true;
        default:
            return true;
    }
}

double PolicyEngine::computeViolationPenalty(const PolicyConstraint& constraint, double value) const {
    if (validateConstraint(constraint, value)) {
        return 0.0;
    }
    
    switch (constraint.type) {
        case ConstraintType::RANGE:
            if (constraint.range.size() >= 2) {
                double minVal = constraint.range[0];
                double maxVal = constraint.range[1];
                if (value < minVal) {
                    return (minVal - value) / (maxVal - minVal);
                } else if (value > maxVal) {
                    return (value - maxVal) / (maxVal - minVal);
                }
            }
            break;
        case ConstraintType::CUSTOM:
            // Custom penalty calculation would be implemented here
            break;
        default:
            break;
    }
    
    return 1.0; // Maximum penalty
}

// PolicyManager implementation
void PolicyManager::loadPoliciesFromDirectory(const std::string& directoryPath) {
    // Implementation would scan directory for YAML files and load them
    // This is a placeholder
}

const RolePolicy* PolicyManager::getPolicy(Role role) const {
    auto it = policies_.find(role);
    return (it != policies_.end()) ? &it->second : nullptr;
}

std::vector<RolePolicy> PolicyManager::getAllPolicies() const {
    std::vector<RolePolicy> result;
    for (const auto& [role, policy] : policies_) {
        result.push_back(policy);
    }
    return result;
}

void PolicyManager::updatePolicy(const RolePolicy& policy) {
    policies_[policy.role] = policy;
}

void PolicyManager::removePolicy(Role role) {
    policies_.erase(role);
}

std::vector<std::string> PolicyManager::getConflicts() const {
    std::vector<std::string> conflicts;
    
    auto allPolicies = getAllPolicies();
    for (size_t i = 0; i < allPolicies.size(); ++i) {
        for (size_t j = i + 1; j < allPolicies.size(); ++j) {
            auto policyConflicts = detectConflicts(allPolicies[i], allPolicies[j]);
            conflicts.insert(conflicts.end(), policyConflicts.begin(), policyConflicts.end());
        }
    }
    
    return conflicts;
}

void PolicyManager::resolveAllConflicts() {
    auto allPolicies = getAllPolicies();
    if (allPolicies.empty()) return;
    
    RolePolicy resolved = compiler_.resolveConflicts(allPolicies);
    
    // Update all policies with resolved version
    for (const auto& policy : allPolicies) {
        policies_[policy.role] = resolved;
    }
}

std::string PolicyManager::exportPoliciesToYAML() const {
    // Implementation would serialize policies to YAML
    return "";
}

void PolicyManager::importPoliciesFromYAML(const std::string& yamlContent) {
    // Implementation would parse YAML and load policies
}

std::vector<std::string> PolicyManager::detectConflicts(const RolePolicy& policy1, 
                                                       const RolePolicy& policy2) const {
    std::vector<std::string> conflicts;
    
    // Check for conflicting constraints
    for (const auto& [paramName, constraint1] : policy1.constraints) {
        auto it = policy2.constraints.find(paramName);
        if (it != policy2.constraints.end()) {
            const auto& constraint2 = it->second;
            
            // Check for range conflicts
            if (constraint1.type == ConstraintType::RANGE && 
                constraint2.type == ConstraintType::RANGE) {
                if (constraint1.range.size() >= 2 && constraint2.range.size() >= 2) {
                    double min1 = constraint1.range[0];
                    double max1 = constraint1.range[1];
                    double min2 = constraint2.range[0];
                    double max2 = constraint2.range[1];
                    
                    if (max1 < min2 || max2 < min1) {
                        conflicts.push_back("Conflicting ranges for parameter " + paramName);
                    }
                }
            }
        }
    }
    
    return conflicts;
}

bool PolicyManager::validatePolicyConsistency(const RolePolicy& policy) const {
    // Check that all constraints are internally consistent
    for (const auto& [paramName, constraint] : policy.constraints) {
        if (constraint.type == ConstraintType::RANGE && constraint.range.size() >= 2) {
            if (constraint.range[0] > constraint.range[1]) {
                return false;
            }
        }
    }
    
    return true;
}

} // namespace aiaudio