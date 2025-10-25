#pragma once

#include "core_types.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace aiaudio {

// Multi-Objective Optimization framework
class MOOOptimizer {
public:
    struct ParetoPoint {
        ObjectiveVector objectives;
        std::vector<double> parameters;
        double hypervolume = 0.0;
        bool dominated = false;
    };
    
    struct ConstraintViolation {
        std::string constraint;
        double violation;
        double threshold;
    };
    
    // Evaluate audio against objectives
    struct EvalMetrics {
        ObjectiveVector objectives;
        std::vector<ConstraintViolation> violations;
        bool feasible = true;
        double overallScore = 0.0;
    };
    
    // Initialize with role-specific thresholds
    explicit MOOOptimizer(const std::string& metricsConfigPath);
    
    // Main evaluation function
    EvalMetrics evaluate(const AudioBuffer& audio, 
                        Role role, 
                        const MusicalContext& context,
                        const std::string& query = "");
    
    // Pareto dominance checking
    bool dominates(const ParetoPoint& a, const ParetoPoint& b) const;
    
    // Hypervolume calculation
    double calculateHypervolume(const std::vector<ParetoPoint>& front) const;
    
    // Epsilon-constraint method
    std::vector<ParetoPoint> epsilonConstraint(const std::vector<ParetoPoint>& population,
                                              size_t primaryObjective = 0,
                                              double epsilon = 0.1) const;
    
    // NSGA-II selection
    std::vector<ParetoPoint> nsga2Selection(const std::vector<ParetoPoint>& population,
                                           size_t targetSize) const;
    
    // Bradley-Terry preference model
    double bradleyTerryWinProb(const Trace& traceA, const Trace& traceB) const;
    
    // Load role-specific thresholds
    void loadRoleThresholds(const std::string& role, const std::map<std::string, double>& thresholds);
    
private:
    // Individual objective calculators
    double calculateSemanticMatch(const AudioBuffer& audio, 
                                 const std::string& query, 
                                 Role role) const;
    
    double calculateMixReadiness(const AudioBuffer& audio, 
                                Role role, 
                                const AudioConstraints& constraints) const;
    
    double calculatePerceptualQuality(const AudioBuffer& audio) const;
    
    double calculateStability(const AudioBuffer& audio) const;
    
    double calculatePreferenceWin(const Trace& trace, 
                                 const std::vector<Trace>& baselines) const;
    
    // Constraint checking
    std::vector<ConstraintViolation> checkConstraints(const AudioBuffer& audio,
                                                     const AudioConstraints& constraints) const;
    
    // Role-specific thresholds
    std::map<Role, std::map<std::string, double>> roleThresholds_;
    
    // Semantic embedding model (placeholder)
    std::vector<double> encodeQuery(const std::string& query) const;
    
    // Audio analysis utilities
    double calculateLUFS(const AudioBuffer& audio) const;
    double calculateTruePeak(const AudioBuffer& audio) const;
    double calculateCrestFactor(const AudioBuffer& audio) const;
    double calculateSpectralCentroid(const AudioBuffer& audio) const;
    
    // Multiresolution STFT loss
    double multiresSTFTLoss(const AudioBuffer& reference, const AudioBuffer& generated) const;
    
    // Bark loudness calculation
    double barkLoudnessError(const AudioBuffer& reference, const AudioBuffer& generated) const;
};

// Pareto front visualization and analysis
class ParetoAnalyzer {
public:
    struct FrontAnalysis {
        std::vector<ParetoPoint> paretoFront;
        double hypervolume;
        std::vector<double> objectiveRanges;
        std::map<std::string, double> statistics;
    };
    
    FrontAnalysis analyzeFront(const std::vector<ParetoPoint>& population) const;
    
    // Find knee point (best compromise)
    ParetoPoint findKneePoint(const std::vector<ParetoPoint>& front) const;
    
    // Diversity metrics
    double calculateDiversity(const std::vector<ParetoPoint>& front) const;
    
    // Convergence metrics
    double calculateConvergence(const std::vector<ParetoPoint>& current,
                               const std::vector<ParetoPoint>& previous) const;
};

} // namespace aiaudio