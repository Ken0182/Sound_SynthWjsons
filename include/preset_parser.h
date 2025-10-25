#pragma once

#include "core_types.h"
#include <json/json.h>
#include <string>
#include <vector>
#include <map>

namespace aiaudio {

// Preset data structures matching the schema
struct OscillatorParameters {
    double frequency = 440.0;
    std::string waveform = "sine";
    double detune = 0.0;
    double phase = 0.0;
};

struct EnvelopeParameters {
    double attack = 0.1;
    double decay = 0.1;
    double sustain = 0.7;
    double release = 0.5;
};

struct FilterParameters {
    std::string type = "lowpass";
    double cutoff = 1000.0;
    double resonance = 1.0;
    int slope = 12;
};

struct EffectParameters {
    std::string type;
    double amount = 0.5;
    std::map<std::string, std::string> parameters;
};

struct ModulationParameters {
    double lfoRate = 1.0;
    double lfoDepth = 0.1;
    std::string lfoTarget = "frequency";
};

struct PerformanceParameters {
    int polyphony = 8;
    bool voiceStealing = true;
    double portamento = 0.0;
};

struct QualityParameters {
    int sampleRate = 44100;
    int bitDepth = 24;
    int oversampling = 1;
};

struct PresetMetadata {
    std::string author;
    std::string version = "1.0.0";
    std::vector<std::string> tags;
    std::string created;
    std::string modified;
};

struct PresetParameters {
    OscillatorParameters oscillator;
    EnvelopeParameters envelope;
    FilterParameters filter;
    std::vector<EffectParameters> effects;
    ModulationParameters modulation;
    PerformanceParameters performance;
    QualityParameters quality;
    PresetMetadata metadata;
};

struct PresetData {
    std::string name;
    std::string category;
    std::string description;
    Role role = Role::UNKNOWN;
    PresetParameters parameters;
};

// Quality weights structure
struct QualityWeights {
    struct OverallWeights {
        double semanticMatch = 0.3;
        double mixReadiness = 0.25;
        double perceptualQuality = 0.25;
        double stability = 0.2;
    } overallWeights;
    
    struct SemanticMatchWeights {
        double frequencyMatch = 0.4;
        double timbreMatch = 0.3;
        double rhythmMatch = 0.2;
        double harmonicMatch = 0.1;
    } semanticMatch;
    
    struct MixReadinessWeights {
        double frequencyBalance = 0.3;
        double dynamicRange = 0.25;
        double stereoWidth = 0.2;
        double frequencySeparation = 0.15;
        double transientHandling = 0.1;
    } mixReadiness;
    
    struct PerceptualQualityWeights {
        double clarity = 0.3;
        double warmth = 0.25;
        double brightness = 0.2;
        double depth = 0.15;
        double spatialPresence = 0.1;
    } perceptualQuality;
    
    struct StabilityWeights {
        double amplitudeStability = 0.4;
        double frequencyStability = 0.3;
        double phaseStability = 0.2;
        double noiseFloor = 0.1;
    } stability;
};

class PresetParser {
public:
    PresetParser();
    
    // Parse preset from file
    PresetData parseFromFile(const std::string& filePath);
    
    // Parse preset from JSON string
    PresetData parseFromJson(const std::string& jsonContent);
    
    // Parse preset from Json::Value
    PresetData parseFromJsonValue(const Json::Value& root);
    
    // Convert preset to JSON
    Json::Value presetToJson(const PresetData& preset);
    
    // Validate preset data
    void validatePreset(const PresetData& preset);
    
    // Role conversion utilities
    Role stringToRole(const std::string& roleStr);
    std::string roleToString(Role role);
    
    // Quality weights management
    QualityWeights getQualityWeights() const;
    void setQualityWeights(const QualityWeights& weights);
    
private:
    // Parse parameters from JSON
    PresetParameters parseParameters(const Json::Value& params);
    
    // Load quality weights from configuration
    void loadQualityWeights();
    
    QualityWeights qualityWeights_;
};

} // namespace aiaudio