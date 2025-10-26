#include "preset_parser.h"
#include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace aiaudio {

PresetParser::PresetParser() {
    // Initialize default values
    loadQualityWeights();
}

PresetData PresetParser::parseFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    return parseFromJson(content);
}

PresetData PresetParser::parseFromJson(const std::string& jsonContent) {
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(jsonContent, root)) {
        throw std::runtime_error("Invalid JSON: " + reader.getFormattedErrorMessages());
    }
    
    return parseFromJsonValue(root);
}

PresetData PresetParser::parseFromJsonValue(const Json::Value& root) {
    PresetData preset;
    
    // Basic fields
    preset.name = root.get("name", "Unknown").asString();
    preset.category = root.get("category", "electronic").asString();
    preset.description = root.get("description", "").asString();
    preset.role = stringToRole(root.get("role", "UNKNOWN").asString());
    
    // Parameters
    if (root.isMember("parameters")) {
        preset.parameters = parseParameters(root["parameters"]);
    }
    
    // Validate the preset
    validatePreset(preset);
    
    return preset;
}

PresetParameters PresetParser::parseParameters(const Json::Value& params) {
    PresetParameters parameters;
    
    // Oscillator
    if (params.isMember("oscillator")) {
        const Json::Value& osc = params["oscillator"];
        parameters.oscillator.frequency = osc.get("frequency", 440.0).asDouble();
        parameters.oscillator.waveform = osc.get("waveform", "sine").asString();
        parameters.oscillator.detune = osc.get("detune", 0.0).asDouble();
        parameters.oscillator.phase = osc.get("phase", 0.0).asDouble();
    }
    
    // Envelope
    if (params.isMember("envelope")) {
        const Json::Value& env = params["envelope"];
        parameters.envelope.attack = env.get("attack", 0.1).asDouble();
        parameters.envelope.decay = env.get("decay", 0.1).asDouble();
        parameters.envelope.sustain = env.get("sustain", 0.7).asDouble();
        parameters.envelope.release = env.get("release", 0.5).asDouble();
    }
    
    // Filter
    if (params.isMember("filter")) {
        const Json::Value& filt = params["filter"];
        parameters.filter.type = filt.get("type", "lowpass").asString();
        parameters.filter.cutoff = filt.get("cutoff", 1000.0).asDouble();
        parameters.filter.resonance = filt.get("resonance", 1.0).asDouble();
        parameters.filter.slope = filt.get("slope", 12).asInt();
    }
    
    // Effects
    if (params.isMember("effects") && params["effects"].isArray()) {
        const Json::Value& effects = params["effects"];
        for (const auto& effect : effects) {
            EffectParameters effectParams;
            effectParams.type = effect.get("type", "").asString();
            effectParams.amount = effect.get("amount", 0.5).asDouble();
            
            if (effect.isMember("parameters")) {
                const Json::Value& effectParamsJson = effect["parameters"];
                for (const auto& key : effectParamsJson.getMemberNames()) {
                    effectParams.parameters[key] = effectParamsJson[key].asString();
                }
            }
            
            parameters.effects.push_back(effectParams);
        }
    }
    
    // Modulation
    if (params.isMember("modulation")) {
        const Json::Value& mod = params["modulation"];
        parameters.modulation.lfoRate = mod.get("lfo_rate", 1.0).asDouble();
        parameters.modulation.lfoDepth = mod.get("lfo_depth", 0.1).asDouble();
        parameters.modulation.lfoTarget = mod.get("lfo_target", "frequency").asString();
    }
    
    // Performance
    if (params.isMember("performance")) {
        const Json::Value& perf = params["performance"];
        parameters.performance.polyphony = perf.get("polyphony", 8).asInt();
        parameters.performance.voiceStealing = perf.get("voice_stealing", true).asBool();
        parameters.performance.portamento = perf.get("portamento", 0.0).asDouble();
    }
    
    // Quality
    if (params.isMember("quality")) {
        const Json::Value& qual = params["quality"];
        parameters.quality.sampleRate = qual.get("sample_rate", 44100).asInt();
        parameters.quality.bitDepth = qual.get("bit_depth", 24).asInt();
        parameters.quality.oversampling = qual.get("oversampling", 1).asInt();
    }
    
    // Metadata
    if (params.isMember("metadata")) {
        const Json::Value& meta = params["metadata"];
        parameters.metadata.author = meta.get("author", "").asString();
        parameters.metadata.version = meta.get("version", "1.0.0").asString();
        
        if (meta.isMember("tags") && meta["tags"].isArray()) {
            for (const auto& tag : meta["tags"]) {
                parameters.metadata.tags.push_back(tag.asString());
            }
        }
        
        parameters.metadata.created = meta.get("created", "").asString();
        parameters.metadata.modified = meta.get("modified", "").asString();
    }
    
    return parameters;
}

Role PresetParser::stringToRole(const std::string& roleStr) {
    std::string upper = roleStr;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "PAD") return Role::PAD;
    if (upper == "BASS") return Role::BASS;
    if (upper == "LEAD") return Role::LEAD;
    if (upper == "FX") return Role::FX;
    if (upper == "TEXTURE") return Role::TEXTURE;
    if (upper == "ARP") return Role::ARP;
    if (upper == "DRONE") return Role::DRONE;
    if (upper == "RHYTHM") return Role::RHYTHM;
    if (upper == "BELL") return Role::BELL;
    if (upper == "CHORD") return Role::CHORD;
    if (upper == "PLUCK") return Role::PLUCK;
    
    return Role::UNKNOWN;
}

std::string PresetParser::roleToString(Role role) {
    switch (role) {
        case Role::PAD: return "PAD";
        case Role::BASS: return "BASS";
        case Role::LEAD: return "LEAD";
        case Role::FX: return "FX";
        case Role::TEXTURE: return "TEXTURE";
        case Role::ARP: return "ARP";
        case Role::DRONE: return "DRONE";
        case Role::RHYTHM: return "RHYTHM";
        case Role::BELL: return "BELL";
        case Role::CHORD: return "CHORD";
        case Role::PLUCK: return "PLUCK";
        default: return "UNKNOWN";
    }
}

void PresetParser::validatePreset(const PresetData& preset) {
    std::vector<std::string> errors;
    
    // Validate basic fields
    if (preset.name.empty()) {
        errors.push_back("Preset name cannot be empty");
    }
    
    if (preset.category.empty()) {
        errors.push_back("Preset category cannot be empty");
    }
    
    // Validate oscillator parameters
    if (preset.parameters.oscillator.frequency < 20.0 || preset.parameters.oscillator.frequency > 20000.0) {
        errors.push_back("Oscillator frequency must be between 20Hz and 20kHz");
    }
    
    // Validate envelope parameters
    if (preset.parameters.envelope.attack < 0.0 || preset.parameters.envelope.attack > 10.0) {
        errors.push_back("Attack time must be between 0 and 10 seconds");
    }
    
    if (preset.parameters.envelope.sustain < 0.0 || preset.parameters.envelope.sustain > 1.0) {
        errors.push_back("Sustain level must be between 0 and 1");
    }
    
    // Validate filter parameters
    if (preset.parameters.filter.cutoff < 20.0 || preset.parameters.filter.cutoff > 20000.0) {
        errors.push_back("Filter cutoff must be between 20Hz and 20kHz");
    }
    
    if (preset.parameters.filter.resonance < 0.1 || preset.parameters.filter.resonance > 10.0) {
        errors.push_back("Filter resonance must be between 0.1 and 10.0");
    }
    
    // Validate quality parameters
    if (preset.parameters.quality.sampleRate != 44100 && 
        preset.parameters.quality.sampleRate != 48000 &&
        preset.parameters.quality.sampleRate != 88200 &&
        preset.parameters.quality.sampleRate != 96000) {
        errors.push_back("Sample rate must be 44100, 48000, 88200, or 96000 Hz");
    }
    
    if (preset.parameters.quality.bitDepth != 16 && 
        preset.parameters.quality.bitDepth != 24 &&
        preset.parameters.quality.bitDepth != 32) {
        errors.push_back("Bit depth must be 16, 24, or 32 bits");
    }
    
    if (!errors.empty()) {
        std::string errorMsg = "Preset validation failed:\n";
        for (const auto& error : errors) {
            errorMsg += "  - " + error + "\n";
        }
        throw std::runtime_error(errorMsg);
    }
}

Json::Value PresetParser::presetToJson(const PresetData& preset) {
    Json::Value root;
    
    // Basic fields
    root["name"] = preset.name;
    root["category"] = preset.category;
    root["description"] = preset.description;
    root["role"] = roleToString(preset.role);
    
    // Parameters
    Json::Value params;
    
    // Oscillator
    Json::Value oscillator;
    oscillator["frequency"] = preset.parameters.oscillator.frequency;
    oscillator["waveform"] = preset.parameters.oscillator.waveform;
    oscillator["detune"] = preset.parameters.oscillator.detune;
    oscillator["phase"] = preset.parameters.oscillator.phase;
    params["oscillator"] = oscillator;
    
    // Envelope
    Json::Value envelope;
    envelope["attack"] = preset.parameters.envelope.attack;
    envelope["decay"] = preset.parameters.envelope.decay;
    envelope["sustain"] = preset.parameters.envelope.sustain;
    envelope["release"] = preset.parameters.envelope.release;
    params["envelope"] = envelope;
    
    // Filter
    Json::Value filter;
    filter["type"] = preset.parameters.filter.type;
    filter["cutoff"] = preset.parameters.filter.cutoff;
    filter["resonance"] = preset.parameters.filter.resonance;
    filter["slope"] = preset.parameters.filter.slope;
    params["filter"] = filter;
    
    // Effects
    Json::Value effects(Json::arrayValue);
    for (const auto& effect : preset.parameters.effects) {
        Json::Value effectJson;
        effectJson["type"] = effect.type;
        effectJson["amount"] = effect.amount;
        
        Json::Value effectParams;
        for (const auto& param : effect.parameters) {
            effectParams[param.first] = param.second;
        }
        effectJson["parameters"] = effectParams;
        
        effects.append(effectJson);
    }
    params["effects"] = effects;
    
    // Modulation
    Json::Value modulation;
    modulation["lfo_rate"] = preset.parameters.modulation.lfoRate;
    modulation["lfo_depth"] = preset.parameters.modulation.lfoDepth;
    modulation["lfo_target"] = preset.parameters.modulation.lfoTarget;
    params["modulation"] = modulation;
    
    // Performance
    Json::Value performance;
    performance["polyphony"] = preset.parameters.performance.polyphony;
    performance["voice_stealing"] = preset.parameters.performance.voiceStealing;
    performance["portamento"] = preset.parameters.performance.portamento;
    params["performance"] = performance;
    
    // Quality
    Json::Value quality;
    quality["sample_rate"] = preset.parameters.quality.sampleRate;
    quality["bit_depth"] = preset.parameters.quality.bitDepth;
    quality["oversampling"] = preset.parameters.quality.oversampling;
    params["quality"] = quality;
    
    // Metadata
    Json::Value metadata;
    metadata["author"] = preset.parameters.metadata.author;
    metadata["version"] = preset.parameters.metadata.version;
    
    Json::Value tags(Json::arrayValue);
    for (const auto& tag : preset.parameters.metadata.tags) {
        tags.append(tag);
    }
    metadata["tags"] = tags;
    
    metadata["created"] = preset.parameters.metadata.created;
    metadata["modified"] = preset.parameters.metadata.modified;
    params["metadata"] = metadata;
    
    root["parameters"] = params;
    
    return root;
}

void PresetParser::loadQualityWeights() {
    try {
        YAML::Node config = YAML::LoadFile("config/quality_weights.json");
        // Parse quality weights configuration
        // This would be implemented based on the actual quality weights structure
    } catch (const std::exception& e) {
        // Use default weights if file not found
        qualityWeights_.overallWeights.semanticMatch = 0.3;
        qualityWeights_.overallWeights.mixReadiness = 0.25;
        qualityWeights_.overallWeights.perceptualQuality = 0.25;
        qualityWeights_.overallWeights.stability = 0.2;
    }
}

QualityWeights PresetParser::getQualityWeights() const {
    return qualityWeights_;
}

void PresetParser::setQualityWeights(const QualityWeights& weights) {
    qualityWeights_ = weights;
}

} // namespace aiaudio