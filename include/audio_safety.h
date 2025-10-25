#pragma once

#include "core_types.h"
#include "dsp_ir.h"
#include <vector>
#include <memory>
#include <map>

namespace aiaudio {

// Audio Safety, Headroom & Anti-Chaos Systems

class GainStager {
public:
    // Stage gain through processing chain
    static void stageGain(DSPGraph& graph, double targetGain = -18.0);
    
    // Check for gain staging issues
    static std::vector<std::string> checkGainStaging(const DSPGraph& graph);
    
    // Auto-gain stage a graph
    static void autoGainStage(DSPGraph& graph);
    
private:
    static double calculateStageGain(const DSPStage& stage);
    static void adjustStageGain(DSPStage& stage, double targetGain);
};

class TruePeakLimiter {
public:
    // Apply true peak limiting
    static void limitTruePeak(AudioBuffer& audio, double limitDb = -1.0);
    
    // Check true peak level
    static double getTruePeak(const AudioBuffer& audio);
    
    // Apply soft limiting
    static void softLimit(AudioBuffer& audio, double threshold = -3.0, double ratio = 4.0);
    
    // Apply hard limiting
    static void hardLimit(AudioBuffer& audio, double limit = -0.1);
    
private:
    static double calculateTruePeak(const AudioBuffer& audio);
    static void applySoftLimiter(AudioBuffer& audio, double threshold, double ratio);
    static void applyHardLimiter(AudioBuffer& audio, double limit);
};

class FeedbackGuard {
public:
    // Check for feedback stability
    static bool checkFeedbackStability(const DSPGraph& graph);
    
    // Calculate loop gain
    static double calculateLoopGain(const DSPGraph& graph);
    
    // Apply feedback protection
    static void applyFeedbackProtection(DSPGraph& graph);
    
    // Check root locus stability
    static bool checkRootLocusStability(const DSPGraph& graph);
    
private:
    static double findFeedbackLoops(const DSPGraph& graph);
    static void addFeedbackProtection(DSPGraph& graph);
    static bool isStable(double gain);
};

class AntiChaosSystem {
public:
    // Prevent chaotic behavior
    static void preventChaos(DSPGraph& graph);
    
    // Check for chaos indicators
    static std::vector<std::string> checkChaosIndicators(const DSPGraph& graph);
    
    // Apply chaos prevention measures
    static void applyChaosPrevention(DSPGraph& graph);
    
    // Monitor for runaway parameters
    static bool monitorRunawayParameters(const DSPGraph& graph);
    
private:
    static bool detectChaos(const AudioBuffer& audio);
    static void applyChaosPreventionMeasures(DSPGraph& graph);
    static bool checkParameterBounds(const DSPGraph& graph);
};

class HeadroomManager {
public:
    // Manage headroom throughout processing
    static void manageHeadroom(DSPGraph& graph, double targetHeadroom = 6.0);
    
    // Check headroom levels
    static double checkHeadroom(const AudioBuffer& audio);
    
    // Apply headroom compensation
    static void applyHeadroomCompensation(AudioBuffer& audio, double targetHeadroom);
    
    // Monitor headroom in real-time
    static bool monitorHeadroom(const AudioBuffer& audio, double minHeadroom = 3.0);
    
private:
    static double calculateHeadroom(const AudioBuffer& audio);
    static void adjustHeadroom(AudioBuffer& audio, double targetHeadroom);
};

class AudioValidator {
public:
    // Validate audio for safety
    static std::vector<std::string> validateAudio(const AudioBuffer& audio);
    
    // Check for common audio issues
    static std::vector<std::string> checkAudioIssues(const AudioBuffer& audio);
    
    // Validate DSP graph
    static std::vector<std::string> validateGraph(const DSPGraph& graph);
    
    // Check for parameter violations
    static std::vector<std::string> checkParameterViolations(const DSPGraph& graph);
    
private:
    static bool checkClipping(const AudioBuffer& audio);
    static bool checkDCOffset(const AudioBuffer& audio);
    static bool checkSilence(const AudioBuffer& audio);
    static bool checkDenormals(const AudioBuffer& audio);
    static bool checkParameterRanges(const DSPGraph& graph);
};

class SafetyMonitor {
public:
    // Monitor audio safety in real-time
    static bool monitorSafety(const AudioBuffer& audio);
    
    // Get safety metrics
    struct SafetyMetrics {
        double truePeak;
        double rms;
        double crestFactor;
        double dcOffset;
        bool clipping;
        bool denormals;
        double headroom;
    };
    static SafetyMetrics getSafetyMetrics(const AudioBuffer& audio);
    
    // Check safety thresholds
    static bool checkSafetyThresholds(const SafetyMetrics& metrics);
    
private:
    static SafetyMetrics calculateSafetyMetrics(const AudioBuffer& audio);
    static bool isWithinThresholds(const SafetyMetrics& metrics);
};

class AudioProtection {
public:
    // Apply comprehensive audio protection
    static void applyProtection(DSPGraph& graph, const AudioConstraints& constraints);
    
    // Protect against common issues
    static void protectAgainstIssues(DSPGraph& graph);
    
    // Apply emergency protection
    static void applyEmergencyProtection(AudioBuffer& audio);
    
    // Check protection status
    static bool isProtected(const DSPGraph& graph);
    
private:
    static void applyBasicProtection(DSPGraph& graph);
    static void applyAdvancedProtection(DSPGraph& graph);
    static void applyEmergencyMeasures(AudioBuffer& audio);
};

} // namespace aiaudio