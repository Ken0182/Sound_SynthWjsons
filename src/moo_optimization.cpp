#include "moo_optimization.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <queue>

namespace aiaudio {

MOOOptimizer::MOOOptimizer(const std::string& metricsConfigPath) {
    // Load metrics configuration
    std::ifstream file(metricsConfigPath);
    if (!file.is_open()) {
        throw AIAudioException("Could not open metrics config: " + metricsConfigPath);
    }
    
    // Parse YAML-like config (simplified)
    std::string line;
    Role currentRole = Role::UNKNOWN;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line.find("role:") != std::string::npos) {
            std::string roleStr = line.substr(line.find(':') + 1);
            // Parse role string to enum
            if (roleStr.find("pad") != std::string::npos) currentRole = Role::PAD;
            else if (roleStr.find("bass") != std::string::npos) currentRole = Role::BASS;
            else if (roleStr.find("lead") != std::string::npos) currentRole = Role::LEAD;
            else if (roleStr.find("drum") != std::string::npos) currentRole = Role::DRUM;
            else if (roleStr.find("percussion") != std::string::npos) currentRole = Role::PERCUSSION;
            else if (roleStr.find("ambient") != std::string::npos) currentRole = Role::AMBIENT;
            else if (roleStr.find("texture") != std::string::npos) currentRole = Role::TEXTURE;
        }
        else if (line.find("thresholds:") != std::string::npos) {
            // Parse thresholds for current role
            std::map<std::string, double> thresholds;
            // Implementation would parse threshold values
            roleThresholds_[currentRole] = thresholds;
        }
    }
}

MOOOptimizer::EvalMetrics MOOOptimizer::evaluate(const AudioBuffer& audio, 
                                                 Role role, 
                                                 const MusicalContext& context,
                                                 const std::string& query) {
    EvalMetrics metrics;
    
    // Calculate individual objectives
    metrics.objectives.semMatch = calculateSemanticMatch(audio, query, role);
    metrics.objectives.mixReadiness = calculateMixReadiness(audio, role, AudioConstraints{});
    metrics.objectives.perceptualQuality = calculatePerceptualQuality(audio);
    metrics.objectives.stability = calculateStability(audio);
    metrics.objectives.preferenceWin = 0.5; // Placeholder - would need trace data
    
    // Check constraints
    AudioConstraints constraints;
    metrics.violations = checkConstraints(audio, constraints);
    metrics.feasible = metrics.violations.empty();
    
    // Calculate overall score (weighted sum for now)
    if (metrics.feasible) {
        double weights[] = {0.3, 0.25, 0.25, 0.1, 0.1};
        metrics.overallScore = 0.0;
        for (size_t i = 0; i < 5; ++i) {
            metrics.overallScore += weights[i] * metrics.objectives[i];
        }
    }
    
    return metrics;
}

bool MOOOptimizer::dominates(const ParetoPoint& a, const ParetoPoint& b) const {
    bool atLeastOneBetter = false;
    bool allEqualOrBetter = true;
    
    for (size_t i = 0; i < ObjectiveVector::size(); ++i) {
        if (a.objectives[i] > b.objectives[i]) {
            atLeastOneBetter = true;
        } else if (a.objectives[i] < b.objectives[i]) {
            allEqualOrBetter = false;
        }
    }
    
    return atLeastOneBetter && allEqualOrBetter;
}

double MOOOptimizer::calculateHypervolume(const std::vector<ParetoPoint>& front) const {
    if (front.empty()) return 0.0;
    
    // Reference point (worst possible values)
    ObjectiveVector reference;
    for (size_t i = 0; i < ObjectiveVector::size(); ++i) {
        reference[i] = 0.0; // Assuming all objectives are maximized
    }
    
    // Calculate hypervolume using Lebesgue measure
    double hypervolume = 0.0;
    std::vector<ParetoPoint> sortedFront = front;
    
    // Sort by first objective
    std::sort(sortedFront.begin(), sortedFront.end(),
              [](const ParetoPoint& a, const ParetoPoint& b) {
                  return a.objectives[0] > b.objectives[0];
              });
    
    // Calculate hypervolume contribution of each point
    for (const auto& point : sortedFront) {
        double contribution = 1.0;
        for (size_t i = 0; i < ObjectiveVector::size(); ++i) {
            contribution *= std::max(0.0, point.objectives[i] - reference[i]);
        }
        hypervolume += contribution;
    }
    
    return hypervolume;
}

std::vector<ParetoPoint> MOOOptimizer::epsilonConstraint(
    const std::vector<ParetoPoint>& population,
    size_t primaryObjective,
    double epsilon) const {
    
    std::vector<ParetoPoint> result;
    
    for (const auto& point : population) {
        bool satisfiesConstraints = true;
        
        // Check epsilon constraints for non-primary objectives
        for (size_t i = 0; i < ObjectiveVector::size(); ++i) {
            if (i != primaryObjective) {
                if (point.objectives[i] < epsilon) {
                    satisfiesConstraints = false;
                    break;
                }
            }
        }
        
        if (satisfiesConstraints) {
            result.push_back(point);
        }
    }
    
    return result;
}

std::vector<ParetoPoint> MOOOptimizer::nsga2Selection(
    const std::vector<ParetoPoint>& population,
    size_t targetSize) const {
    
    std::vector<ParetoPoint> result;
    std::vector<ParetoPoint> working = population;
    
    // Calculate dominance relationships
    std::vector<std::vector<size_t>> dominatedBy(population.size());
    std::vector<int> dominationCount(population.size(), 0);
    
    for (size_t i = 0; i < population.size(); ++i) {
        for (size_t j = 0; j < population.size(); ++j) {
            if (i != j) {
                if (dominates(population[i], population[j])) {
                    dominatedBy[i].push_back(j);
                    dominationCount[j]++;
                }
            }
        }
    }
    
    // NSGA-II selection process
    std::vector<int> front(population.size());
    std::vector<int> crowdingDistance(population.size(), 0);
    
    // Assign front numbers
    std::queue<size_t> currentFront;
    for (size_t i = 0; i < population.size(); ++i) {
        if (dominationCount[i] == 0) {
            front[i] = 0;
            currentFront.push(i);
        }
    }
    
    int frontNumber = 0;
    while (!currentFront.empty() && result.size() < targetSize) {
        std::vector<size_t> nextFront;
        
        while (!currentFront.empty() && result.size() < targetSize) {
            size_t current = currentFront.front();
            currentFront.pop();
            
            if (result.size() < targetSize) {
                result.push_back(population[current]);
            }
            
            // Update domination counts for dominated solutions
            for (size_t dominated : dominatedBy[current]) {
                dominationCount[dominated]--;
                if (dominationCount[dominated] == 0) {
                    front[dominated] = frontNumber + 1;
                    nextFront.push_back(dominated);
                }
            }
        }
        
        // Add remaining solutions from current front if space available
        if (result.size() < targetSize) {
            // Calculate crowding distance and sort
            std::vector<std::pair<double, size_t>> crowdingPairs;
            for (size_t i = 0; i < population.size(); ++i) {
                if (front[i] == frontNumber) {
                    crowdingPairs.push_back({crowdingDistance[i], i});
                }
            }
            
            std::sort(crowdingPairs.rbegin(), crowdingPairs.rend());
            
            for (const auto& pair : crowdingPairs) {
                if (result.size() < targetSize) {
                    result.push_back(population[pair.second]);
                }
            }
        }
        
        // Move to next front
        for (size_t idx : nextFront) {
            currentFront.push(idx);
        }
        frontNumber++;
    }
    
    return result;
}

double MOOOptimizer::bradleyTerryWinProb(const Trace& traceA, const Trace& traceB) const {
    // Bradley-Terry model: P(A beats B) = exp(θ_A) / (exp(θ_A) + exp(θ_B))
    // where θ is the strength parameter
    
    double strengthA = traceA.decisions.confidence;
    double strengthB = traceB.decisions.confidence;
    
    double expA = std::exp(strengthA);
    double expB = std::exp(strengthB);
    
    return expA / (expA + expB);
}

double MOOOptimizer::calculateSemanticMatch(const AudioBuffer& audio, 
                                           const std::string& query, 
                                           Role role) const {
    // Placeholder implementation
    // In practice, this would use semantic embeddings and audio feature extraction
    
    if (query.empty()) return 0.5;
    
    // Simple keyword matching for demonstration
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    double score = 0.0;
    
    // Role-based semantic matching
    switch (role) {
        case Role::PAD:
            if (lowerQuery.find("pad") != std::string::npos ||
                lowerQuery.find("ambient") != std::string::npos ||
                lowerQuery.find("atmospheric") != std::string::npos) {
                score += 0.3;
            }
            break;
        case Role::BASS:
            if (lowerQuery.find("bass") != std::string::npos ||
                lowerQuery.find("low") != std::string::npos ||
                lowerQuery.find("sub") != std::string::npos) {
                score += 0.3;
            }
            break;
        case Role::LEAD:
            if (lowerQuery.find("lead") != std::string::npos ||
                lowerQuery.find("melody") != std::string::npos ||
                lowerQuery.find("solo") != std::string::npos) {
                score += 0.3;
            }
            break;
        default:
            break;
    }
    
    // Audio feature-based matching
    double spectralCentroid = calculateSpectralCentroid(audio);
    double lufs = calculateLUFS(audio);
    
    // Match spectral characteristics
    if (role == Role::BASS && spectralCentroid < 200.0) score += 0.2;
    if (role == Role::PAD && spectralCentroid > 1000.0) score += 0.2;
    if (role == Role::LEAD && spectralCentroid > 2000.0) score += 0.2;
    
    // Match loudness characteristics
    if (lufs > -20.0 && lufs < -10.0) score += 0.2;
    
    return std::min(1.0, score);
}

double MOOOptimizer::calculateMixReadiness(const AudioBuffer& audio, 
                                          Role role, 
                                          const AudioConstraints& constraints) const {
    double score = 0.0;
    
    // LUFS target compliance
    double lufs = calculateLUFS(audio);
    double lufsError = std::abs(lufs - constraints.lufsTarget);
    if (lufsError < 1.0) score += 0.3;
    else if (lufsError < 3.0) score += 0.2;
    
    // True peak compliance
    double truePeak = calculateTruePeak(audio);
    if (truePeak <= constraints.truePeakLimit) score += 0.3;
    else if (truePeak <= constraints.truePeakLimit + 1.0) score += 0.2;
    
    // Crest factor compliance
    double crestFactor = calculateCrestFactor(audio);
    if (crestFactor >= constraints.crestFactorMin && crestFactor <= constraints.crestFactorMax) {
        score += 0.4;
    }
    
    return std::min(1.0, score);
}

double MOOOptimizer::calculatePerceptualQuality(const AudioBuffer& audio) const {
    // Placeholder for perceptual quality calculation
    // Would include multiresolution STFT loss, spectral centroid bounds, etc.
    
    double score = 0.0;
    
    // Check for clipping
    bool clipped = false;
    for (double sample : audio) {
        if (std::abs(sample) >= 1.0) {
            clipped = true;
            break;
        }
    }
    if (!clipped) score += 0.3;
    
    // Check for DC offset
    double dcOffset = std::accumulate(audio.begin(), audio.end(), 0.0) / audio.size();
    if (std::abs(dcOffset) < 0.001) score += 0.2;
    
    // Check for silence
    double rms = 0.0;
    for (double sample : audio) {
        rms += sample * sample;
    }
    rms = std::sqrt(rms / audio.size());
    if (rms > 0.001) score += 0.3;
    
    // Spectral quality (simplified)
    double spectralCentroid = calculateSpectralCentroid(audio);
    if (spectralCentroid > 0.0 && spectralCentroid < 20000.0) score += 0.2;
    
    return std::min(1.0, score);
}

double MOOOptimizer::calculateStability(const AudioBuffer& audio) const {
    double score = 0.0;
    
    // Check for xruns (simplified - would need real-time monitoring)
    score += 0.3; // Assume no xruns for now
    
    // Check for denormals (simplified)
    bool hasDenormals = false;
    for (double sample : audio) {
        if (std::fpclassify(sample) == FP_SUBNORMAL) {
            hasDenormals = true;
            break;
        }
    }
    if (!hasDenormals) score += 0.3;
    
    // Check DC offset
    double dcOffset = std::accumulate(audio.begin(), audio.end(), 0.0) / audio.size();
    if (std::abs(dcOffset) < 0.001) score += 0.2;
    
    // Check for stability over time (variance)
    double mean = dcOffset;
    double variance = 0.0;
    for (double sample : audio) {
        double diff = sample - mean;
        variance += diff * diff;
    }
    variance /= audio.size();
    
    if (variance > 0.0 && variance < 1.0) score += 0.2;
    
    return std::min(1.0, score);
}

std::vector<MOOOptimizer::ConstraintViolation> MOOOptimizer::checkConstraints(
    const AudioBuffer& audio,
    const AudioConstraints& constraints) const {
    
    std::vector<ConstraintViolation> violations;
    
    // Check for hard clips
    if (constraints.noHardClips) {
        for (double sample : audio) {
            if (std::abs(sample) >= 1.0) {
                violations.push_back({"hard_clip", std::abs(sample), 1.0});
                break;
            }
        }
    }
    
    // Check true peak limit
    double truePeak = calculateTruePeak(audio);
    if (truePeak > constraints.truePeakLimit) {
        violations.push_back({"true_peak", truePeak, constraints.truePeakLimit});
    }
    
    // Check LUFS target
    double lufs = calculateLUFS(audio);
    if (std::abs(lufs - constraints.lufsTarget) > 3.0) {
        violations.push_back({"lufs_target", lufs, constraints.lufsTarget});
    }
    
    return violations;
}

double MOOOptimizer::calculateLUFS(const AudioBuffer& audio) const {
    // Simplified LUFS calculation
    // In practice, this would use proper K-weighting and gating
    
    double sum = 0.0;
    for (double sample : audio) {
        sum += sample * sample;
    }
    double rms = std::sqrt(sum / audio.size());
    
    // Convert to LUFS (simplified)
    return 20.0 * std::log10(std::max(rms, 1e-10)) - 23.0;
}

double MOOOptimizer::calculateTruePeak(const AudioBuffer& audio) const {
    double maxPeak = 0.0;
    for (double sample : audio) {
        maxPeak = std::max(maxPeak, std::abs(sample));
    }
    return 20.0 * std::log10(std::max(maxPeak, 1e-10));
}

double MOOOptimizer::calculateCrestFactor(const AudioBuffer& audio) const {
    double sum = 0.0;
    double maxPeak = 0.0;
    
    for (double sample : audio) {
        sum += sample * sample;
        maxPeak = std::max(maxPeak, std::abs(sample));
    }
    
    double rms = std::sqrt(sum / audio.size());
    if (rms < 1e-10) return 0.0;
    
    return 20.0 * std::log10(maxPeak / rms);
}

double MOOOptimizer::calculateSpectralCentroid(const AudioBuffer& audio) const {
    // Simplified spectral centroid calculation
    // In practice, this would use FFT
    
    double weightedSum = 0.0;
    double magnitudeSum = 0.0;
    
    for (size_t i = 0; i < audio.size(); ++i) {
        double magnitude = std::abs(audio[i]);
        double frequency = static_cast<double>(i) * 44100.0 / audio.size();
        
        weightedSum += frequency * magnitude;
        magnitudeSum += magnitude;
    }
    
    return magnitudeSum > 0.0 ? weightedSum / magnitudeSum : 0.0;
}

} // namespace aiaudio