#pragma once

#include "core_types.h"
#include "moo_optimization.h"
#include "dsp_ir.h"
#include "normalization.h"
#include "semantic_fusion.h"
#include "roles_policies.h"
#include "decision_heads.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace aiaudio {

// Main AI Audio Generation System
class AIAudioGenerator {
public:
    // Initialize the system
    AIAudioGenerator();
    
    // Generate audio from prompt
    struct GenerationRequest {
        std::string prompt;
        Role role = Role::UNKNOWN;
        MusicalContext context;
        AudioConstraints constraints;
        std::string presetPath = "";
        bool useSemanticSearch = true;
        bool applyPolicies = true;
        bool optimizeForMOO = true;
    };
    
    struct GenerationResult {
        AudioBuffer audio;
        Trace trace;
        double qualityScore;
        std::vector<std::string> warnings;
        std::string explanation;
    };
    
    // Main generation function
    GenerationResult generate(const GenerationRequest& request);
    
    // Load preset from JSON
    void loadPreset(const std::string& presetPath);
    
    // Save preset to JSON
    void savePreset(const std::string& presetPath, const DSPGraph& graph);
    
    // Get available presets
    std::vector<std::string> getAvailablePresets() const;
    
    // Set system configuration
    void setConfiguration(const std::map<std::string, std::string>& config);
    
    // Get system status
    struct SystemStatus {
        bool initialized;
        size_t loadedPresets;
        double cpuUsage;
        double memoryUsage;
        std::vector<std::string> activeFeatures;
    };
    SystemStatus getStatus() const;
    
private:
    // Core components
    std::unique_ptr<MOOOptimizer> mooOptimizer_;
    std::unique_ptr<IRParser> irParser_;
    std::unique_ptr<PresetNormalizer> normalizer_;
    std::unique_ptr<SemanticFusionEngine> semanticEngine_;
    std::unique_ptr<PolicyManager> policyManager_;
    std::unique_ptr<DecisionHeads> decisionHeads_;
    
    // System state
    std::map<std::string, std::unique_ptr<DSPGraph>> loadedPresets_;
    std::map<std::string, std::string> configuration_;
    bool initialized_ = false;
    
    // Generation pipeline
    DSPGraph createGraphFromPrompt(const GenerationRequest& request);
    DSPGraph applySemanticSearch(const std::string& prompt, Role role);
    const DSPGraph& applyDecisionHeads(const DSPGraph& graph, const GenerationRequest& request);
    const DSPGraph& applyPolicies(const DSPGraph& graph, Role role, const MusicalContext& context);
    AudioBuffer renderGraph(const DSPGraph& graph, size_t numSamples);
    Trace createTrace(const GenerationRequest& request, const DSPGraph& graph, const AudioBuffer& audio);
    
    // Quality assessment
    double assessQuality(const AudioBuffer& audio, const GenerationRequest& request);
    std::vector<std::string> checkWarnings(const AudioBuffer& audio, const AudioConstraints& constraints);
    std::string generateExplanation(const GenerationRequest& request, const DSPGraph& graph);
    
    // System initialization
    void initializeComponents();
    void loadDefaultPresets();
    void setupSemanticEngine();
    void setupPolicyManager();
    void setupDecisionHeads();
};

// Configuration manager
class ConfigurationManager {
public:
    // Load configuration from file
    void loadFromFile(const std::string& filePath);
    
    // Save configuration to file
    void saveToFile(const std::string& filePath) const;
    
    // Get configuration value
    std::string getValue(const std::string& key, const std::string& defaultValue = "") const;
    
    // Set configuration value
    void setValue(const std::string& key, const std::string& value);
    
    // Get all configuration
    std::map<std::string, std::string> getAllValues() const;
    
private:
    std::map<std::string, std::string> config_;
};

// Preset manager
class PresetManager {
public:
    // Load preset from file
    std::unique_ptr<DSPGraph> loadPreset(const std::string& filePath);
    
    // Save preset to file
    void savePreset(const DSPGraph& graph, const std::string& filePath);
    
    // Get preset metadata
    struct PresetMetadata {
        std::string name;
        std::string description;
        Role role;
        std::vector<std::string> tags;
        std::string author;
        std::string version;
        std::chrono::system_clock::time_point created;
        std::chrono::system_clock::time_point modified;
    };
    
    PresetMetadata getMetadata(const std::string& filePath) const;
    void setMetadata(const std::string& filePath, const PresetMetadata& metadata);
    
    // Search presets
    std::vector<std::string> searchPresets(const std::string& query, Role role = Role::UNKNOWN) const;
    
    // Get all presets
    std::vector<std::string> getAllPresets() const;
    
private:
    std::map<std::string, PresetMetadata> presetMetadata_;
};

// Audio renderer
class AudioRenderer {
public:
    // Render audio from graph
    AudioBuffer render(const DSPGraph& graph, size_t numSamples, double sampleRate = 44100.0);
    
    // Render with real-time constraints
    AudioBuffer renderRealtime(const DSPGraph& graph, size_t numSamples, 
                              double maxLatencyMs = 10.0);
    
    // Get rendering statistics
    struct RenderStats {
        double renderTime;
        double cpuUsage;
        size_t memoryUsed;
        bool realtimeSuccess;
    };
    RenderStats getLastRenderStats() const;
    
private:
    RenderStats lastStats_;
    double sampleRate_ = 44100.0;
    
    // Rendering helpers
    void processGraph(DSPGraph& graph, AudioBuffer& output, size_t numSamples);
    bool checkRealtimeConstraints(double renderTime, double maxLatencyMs);
};

// Quality assessor
class QualityAssessor {
public:
    // Assess audio quality
    double assessQuality(const AudioBuffer& audio, Role role, const AudioConstraints& constraints);
    
    // Get detailed quality metrics
    struct QualityMetrics {
        double overallScore;
        double semanticMatch;
        double mixReadiness;
        double perceptualQuality;
        double stability;
        std::vector<std::string> issues;
    };
    QualityMetrics getDetailedMetrics(const AudioBuffer& audio, Role role, 
                                     const AudioConstraints& constraints);
    
    // Compare two audio samples
    double compareAudio(const AudioBuffer& audio1, const AudioBuffer& audio2);
    
private:
    std::unique_ptr<MOOOptimizer> mooOptimizer_;
    
    // Quality assessment helpers
    double computeSemanticScore(const AudioBuffer& audio, Role role);
    double computeMixReadiness(const AudioBuffer& audio, const AudioConstraints& constraints);
    double computePerceptualQuality(const AudioBuffer& audio);
    double computeStability(const AudioBuffer& audio);
};

// System monitor
class SystemMonitor {
public:
    // Get system performance metrics
    struct PerformanceMetrics {
        double cpuUsage;
        double memoryUsage;
        double diskUsage;
        size_t activeThreads;
        double averageLatency;
        size_t totalRenders;
        size_t successfulRenders;
    };
    PerformanceMetrics getMetrics() const;
    
    // Start monitoring
    void startMonitoring();
    
    // Stop monitoring
    void stopMonitoring();
    
    // Get monitoring status
    bool isMonitoring() const;
    
private:
    bool monitoring_ = false;
    std::chrono::system_clock::time_point startTime_;
    size_t totalRenders_ = 0;
    size_t successfulRenders_ = 0;
    
    // Monitoring helpers
    double getCPUUsage() const;
    double getMemoryUsage() const;
    double getDiskUsage() const;
    size_t getActiveThreads() const;
};

} // namespace aiaudio