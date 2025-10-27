#pragma once

#include "core_types.h"
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace aiaudio {

// DSP Intermediate Representation (IR) System

// Ranged parameters with validation
template<typename T>
struct RangedParam {
    T value;
    T min;
    T max;
    std::string name;
    
    RangedParam(T val, T minVal, T maxVal, const std::string& paramName)
        : value(val), min(minVal), max(maxVal), name(paramName) {
        validate();
    }
    
    void setValue(T val) {
        value = val;
        validate();
    }
    
private:
    void validate() {
        if (value < min || value > max) {
            throw AIAudioException("Parameter " + name + " out of range: " + 
                                 std::to_string(value) + " not in [" + 
                                 std::to_string(min) + ", " + std::to_string(max) + "]");
        }
    }
};

// DSP Stage types
enum class StageType {
    OSCILLATOR,
    SAMPLER,
    WAVETABLE,
    SHAPER,
    FILTER,
    SPATIAL,
    EFFECT,
    METER,
    LIMITER,
    ENVELOPE,
    LFO,
    MACRO
};

// Parameter types
using ParamValue = std::variant<double, int, bool, std::string>;
using ParamMap = std::unordered_map<std::string, ParamValue>;

// Base stage interface
class DSPStage {
public:
    virtual ~DSPStage() = default;
    virtual StageType getType() const = 0;
    virtual void process(const AudioBuffer& input, AudioBuffer& output) = 0;
    virtual void setParameter(const std::string& name, const ParamValue& value) = 0;
    virtual ParamValue getParameter(const std::string& name) const = 0;
    virtual std::vector<std::string> getParameterNames() const = 0;
    virtual void reset() = 0;
    virtual std::string getDescription() const = 0;
};

// Specific stage implementations
class OscillatorStage : public DSPStage {
public:
    OscillatorStage();
    StageType getType() const override { return StageType::OSCILLATOR; }
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    RangedParam<double> frequency_{440.0, 20.0, 20000.0, "frequency"};
    RangedParam<double> amplitude_{0.5, 0.0, 1.0, "amplitude"};
    RangedParam<double> phase_{0.0, 0.0, 1.0, "phase"};
    std::string waveType_ = "sine";
    double phaseAccumulator_ = 0.0;
    double sampleRate_ = 44100.0;
};

class FilterStage : public DSPStage {
public:
    FilterStage();
    StageType getType() const override { return StageType::FILTER; }
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    RangedParam<double> cutoff_{1000.0, 20.0, 20000.0, "cutoff"};
    RangedParam<double> resonance_{0.1, 0.0, 0.99, "resonance"};
    std::string filterType_ = "lowpass";
    double x1_ = 0.0, x2_ = 0.0, y1_ = 0.0, y2_ = 0.0; // State variables
};

class EnvelopeStage : public DSPStage {
public:
    EnvelopeStage();
    StageType getType() const override { return StageType::ENVELOPE; }
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    RangedParam<double> attack_{0.01, 0.001, 2.0, "attack"};
    RangedParam<double> decay_{0.1, 0.001, 2.0, "decay"};
    RangedParam<double> sustain_{0.7, 0.0, 1.0, "sustain"};
    RangedParam<double> release_{0.5, 0.001, 5.0, "release"};
    
    enum class EnvState { ATTACK, DECAY, SUSTAIN, RELEASE, IDLE };
    EnvState state_ = EnvState::IDLE;
    double currentLevel_ = 0.0;
    double targetLevel_ = 0.0;
    double rate_ = 0.0;
    size_t sampleCount_ = 0;
};

class LFOStage : public DSPStage {
public:
    LFOStage();
    StageType getType() const override { return StageType::LFO; }
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    RangedParam<double> rate_{1.0, 0.01, 20.0, "rate"};
    RangedParam<double> depth_{0.5, 0.0, 1.0, "depth"};
    std::string waveType_ = "sine";
    double phase_ = 0.0;
    double sampleRate_ = 44100.0;
};

// Routing and connections
struct Connection {
    std::string source;
    std::string destination;
    std::string parameter; // For modulation connections
    double amount = 1.0;
    bool enabled = true;
};

// DSP Graph representation
class DSPGraph {
public:
    DSPGraph() = default;
    ~DSPGraph() = default;
    
    // Graph construction
    void addStage(const std::string& name, std::unique_ptr<DSPStage> stage);
    void removeStage(const std::string& name);
    void addConnection(const Connection& connection);
    void removeConnection(const std::string& source, const std::string& destination);
    
    // Graph processing
    void process(const AudioBuffer& input, AudioBuffer& output);
    void reset();
    
    // Graph analysis
    bool hasCycles() const;
    bool isConnected() const;
    std::vector<std::string> getTopologicalOrder() const;
    double getTotalGain() const; // For feedback stability
    
    // Access
    DSPStage* getStage(const std::string& name);
    const DSPStage* getStage(const std::string& name) const;
    std::vector<std::string> getStageNames() const;
    std::vector<Connection> getConnections() const;
    
    // Validation
    std::vector<std::string> validate() const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<DSPStage>> stages_;
    std::vector<Connection> connections_;
    
    // Graph analysis helpers
    bool hasCycleDFS(const std::string& node, 
                     std::unordered_set<std::string>& visited,
                     std::unordered_set<std::string>& recStack) const;
    void topologicalSortDFS(const std::string& node,
                           std::unordered_set<std::string>& visited,
                           std::vector<std::string>& result) const;
};

// JSON to IR Parser
class IRParser {
public:
    // Parse JSON preset to DSP graph
    std::unique_ptr<DSPGraph> parsePreset(const std::string& jsonData);
    
    // Validate IR structure
    std::vector<std::string> validate(const DSPGraph& graph);
    
    // Compile IR to executable graph
    std::unique_ptr<DSPGraph> compile(const DSPGraph& ir, 
                                     const std::map<std::string, double>& runtimeCaps);
    
private:
    // JSON parsing helpers
    std::unique_ptr<DSPStage> createStageFromJSON(const std::string& type, 
                                                  const ParamMap& params);
    ParamMap parseParameters(const std::string& jsonParams);
    std::vector<Connection> parseConnections(const std::string& jsonConnections);
    
    // Validation helpers
    bool validateParameterRanges(const DSPStage& stage) const;
    bool validateConnections(const DSPGraph& graph) const;
    bool validateFeedbackStability(const DSPGraph& graph) const;
};

// IR Compiler
class IRCompiler {
public:
    struct CompileOptions {
        bool optimizeForCPU = true;
        bool enableSIMD = true;
        bool enableParallel = false;
        double maxLatency = 10.0; // ms
        double cpuBudget = 0.8;   // 0-1
    };
    
    // Compile IR to optimized graph
    std::unique_ptr<DSPGraph> compile(const DSPGraph& ir, 
                                     const CompileOptions& options);
    
    // Estimate CPU cost
    double estimateCPUCost(const DSPGraph& graph) const;
    
    // Estimate latency
    double estimateLatency(const DSPGraph& graph) const;
    
private:
    // Optimization passes
    void optimizeForCPU(DSPGraph& graph);
    void enableSIMD(DSPGraph& graph);
    void enableParallel(DSPGraph& graph);
    void optimizeLatency(DSPGraph& graph);
    
    // Cost estimation
    double getStageCost(const DSPStage& stage) const;
    double getConnectionCost(const Connection& connection) const;
};

} // namespace aiaudio