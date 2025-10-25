#pragma once

#include "core_types.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace aiaudio {

// Roles & Policy Language System

// Policy constraint types
enum class ConstraintType {
    RANGE,      // min <= value <= max
    ENUM,       // value in {options}
    BOOLEAN,    // true/false
    CUSTOM      // custom validation function
};

// Policy constraint definition
struct PolicyConstraint {
    ConstraintType type;
    std::string parameter;
    std::vector<double> range;        // [min, max] for RANGE
    std::vector<std::string> options; // for ENUM
    bool defaultValue = false;        // for BOOLEAN
    std::function<bool(double)> customValidator; // for CUSTOM
    double weight = 1.0;              // constraint importance
};

// Policy definition for a role
struct RolePolicy {
    Role role;
    std::string version;
    std::map<std::string, PolicyConstraint> constraints;
    std::map<std::string, double> priors;     // parameter priors
    std::map<std::string, double> penalties;  // violation penalties
    int precedence = 0;                       // conflict resolution order
    std::string description;
};

// Policy compiler
class PolicyCompiler {
public:
    // Load policy from YAML
    RolePolicy loadPolicy(const std::string& yamlContent, Role role);
    
    // Load policy from file
    RolePolicy loadPolicyFromFile(const std::string& filePath, Role role);
    
    // Validate policy
    std::vector<std::string> validatePolicy(const RolePolicy& policy) const;
    
    // Compile constraints to convex ranges
    std::map<std::string, std::pair<double, double>> compileConstraints(
        const RolePolicy& policy) const;
    
    // Resolve conflicts between policies
    RolePolicy resolveConflicts(const std::vector<RolePolicy>& policies) const;
    
private:
    // YAML parsing helpers
    PolicyConstraint parseConstraint(const YAML::Node& node) const;
    std::map<std::string, double> parsePriors(const YAML::Node& node) const;
    std::map<std::string, double> parsePenalties(const YAML::Node& node) const;
    
    // Constraint validation
    bool validateConstraint(const PolicyConstraint& constraint, double value) const;
    double computeViolationPenalty(const PolicyConstraint& constraint, double value) const;
};

// Policy application engine
class PolicyEngine {
public:
    // Apply policy to DSP graph
    void applyPolicy(DSPGraph& graph, const RolePolicy& policy, 
                    const MusicalContext& context) const;
    
    // Check policy compliance
    std::vector<std::string> checkCompliance(const DSPGraph& graph, 
                                            const RolePolicy& policy) const;
    
    // Apply role-specific transformations
    void applyRoleTransformations(DSPGraph& graph, Role role, 
                                 const MusicalContext& context) const;
    
    // Compute policy score
    double computePolicyScore(const DSPGraph& graph, const RolePolicy& policy) const;
    
    // Get policy recommendations
    std::vector<std::string> getRecommendations(const DSPGraph& graph, 
                                               const RolePolicy& policy) const;
    
private:
    // Role-specific transformations
    void applyPadTransformations(DSPGraph& graph, const MusicalContext& context) const;
    void applyBassTransformations(DSPGraph& graph, const MusicalContext& context) const;
    void applyLeadTransformations(DSPGraph& graph, const MusicalContext& context) const;
    void applyDrumTransformations(DSPGraph& graph, const MusicalContext& context) const;
    void applyPercussionTransformations(DSPGraph& graph, const MusicalContext& context) const;
    void applyAmbientTransformations(DSPGraph& graph, const MusicalContext& context) const;
    void applyTextureTransformations(DSPGraph& graph, const MusicalContext& context) const;
    
    // Constraint application
    void applyConstraint(DSPStage& stage, const PolicyConstraint& constraint) const;
    void applyRangeConstraint(DSPStage& stage, const std::string& param, 
                             double minVal, double maxVal) const;
    void applyEnumConstraint(DSPStage& stage, const std::string& param, 
                            const std::vector<std::string>& options) const;
    void applyBooleanConstraint(DSPStage& stage, const std::string& param, 
                               bool defaultValue) const;
    void applyCustomConstraint(DSPStage& stage, const PolicyConstraint& constraint) const;
    
    // Context-aware adjustments
    void adjustForTempo(DSPGraph& graph, const MusicalContext& context) const;
    void adjustForKey(DSPGraph& graph, const MusicalContext& context) const;
    void adjustForScale(DSPGraph& graph, const MusicalContext& context) const;
};

// Policy manager
class PolicyManager {
public:
    // Load all policies from directory
    void loadPoliciesFromDirectory(const std::string& directoryPath);
    
    // Get policy for role
    const RolePolicy* getPolicy(Role role) const;
    
    // Get all policies
    std::vector<RolePolicy> getAllPolicies() const;
    
    // Update policy
    void updatePolicy(const RolePolicy& policy);
    
    // Remove policy
    void removePolicy(Role role);
    
    // Get policy conflicts
    std::vector<std::string> getConflicts() const;
    
    // Resolve all conflicts
    void resolveAllConflicts();
    
    // Export policies to YAML
    std::string exportPoliciesToYAML() const;
    
    // Import policies from YAML
    void importPoliciesFromYAML(const std::string& yamlContent);
    
private:
    std::map<Role, RolePolicy> policies_;
    PolicyCompiler compiler_;
    PolicyEngine engine_;
    
    // Conflict detection
    std::vector<std::string> detectConflicts(const RolePolicy& policy1, 
                                            const RolePolicy& policy2) const;
    
    // Policy validation
    bool validatePolicyConsistency(const RolePolicy& policy) const;
};

// Role-specific policy definitions
class RolePolicyDefinitions {
public:
    // Get default policy for role
    static RolePolicy getDefaultPolicy(Role role);
    
    // Get all default policies
    static std::map<Role, RolePolicy> getAllDefaultPolicies();
    
    // Create custom policy
    static RolePolicy createCustomPolicy(Role role, 
                                        const std::map<std::string, PolicyConstraint>& constraints,
                                        const std::map<std::string, double>& priors = {},
                                        const std::map<std::string, double>& penalties = {});
    
private:
    // Default policy creators
    static RolePolicy createPadPolicy();
    static RolePolicy createBassPolicy();
    static RolePolicy createLeadPolicy();
    static RolePolicy createDrumPolicy();
    static RolePolicy createPercussionPolicy();
    static RolePolicy createAmbientPolicy();
    static RolePolicy createTexturePolicy();
};

// Policy validation and testing
class PolicyValidator {
public:
    // Validate policy against test cases
    bool validatePolicy(const RolePolicy& policy, 
                       const std::vector<DSPGraph>& testGraphs) const;
    
    // Test policy compliance
    std::vector<std::string> testCompliance(const RolePolicy& policy, 
                                           const DSPGraph& graph) const;
    
    // Generate test cases
    std::vector<DSPGraph> generateTestCases(Role role, size_t count = 100) const;
    
    // Benchmark policy performance
    double benchmarkPolicy(const RolePolicy& policy, 
                          const std::vector<DSPGraph>& testGraphs) const;
    
private:
    // Test case generation
    DSPGraph generateRandomGraph(Role role) const;
    void randomizeParameters(DSPGraph& graph, Role role) const;
    
    // Performance measurement
    double measureApplicationTime(const RolePolicy& policy, const DSPGraph& graph) const;
    double measureComplianceTime(const RolePolicy& policy, const DSPGraph& graph) const;
};

// Policy learning and optimization
class PolicyLearner {
public:
    // Learn policy from examples
    RolePolicy learnPolicy(Role role, 
                          const std::vector<DSPGraph>& positiveExamples,
                          const std::vector<DSPGraph>& negativeExamples) const;
    
    // Optimize policy parameters
    RolePolicy optimizePolicy(const RolePolicy& policy, 
                             const std::vector<DSPGraph>& examples,
                             const std::vector<double>& scores) const;
    
    // Extract constraints from examples
    std::vector<PolicyConstraint> extractConstraints(Role role,
                                                    const std::vector<DSPGraph>& examples) const;
    
    // Learn priors from examples
    std::map<std::string, double> learnPriors(Role role,
                                             const std::vector<DSPGraph>& examples) const;
    
private:
    // Constraint extraction
    std::map<std::string, std::pair<double, double>> extractRanges(
        const std::vector<DSPGraph>& examples) const;
    std::map<std::string, std::vector<std::string>> extractEnums(
        const std::vector<DSPGraph>& examples) const;
    
    // Prior learning
    double computeParameterPrior(const std::string& param, 
                                const std::vector<DSPGraph>& examples) const;
    
    // Optimization
    double computePolicyScore(const RolePolicy& policy, 
                             const std::vector<DSPGraph>& examples,
                             const std::vector<double>& scores) const;
};

} // namespace aiaudio