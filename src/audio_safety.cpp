#include "audio_safety.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace aiaudio {

// GainStager implementation
void GainStager::stageGain(DSPGraph& graph, double targetGain) {
    auto stageNames = graph.getStageNames();
    
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            double currentGain = calculateStageGain(*stage);
            if (currentGain > targetGain + 3.0) { // 3dB tolerance
                adjustStageGain(*stage, targetGain);
            }
        }
    }
}

std::vector<std::string> GainStager::checkGainStaging(const DSPGraph& graph) {
    std::vector<std::string> issues;
    
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            double gain = calculateStageGain(*stage);
            if (gain > 0.0) {
                issues.push_back("Stage " + stageName + " has positive gain: " + std::to_string(gain) + " dB");
            }
            if (gain < -30.0) {
                issues.push_back("Stage " + stageName + " has very low gain: " + std::to_string(gain) + " dB");
            }
        }
    }
    
    return issues;
}

void GainStager::autoGainStage(DSPGraph& graph) {
    const double targetGain = -18.0;
    stageGain(graph, targetGain);
}

double GainStager::calculateStageGain(const DSPStage& stage) {
    // Calculate gain based on stage type and parameters
    if (stage.getType() == StageType::OSCILLATOR) {
        auto amp = stage.getParameter("amplitude");
        if (std::holds_alternative<double>(amp)) {
            double amplitude = std::get<double>(amp);
            return 20.0 * std::log10(std::max(amplitude, 1e-10));
        }
    }
    
    return 0.0; // Default gain
}

void GainStager::adjustStageGain(DSPStage& stage, double targetGain) {
    if (stage.getType() == StageType::OSCILLATOR) {
        auto amp = stage.getParameter("amplitude");
        if (std::holds_alternative<double>(amp)) {
            double currentAmp = std::get<double>(amp);
            double targetAmp = std::pow(10.0, targetGain / 20.0);
            stage.setParameter("amplitude", targetAmp);
        }
    }
}

// TruePeakLimiter implementation
void TruePeakLimiter::limitTruePeak(AudioBuffer& audio, double limitDb) {
    double truePeak = getTruePeak(audio);
    double limitLinear = std::pow(10.0, limitDb / 20.0);
    
    if (truePeak > limitLinear) {
        double gain = limitLinear / truePeak;
        for (float& sample : audio) {
            sample *= gain;
        }
    }
}

double TruePeakLimiter::getTruePeak(const AudioBuffer& audio) {
    return calculateTruePeak(audio);
}

void TruePeakLimiter::softLimit(AudioBuffer& audio, double threshold, double ratio) {
    applySoftLimiter(audio, threshold, ratio);
}

void TruePeakLimiter::hardLimit(AudioBuffer& audio, double limit) {
    applyHardLimiter(audio, limit);
}

double TruePeakLimiter::calculateTruePeak(const AudioBuffer& audio) {
    double maxPeak = 0.0;
    for (double sample : audio) {
        maxPeak = std::max(maxPeak, std::abs(sample));
    }
    return maxPeak;
}

void TruePeakLimiter::applySoftLimiter(AudioBuffer& audio, double threshold, double ratio) {
    double thresholdLinear = std::pow(10.0, threshold / 20.0);
    double ratioLinear = 1.0 / ratio;
    
    for (float& sample : audio) {
        double absSample = std::abs(sample);
        if (absSample > thresholdLinear) {
            double excess = absSample - thresholdLinear;
            double limitedExcess = excess * ratioLinear;
            double newSample = thresholdLinear + limitedExcess;
            
            if (sample < 0) {
                sample = -newSample;
            } else {
                sample = newSample;
            }
        }
    }
}

void TruePeakLimiter::applyHardLimiter(AudioBuffer& audio, double limit) {
    double limitLinear = std::pow(10.0, limit / 20.0);
    
    for (float& sample : audio) {
        if (sample > limitLinear) {
            sample = limitLinear;
        } else if (sample < -limitLinear) {
            sample = -limitLinear;
        }
    }
}

// FeedbackGuard implementation
bool FeedbackGuard::checkFeedbackStability(const DSPGraph& graph) {
    double loopGain = calculateLoopGain(graph);
    return isStable(loopGain);
}

double FeedbackGuard::calculateLoopGain(const DSPGraph& graph) {
    return findFeedbackLoops(graph);
}

void FeedbackGuard::applyFeedbackProtection(DSPGraph& graph) {
    if (!checkFeedbackStability(graph)) {
        addFeedbackProtection(graph);
    }
}

bool FeedbackGuard::checkRootLocusStability(const DSPGraph& graph) {
    double loopGain = calculateLoopGain(graph);
    return loopGain < 1.0;
}

double FeedbackGuard::findFeedbackLoops(const DSPGraph& graph) {
    // Simplified feedback loop detection
    // In practice, would use graph analysis algorithms
    
    double totalGain = 1.0;
    auto stageNames = graph.getStageNames();
    
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            // Estimate gain contribution
            if (stage->getType() == StageType::OSCILLATOR) {
                auto amp = stage->getParameter("amplitude");
                if (std::holds_alternative<double>(amp)) {
                    totalGain *= std::get<double>(amp);
                }
            }
        }
    }
    
    return totalGain;
}

void FeedbackGuard::addFeedbackProtection(DSPGraph& graph) {
    // Add feedback protection stages
    // This would add limiters, filters, or other protection measures
}

bool FeedbackGuard::isStable(double gain) {
    return gain < 0.99; // Safety margin
}

// AntiChaosSystem implementation
void AntiChaosSystem::preventChaos(DSPGraph& graph) {
    applyChaosPrevention(graph);
}

std::vector<std::string> AntiChaosSystem::checkChaosIndicators(const DSPGraph& graph) {
    std::vector<std::string> indicators;
    
    // Check for extreme parameter values
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            auto paramNames = stage->getParameterNames();
            for (const auto& paramName : paramNames) {
                auto paramValue = stage->getParameter(paramName);
                if (std::holds_alternative<double>(paramValue)) {
                    double value = std::get<double>(paramValue);
                    if (std::isnan(value) || std::isinf(value)) {
                        indicators.push_back("Stage " + stageName + " parameter " + paramName + " is NaN/Inf");
                    }
                    if (std::abs(value) > 1000.0) {
                        indicators.push_back("Stage " + stageName + " parameter " + paramName + " has extreme value: " + std::to_string(value));
                    }
                }
            }
        }
    }
    
    return indicators;
}

void AntiChaosSystem::applyChaosPrevention(DSPGraph& graph) {
    applyChaosPreventionMeasures(graph);
}

bool AntiChaosSystem::monitorRunawayParameters(const DSPGraph& graph) {
    return checkParameterBounds(graph);
}

bool AntiChaosSystem::detectChaos(const AudioBuffer& audio) {
    // Simple chaos detection based on signal properties
    double variance = 0.0;
    double mean = 0.0;
    
    for (double sample : audio) {
        mean += sample;
    }
    mean /= audio.size();
    
    for (double sample : audio) {
        double diff = sample - mean;
        variance += diff * diff;
    }
    variance /= audio.size();
    
    // High variance might indicate chaos
    return variance > 1.0;
}

void AntiChaosSystem::applyChaosPreventionMeasures(DSPGraph& graph) {
    // Apply parameter clamping and other prevention measures
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            auto paramNames = stage->getParameterNames();
            for (const auto& paramName : paramNames) {
                auto paramValue = stage->getParameter(paramName);
                if (std::holds_alternative<double>(paramValue)) {
                    double value = std::get<double>(paramValue);
                    
                    // Clamp extreme values
                    if (std::isnan(value) || std::isinf(value)) {
                        stage->setParameter(paramName, 0.0);
                    } else if (std::abs(value) > 1000.0) {
                        double clampedValue = std::clamp(value, -1000.0, 1000.0);
                        stage->setParameter(paramName, clampedValue);
                    }
                }
            }
        }
    }
}

bool AntiChaosSystem::checkParameterBounds(const DSPGraph& graph) {
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            auto paramNames = stage->getParameterNames();
            for (const auto& paramName : paramNames) {
                auto paramValue = stage->getParameter(paramName);
                if (std::holds_alternative<double>(paramValue)) {
                    double value = std::get<double>(paramValue);
                    if (std::isnan(value) || std::isinf(value) || std::abs(value) > 1000.0) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// HeadroomManager implementation
void HeadroomManager::manageHeadroom(DSPGraph& graph, double targetHeadroom) {
    // Apply headroom management throughout the processing chain
    auto stageNames = graph.getStageNames();
    
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            // Adjust stage parameters to maintain headroom
            if (stage->getType() == StageType::OSCILLATOR) {
                auto amp = stage->getParameter("amplitude");
                if (std::holds_alternative<double>(amp)) {
                    double currentAmp = std::get<double>(amp);
                    double headroomGain = std::pow(10.0, -targetHeadroom / 20.0);
                    double newAmp = currentAmp * headroomGain;
                    stage->setParameter("amplitude", newAmp);
                }
            }
        }
    }
}

double HeadroomManager::checkHeadroom(const AudioBuffer& audio) {
    return calculateHeadroom(audio);
}

void HeadroomManager::applyHeadroomCompensation(AudioBuffer& audio, double targetHeadroom) {
    adjustHeadroom(audio, targetHeadroom);
}

bool HeadroomManager::monitorHeadroom(const AudioBuffer& audio, double minHeadroom) {
    double currentHeadroom = calculateHeadroom(audio);
    return currentHeadroom >= minHeadroom;
}

double HeadroomManager::calculateHeadroom(const AudioBuffer& audio) {
    double peak = 0.0;
    for (double sample : audio) {
        peak = std::max(peak, std::abs(sample));
    }
    
    if (peak > 0.0) {
        return 20.0 * std::log10(1.0 / peak);
    }
    return 0.0;
}

void HeadroomManager::adjustHeadroom(AudioBuffer& audio, double targetHeadroom) {
    double currentHeadroom = calculateHeadroom(audio);
    if (currentHeadroom < targetHeadroom) {
        double gain = std::pow(10.0, (currentHeadroom - targetHeadroom) / 20.0);
        for (float& sample : audio) {
            sample *= gain;
        }
    }
}

// AudioValidator implementation
std::vector<std::string> AudioValidator::validateAudio(const AudioBuffer& audio) {
    std::vector<std::string> issues;
    
    if (checkClipping(audio)) {
        issues.push_back("Audio clipping detected");
    }
    
    if (checkDCOffset(audio)) {
        issues.push_back("DC offset detected");
    }
    
    if (checkSilence(audio)) {
        issues.push_back("Audio is silent or too quiet");
    }
    
    if (checkDenormals(audio)) {
        issues.push_back("Denormal numbers detected");
    }
    
    return issues;
}

std::vector<std::string> AudioValidator::checkAudioIssues(const AudioBuffer& audio) {
    return validateAudio(audio);
}

std::vector<std::string> AudioValidator::validateGraph(const DSPGraph& graph) {
    std::vector<std::string> issues;
    
    // Check for cycles
    if (graph.hasCycles()) {
        issues.push_back("Graph contains cycles");
    }
    
    // Check for disconnected components
    if (!graph.isConnected()) {
        issues.push_back("Graph has disconnected components");
    }
    
    // Check parameter violations
    auto paramIssues = checkParameterViolations(graph);
    issues.insert(issues.end(), paramIssues.begin(), paramIssues.end());
    
    return issues;
}

std::vector<std::string> AudioValidator::checkParameterViolations(const DSPGraph& graph) {
    std::vector<std::string> violations;
    
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            auto paramNames = stage->getParameterNames();
            for (const auto& paramName : paramNames) {
                try {
                    auto paramValue = stage->getParameter(paramName);
                    if (std::holds_alternative<double>(paramValue)) {
                        double value = std::get<double>(paramValue);
                        if (std::isnan(value) || std::isinf(value)) {
                            violations.push_back("Stage " + stageName + " parameter " + paramName + " is invalid");
                        }
                    }
                } catch (const std::exception& e) {
                    violations.push_back("Stage " + stageName + " parameter " + paramName + " error: " + e.what());
                }
            }
        }
    }
    
    return violations;
}

bool AudioValidator::checkClipping(const AudioBuffer& audio) {
    for (double sample : audio) {
        if (std::abs(sample) >= 1.0) {
            return true;
        }
    }
    return false;
}

bool AudioValidator::checkDCOffset(const AudioBuffer& audio) {
    double sum = 0.0;
    for (double sample : audio) {
        sum += sample;
    }
    double dcOffset = sum / audio.size();
    return std::abs(dcOffset) > 0.001;
}

bool AudioValidator::checkSilence(const AudioBuffer& audio) {
    double rms = 0.0;
    for (double sample : audio) {
        rms += sample * sample;
    }
    rms = std::sqrt(rms / audio.size());
    return rms < 0.001;
}

bool AudioValidator::checkDenormals(const AudioBuffer& audio) {
    for (double sample : audio) {
        if (std::fpclassify(sample) == FP_SUBNORMAL) {
            return true;
        }
    }
    return false;
}

bool AudioValidator::checkParameterRanges(const DSPGraph& graph) {
    // This would check parameter ranges against defined constraints
    return true;
}

// SafetyMonitor implementation
bool SafetyMonitor::monitorSafety(const AudioBuffer& audio) {
    SafetyMetrics metrics = getSafetyMetrics(audio);
    return checkSafetyThresholds(metrics);
}

SafetyMonitor::SafetyMetrics SafetyMonitor::getSafetyMetrics(const AudioBuffer& audio) {
    return calculateSafetyMetrics(audio);
}

bool SafetyMonitor::checkSafetyThresholds(const SafetyMetrics& metrics) {
    return isWithinThresholds(metrics);
}

SafetyMonitor::SafetyMetrics SafetyMonitor::calculateSafetyMetrics(const AudioBuffer& audio) {
    SafetyMetrics metrics;
    
    // Calculate true peak
    double maxPeak = 0.0;
    for (double sample : audio) {
        maxPeak = std::max(maxPeak, std::abs(sample));
    }
    metrics.truePeak = 20.0 * std::log10(std::max(maxPeak, 1e-10));
    
    // Calculate RMS
    double sum = 0.0;
    for (double sample : audio) {
        sum += sample * sample;
    }
    double rms = std::sqrt(sum / audio.size());
    metrics.rms = 20.0 * std::log10(std::max(rms, 1e-10));
    
    // Calculate crest factor
    if (rms > 0.0) {
        metrics.crestFactor = 20.0 * std::log10(maxPeak / rms);
    } else {
        metrics.crestFactor = 0.0;
    }
    
    // Calculate DC offset
    double dcSum = 0.0;
    for (double sample : audio) {
        dcSum += sample;
    }
    metrics.dcOffset = 20.0 * std::log10(std::max(std::abs(dcSum / audio.size()), 1e-10));
    
    // Check for clipping
    metrics.clipping = false;
    for (double sample : audio) {
        if (std::abs(sample) >= 1.0) {
            metrics.clipping = true;
            break;
        }
    }
    
    // Check for denormals
    metrics.denormals = false;
    for (double sample : audio) {
        if (std::fpclassify(sample) == FP_SUBNORMAL) {
            metrics.denormals = true;
            break;
        }
    }
    
    // Calculate headroom
    metrics.headroom = 20.0 * std::log10(1.0 / std::max(maxPeak, 1e-10));
    
    return metrics;
}

bool SafetyMonitor::isWithinThresholds(const SafetyMetrics& metrics) {
    // Check safety thresholds
    if (metrics.clipping) return false;
    if (metrics.denormals) return false;
    if (metrics.truePeak > -0.1) return false;
    if (metrics.dcOffset > -60.0) return false;
    if (metrics.headroom < 3.0) return false;
    
    return true;
}

// AudioProtection implementation
void AudioProtection::applyProtection(DSPGraph& graph, const AudioConstraints& constraints) {
    applyBasicProtection(graph);
    applyAdvancedProtection(graph);
}

void AudioProtection::protectAgainstIssues(DSPGraph& graph) {
    applyBasicProtection(graph);
}

void AudioProtection::applyEmergencyProtection(AudioBuffer& audio) {
    applyEmergencyMeasures(audio);
}

bool AudioProtection::isProtected(const DSPGraph& graph) {
    // Check if graph has protection measures
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        if (stageName.find("limiter") != std::string::npos ||
            stageName.find("protection") != std::string::npos) {
            return true;
        }
    }
    return false;
}

void AudioProtection::applyBasicProtection(DSPGraph& graph) {
    // Apply basic protection measures
    GainStager::autoGainStage(graph);
    FeedbackGuard::applyFeedbackProtection(graph);
}

void AudioProtection::applyAdvancedProtection(DSPGraph& graph) {
    // Apply advanced protection measures
    AntiChaosSystem::preventChaos(graph);
    HeadroomManager::manageHeadroom(graph);
}

void AudioProtection::applyEmergencyMeasures(AudioBuffer& audio) {
    // Apply emergency protection
    TruePeakLimiter::hardLimit(audio, -0.1);
    TruePeakLimiter::limitTruePeak(audio, -1.0);
}

} // namespace aiaudio