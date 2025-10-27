#include "dsp_ir.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <sstream>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace aiaudio {

// OscillatorStage implementation
OscillatorStage::OscillatorStage() 
    : frequency_(440.0, 20.0, 20000.0, "frequency"),
      amplitude_(0.5, 0.0, 1.0, "amplitude"),
      phase_(0.0, 0.0, 1.0, "phase") {
}

void OscillatorStage::process(const AudioBuffer& input, AudioBuffer& output) {
    output.resize(input.size());
    
    double phaseIncrement = 2.0 * M_PI * frequency_.value / sampleRate_;
    
    for (size_t i = 0; i < input.size(); ++i) {
        double sample = 0.0;
        
        if (waveType_ == "sine") {
            sample = std::sin(phaseAccumulator_ + phase_.value * 2.0 * M_PI);
        } else if (waveType_ == "saw") {
            sample = 2.0 * (phaseAccumulator_ / (2.0 * M_PI)) - 1.0;
        } else if (waveType_ == "square") {
            sample = (phaseAccumulator_ < M_PI) ? 1.0 : -1.0;
        } else if (waveType_ == "triangle") {
            if (phaseAccumulator_ < M_PI) {
                sample = 2.0 * phaseAccumulator_ / M_PI - 1.0;
            } else {
                sample = 3.0 - 2.0 * phaseAccumulator_ / M_PI;
            }
        }
        
        output[i] = sample * amplitude_.value + input[i];
        phaseAccumulator_ += phaseIncrement;
        
        // Wrap phase
        while (phaseAccumulator_ >= 2.0 * M_PI) {
            phaseAccumulator_ -= 2.0 * M_PI;
        }
    }
}

void OscillatorStage::setParameter(const std::string& name, const ParamValue& value) {
    if (name == "frequency") {
        frequency_.setValue(std::get<double>(value));
    } else if (name == "amplitude") {
        amplitude_.setValue(std::get<double>(value));
    } else if (name == "phase") {
        phase_.setValue(std::get<double>(value));
    } else if (name == "waveType") {
        waveType_ = std::get<std::string>(value);
    }
}

ParamValue OscillatorStage::getParameter(const std::string& name) const {
    if (name == "frequency") return frequency_.value;
    if (name == "amplitude") return amplitude_.value;
    if (name == "phase") return phase_.value;
    if (name == "waveType") return waveType_;
    return 0.0;
}

std::vector<std::string> OscillatorStage::getParameterNames() const {
    return {"frequency", "amplitude", "phase", "waveType"};
}

void OscillatorStage::reset() {
    phaseAccumulator_ = 0.0;
}

std::string OscillatorStage::getDescription() const {
    return "Oscillator: " + waveType_ + " wave at " + std::to_string(frequency_.value) + " Hz";
}

// FilterStage implementation
FilterStage::FilterStage() 
    : cutoff_(1000.0, 20.0, 20000.0, "cutoff"),
      resonance_(0.1, 0.0, 0.99, "resonance") {
}

void FilterStage::process(const AudioBuffer& input, AudioBuffer& output) {
    output.resize(input.size());
    
    // Simple biquad filter implementation
    double w = 2.0 * M_PI * cutoff_.value / 44100.0;
    double cosw = std::cos(w);
    double sinw = std::sin(w);
    double alpha = sinw / (2.0 * resonance_.value);
    
    double b0 = (1.0 - cosw) / 2.0;
    double b1 = 1.0 - cosw;
    double b2 = (1.0 - cosw) / 2.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosw;
    double a2 = 1.0 - alpha;
    
    // Normalize coefficients
    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
    
    for (size_t i = 0; i < input.size(); ++i) {
        double sample = b0 * input[i] + b1 * x1_ + b2 * x2_ - a1 * y1_ - a2 * y2_;
        
        // Update state
        x2_ = x1_;
        x1_ = input[i];
        y2_ = y1_;
        y1_ = sample;
        
        output[i] = sample;
    }
}

void FilterStage::setParameter(const std::string& name, const ParamValue& value) {
    if (name == "cutoff") {
        cutoff_.setValue(std::get<double>(value));
    } else if (name == "resonance") {
        resonance_.setValue(std::get<double>(value));
    } else if (name == "filterType") {
        filterType_ = std::get<std::string>(value);
    }
}

ParamValue FilterStage::getParameter(const std::string& name) const {
    if (name == "cutoff") return cutoff_.value;
    if (name == "resonance") return resonance_.value;
    if (name == "filterType") return filterType_;
    return 0.0;
}

std::vector<std::string> FilterStage::getParameterNames() const {
    return {"cutoff", "resonance", "filterType"};
}

void FilterStage::reset() {
    x1_ = x2_ = y1_ = y2_ = 0.0;
}

std::string FilterStage::getDescription() const {
    return "Filter: " + filterType_ + " at " + std::to_string(cutoff_.value) + " Hz";
}

// EnvelopeStage implementation
EnvelopeStage::EnvelopeStage() 
    : attack_(0.01, 0.001, 2.0, "attack"),
      decay_(0.1, 0.001, 2.0, "decay"),
      sustain_(0.7, 0.0, 1.0, "sustain"),
      release_(0.5, 0.001, 5.0, "release") {
}

void EnvelopeStage::process(const AudioBuffer& input, AudioBuffer& output) {
    output.resize(input.size());
    
    for (size_t i = 0; i < input.size(); ++i) {
        // Simple envelope state machine
        if (input[i] > 0.001 && state_ == EnvState::IDLE) {
            // Gate on
            state_ = EnvState::ATTACK;
            currentLevel_ = 0.0;
            targetLevel_ = 1.0;
            rate_ = 1.0 / (attack_.value * 44100.0);
            sampleCount_ = 0;
        } else if (input[i] <= 0.001 && state_ != EnvState::IDLE && state_ != EnvState::RELEASE) {
            // Gate off
            state_ = EnvState::RELEASE;
            targetLevel_ = 0.0;
            rate_ = 1.0 / (release_.value * 44100.0);
            sampleCount_ = 0;
        }
        
        // Update envelope level
        switch (state_) {
            case EnvState::ATTACK:
                currentLevel_ += rate_;
                if (currentLevel_ >= 1.0) {
                    currentLevel_ = 1.0;
                    state_ = EnvState::DECAY;
                    targetLevel_ = sustain_.value;
                    rate_ = (1.0 - sustain_.value) / (decay_.value * 44100.0);
                    sampleCount_ = 0;
                }
                break;
                
            case EnvState::DECAY:
                currentLevel_ -= rate_;
                if (currentLevel_ <= sustain_.value) {
                    currentLevel_ = sustain_.value;
                    state_ = EnvState::SUSTAIN;
                }
                break;
                
            case EnvState::SUSTAIN:
                currentLevel_ = sustain_.value;
                break;
                
            case EnvState::RELEASE:
                currentLevel_ -= rate_;
                if (currentLevel_ <= 0.0) {
                    currentLevel_ = 0.0;
                    state_ = EnvState::IDLE;
                }
                break;
                
            case EnvState::IDLE:
                currentLevel_ = 0.0;
                break;
        }
        
        output[i] = input[i] * currentLevel_;
        sampleCount_++;
    }
}

void EnvelopeStage::setParameter(const std::string& name, const ParamValue& value) {
    if (name == "attack") {
        attack_.setValue(std::get<double>(value));
    } else if (name == "decay") {
        decay_.setValue(std::get<double>(value));
    } else if (name == "sustain") {
        sustain_.setValue(std::get<double>(value));
    } else if (name == "release") {
        release_.setValue(std::get<double>(value));
    }
}

ParamValue EnvelopeStage::getParameter(const std::string& name) const {
    if (name == "attack") return attack_.value;
    if (name == "decay") return decay_.value;
    if (name == "sustain") return sustain_.value;
    if (name == "release") return release_.value;
    return 0.0;
}

std::vector<std::string> EnvelopeStage::getParameterNames() const {
    return {"attack", "decay", "sustain", "release"};
}

void EnvelopeStage::reset() {
    state_ = EnvState::IDLE;
    currentLevel_ = 0.0;
    targetLevel_ = 0.0;
    rate_ = 0.0;
    sampleCount_ = 0;
}

std::string EnvelopeStage::getDescription() const {
    return "Envelope: A=" + std::to_string(attack_.value) + 
           "s D=" + std::to_string(decay_.value) + 
           "s S=" + std::to_string(sustain_.value) + 
           " R=" + std::to_string(release_.value) + "s";
}

// LFOStage implementation
LFOStage::LFOStage() 
    : rate_(1.0, 0.01, 20.0, "rate"),
      depth_(0.5, 0.0, 1.0, "depth") {
}

void LFOStage::process(const AudioBuffer& input, AudioBuffer& output) {
    output.resize(input.size());
    
    double phaseIncrement = 2.0 * M_PI * rate_.value / sampleRate_;
    
    for (size_t i = 0; i < input.size(); ++i) {
        double lfoValue = 0.0;
        
        if (waveType_ == "sine") {
            lfoValue = std::sin(phase_);
        } else if (waveType_ == "saw") {
            lfoValue = 2.0 * (phase_ / (2.0 * M_PI)) - 1.0;
        } else if (waveType_ == "square") {
            lfoValue = (phase_ < M_PI) ? 1.0 : -1.0;
        } else if (waveType_ == "triangle") {
            if (phase_ < M_PI) {
                lfoValue = 2.0 * phase_ / M_PI - 1.0;
            } else {
                lfoValue = 3.0 - 2.0 * phase_ / M_PI;
            }
        }
        
        // Scale by depth and center around 0
        lfoValue = lfoValue * depth_.value;
        
        output[i] = input[i] + lfoValue;
        phase_ += phaseIncrement;
        
        // Wrap phase
        while (phase_ >= 2.0 * M_PI) {
            phase_ -= 2.0 * M_PI;
        }
    }
}

void LFOStage::setParameter(const std::string& name, const ParamValue& value) {
    if (name == "rate") {
        rate_.setValue(std::get<double>(value));
    } else if (name == "depth") {
        depth_.setValue(std::get<double>(value));
    } else if (name == "waveType") {
        waveType_ = std::get<std::string>(value);
    }
}

ParamValue LFOStage::getParameter(const std::string& name) const {
    if (name == "rate") return rate_.value;
    if (name == "depth") return depth_.value;
    if (name == "waveType") return waveType_;
    return 0.0;
}

std::vector<std::string> LFOStage::getParameterNames() const {
    return {"rate", "depth", "waveType"};
}

void LFOStage::reset() {
    phase_ = 0.0;
}

std::string LFOStage::getDescription() const {
    return "LFO: " + waveType_ + " at " + std::to_string(rate_.value) + " Hz, depth " + std::to_string(depth_.value);
}

// DSPGraph implementation
void DSPGraph::addStage(const std::string& name, std::unique_ptr<DSPStage> stage) {
    stages_[name] = std::move(stage);
}

void DSPGraph::removeStage(const std::string& name) {
    stages_.erase(name);
    
    // Remove connections involving this stage
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                      [&name](const Connection& conn) {
                          return conn.source == name || conn.destination == name;
                      }),
        connections_.end()
    );
}

void DSPGraph::addConnection(const Connection& connection) {
    connections_.push_back(connection);
}

void DSPGraph::removeConnection(const std::string& source, const std::string& destination) {
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                      [&source, &destination](const Connection& conn) {
                          return conn.source == source && conn.destination == destination;
                      }),
        connections_.end()
    );
}

void DSPGraph::process(const AudioBuffer& input, AudioBuffer& output) {
    if (stages_.empty()) {
        output = input;
        return;
    }
    
    // Get topological order
    auto order = getTopologicalOrder();
    if (order.empty()) {
        output = input;
        return;
    }
    
    // Process stages in order
    AudioBuffer currentInput = input;
    AudioBuffer tempBuffer;
    
    for (const auto& stageName : order) {
        auto it = stages_.find(stageName);
        if (it != stages_.end()) {
            it->second->process(currentInput, tempBuffer);
            currentInput = tempBuffer;
        }
    }
    
    output = currentInput;
}

void DSPGraph::reset() {
    for (auto& [name, stage] : stages_) {
        stage->reset();
    }
}

bool DSPGraph::hasCycles() const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recStack;
    
    for (const auto& [name, stage] : stages_) {
        if (visited.find(name) == visited.end()) {
            if (hasCycleDFS(name, visited, recStack)) {
                return true;
            }
        }
    }
    
    return false;
}

bool DSPGraph::isConnected() const {
    if (stages_.empty()) return true;
    
    std::unordered_set<std::string> visited;
    std::queue<std::string> queue;
    
    // Start from first stage
    auto firstStage = stages_.begin();
    queue.push(firstStage->first);
    visited.insert(firstStage->first);
    
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        
        // Add connected stages
        for (const auto& conn : connections_) {
            if (conn.source == current && visited.find(conn.destination) == visited.end()) {
                visited.insert(conn.destination);
                queue.push(conn.destination);
            }
            if (conn.destination == current && visited.find(conn.source) == visited.end()) {
                visited.insert(conn.source);
                queue.push(conn.source);
            }
        }
    }
    
    return visited.size() == stages_.size();
}

std::vector<std::string> DSPGraph::getTopologicalOrder() const {
    std::vector<std::string> result;
    std::unordered_set<std::string> visited;
    
    for (const auto& [name, stage] : stages_) {
        if (visited.find(name) == visited.end()) {
            topologicalSortDFS(name, visited, result);
        }
    }
    
    std::reverse(result.begin(), result.end());
    return result;
}

double DSPGraph::getTotalGain() const {
    double totalGain = 1.0;
    
    for (const auto& [name, stage] : stages_) {
        // Estimate gain for each stage (simplified)
        if (stage->getType() == StageType::OSCILLATOR) {
            auto amp = stage->getParameter("amplitude");
            if (std::holds_alternative<double>(amp)) {
                totalGain *= std::get<double>(amp);
            }
        }
    }
    
    return totalGain;
}

DSPStage* DSPGraph::getStage(const std::string& name) {
    auto it = stages_.find(name);
    return (it != stages_.end()) ? it->second.get() : nullptr;
}

const DSPStage* DSPGraph::getStage(const std::string& name) const {
    auto it = stages_.find(name);
    return (it != stages_.end()) ? it->second.get() : nullptr;
}

std::vector<std::string> DSPGraph::getStageNames() const {
    std::vector<std::string> names;
    for (const auto& [name, stage] : stages_) {
        names.push_back(name);
    }
    return names;
}

std::vector<Connection> DSPGraph::getConnections() const {
    return connections_;
}

std::vector<std::string> DSPGraph::validate() const {
    std::vector<std::string> issues;
    
    // Check for cycles
    if (hasCycles()) {
        issues.push_back("Graph contains cycles");
    }
    
    // Check for disconnected components
    if (!isConnected()) {
        issues.push_back("Graph has disconnected components");
    }
    
    // Check feedback stability
    if (getTotalGain() >= 1.0) {
        issues.push_back("Total gain >= 1.0, potential feedback instability");
    }
    
    // Check stage parameters
    for (const auto& [name, stage] : stages_) {
        auto paramNames = stage->getParameterNames();
        for (const auto& paramName : paramNames) {
            try {
                stage->getParameter(paramName);
            } catch (const std::exception& e) {
                issues.push_back("Stage " + name + " parameter " + paramName + ": " + e.what());
            }
        }
    }
    
    return issues;
}

bool DSPGraph::hasCycleDFS(const std::string& node, 
                          std::unordered_set<std::string>& visited,
                          std::unordered_set<std::string>& recStack) const {
    visited.insert(node);
    recStack.insert(node);
    
    // Check all outgoing connections
    for (const auto& conn : connections_) {
        if (conn.source == node) {
            if (visited.find(conn.destination) == visited.end()) {
                if (hasCycleDFS(conn.destination, visited, recStack)) {
                    return true;
                }
            } else if (recStack.find(conn.destination) != recStack.end()) {
                return true;
            }
        }
    }
    
    recStack.erase(node);
    return false;
}

void DSPGraph::topologicalSortDFS(const std::string& node,
                                 std::unordered_set<std::string>& visited,
                                 std::vector<std::string>& result) const {
    visited.insert(node);
    
    // Process all outgoing connections first
    for (const auto& conn : connections_) {
        if (conn.source == node && visited.find(conn.destination) == visited.end()) {
            topologicalSortDFS(conn.destination, visited, result);
        }
    }
    
    result.push_back(node);
}

// IRParser implementation
std::unique_ptr<DSPGraph> IRParser::parsePreset(const std::string& jsonData) {
    auto graph = std::make_unique<DSPGraph>();
    
    try {
        json root = json::parse(jsonData);
        
        // Parse stages
        if (root.contains("stages") && root["stages"].is_object()) {
            const auto& stages = root["stages"];
            for (auto it = stages.begin(); it != stages.end(); ++it) {
                const std::string& stageName = it.key();
                const auto& stageData = it.value();
                
                if (!stageData.contains("type")) {
                    throw AIAudioException("Stage " + stageName + " missing type field");
                }
                
                std::string type = stageData["type"].get<std::string>();
                
                ParamMap params;
                if (stageData.contains("parameters") && stageData["parameters"].is_object()) {
                    const auto& paramData = stageData["parameters"];
                    for (auto paramIt = paramData.begin(); paramIt != paramData.end(); ++paramIt) {
                        const std::string& paramName = paramIt.key();
                        const auto& paramValue = paramIt.value();
                        
                        if (paramValue.is_number_float()) {
                            params[paramName] = paramValue.get<double>();
                        } else if (paramValue.is_number_integer()) {
                            params[paramName] = static_cast<double>(paramValue.get<int>());
                        } else if (paramValue.is_string()) {
                            params[paramName] = paramValue.get<std::string>();
                        } else if (paramValue.is_boolean()) {
                            params[paramName] = paramValue.get<bool>();
                        }
                    }
                }
                
                auto stage = createStageFromJSON(type, params);
                graph->addStage(stageName, std::move(stage));
            }
        }
        
        // Parse connections
        if (root.contains("connections") && root["connections"].is_array()) {
            const auto& connections = root["connections"];
            for (const auto& conn : connections) {
                Connection connection;
                
                if (!conn.contains("source") || !conn.contains("destination")) {
                    throw AIAudioException("Connection missing source or destination");
                }
                
                connection.source = conn["source"].get<std::string>();
                connection.destination = conn["destination"].get<std::string>();
                
                if (conn.contains("parameter")) {
                    connection.parameter = conn["parameter"].get<std::string>();
                }
                if (conn.contains("amount")) {
                    connection.amount = conn["amount"].get<double>();
                }
                if (conn.contains("enabled")) {
                    connection.enabled = conn["enabled"].get<bool>();
                }
                
                graph->addConnection(connection);
            }
        }
        
    } catch (const json::parse_error& e) {
        throw AIAudioException("Failed to parse JSON: " + std::string(e.what()));
    } catch (const json::type_error& e) {
        throw AIAudioException("JSON type error: " + std::string(e.what()));
    }
    
    return graph;
}

std::unique_ptr<DSPStage> IRParser::createStageFromJSON(const std::string& type, 
                                                        const ParamMap& params) {
    if (type == "oscillator") {
        auto stage = std::make_unique<OscillatorStage>();
        for (const auto& [name, value] : params) {
            stage->setParameter(name, value);
        }
        return stage;
    } else if (type == "filter") {
        auto stage = std::make_unique<FilterStage>();
        for (const auto& [name, value] : params) {
            stage->setParameter(name, value);
        }
        return stage;
    } else if (type == "envelope") {
        auto stage = std::make_unique<EnvelopeStage>();
        for (const auto& [name, value] : params) {
            stage->setParameter(name, value);
        }
        return stage;
    } else if (type == "lfo") {
        auto stage = std::make_unique<LFOStage>();
        for (const auto& [name, value] : params) {
            stage->setParameter(name, value);
        }
        return stage;
    }
    
    throw AIAudioException("Unknown stage type: " + type);
}

std::vector<std::string> IRParser::validate(const DSPGraph& graph) {
    return graph.validate();
}

} // namespace aiaudio