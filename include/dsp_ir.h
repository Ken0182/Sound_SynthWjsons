#pragma once

#include "core_types.h"
#include "third_party/nlohmann/json.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace aiaudio {

// DSP Stage base class
class DSPStage {
public:
    virtual ~DSPStage() = default;
    
    virtual void process(const AudioBuffer& input, AudioBuffer& output) = 0;
    virtual void setParameter(const std::string& name, const ParamValue& value) = 0;
    virtual ParamValue getParameter(const std::string& name) const = 0;
    virtual std::vector<std::string> getParameterNames() const = 0;
    virtual void reset() = 0;
    virtual std::string getDescription() const = 0;
    virtual StageType getType() const = 0;
};

// Oscillator Stage
class OscillatorStage : public DSPStage {
public:
    OscillatorStage();
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    StageType getType() const override { return StageType::OSCILLATOR; }

private:
    double frequency_ = 440.0;
    double amplitude_ = 0.5;
    double phase_ = 0.0;
    std::string waveType_ = "sine";
    double phaseAccumulator_ = 0.0;
    double sampleRate_ = 44100.0;
};

// Filter Stage
class FilterStage : public DSPStage {
public:
    FilterStage();
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    StageType getType() const override { return StageType::FILTER; }

private:
    double cutoff_ = 1000.0;
    double resonance_ = 0.1;
    std::string filterType_ = "lowpass";
    double x1_ = 0.0, x2_ = 0.0, y1_ = 0.0, y2_ = 0.0;
};

// Envelope Stage
class EnvelopeStage : public DSPStage {
public:
    EnvelopeStage();
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    StageType getType() const override { return StageType::ENVELOPE; }

private:
    enum class EnvState { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };
    
    double attack_ = 0.01;
    double decay_ = 0.1;
    double sustain_ = 0.7;
    double release_ = 0.5;
    EnvState state_ = EnvState::IDLE;
    double currentLevel_ = 0.0;
    double targetLevel_ = 0.0;
    double rate_ = 0.0;
    size_t sampleCount_ = 0;
};

// LFO Stage
class LFOStage : public DSPStage {
public:
    LFOStage();
    void process(const AudioBuffer& input, AudioBuffer& output) override;
    void setParameter(const std::string& name, const ParamValue& value) override;
    ParamValue getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getDescription() const override;
    StageType getType() const override { return StageType::LFO; }

private:
    double rate_ = 1.0;
    double depth_ = 0.5;
    std::string waveType_ = "sine";
    double phase_ = 0.0;
};

// DSP Graph
class DSPGraph {
public:
    void addStage(const std::string& name, std::unique_ptr<DSPStage> stage);
    void removeStage(const std::string& name);
    void addConnection(const Connection& connection);
    void removeConnection(const std::string& source, const std::string& destination);
    void process(const AudioBuffer& input, AudioBuffer& output);
    void reset();
    
    bool hasCycles() const;
    bool isConnected() const;
    std::vector<std::string> getTopologicalOrder() const;
    double getTotalGain() const;
    
    DSPStage* getStage(const std::string& name);
    const DSPStage* getStage(const std::string& name) const;
    std::vector<std::string> getStageNames() const;
    std::vector<Connection> getConnections() const;
    
    std::vector<std::string> validate() const;

private:
    std::unordered_map<std::string, std::unique_ptr<DSPStage>> stages_;
    std::vector<Connection> connections_;
    
    bool hasCycleDFS(const std::string& node, 
                    std::unordered_set<std::string>& visited,
                    std::unordered_set<std::string>& recStack) const;
    void topologicalSortDFS(const std::string& node,
                           std::unordered_set<std::string>& visited,
                           std::vector<std::string>& result) const;
};

// IR Parser
class IRParser {
public:
    static std::unique_ptr<DSPGraph> parsePreset(const std::string& jsonData);
    static std::vector<std::string> validate(const DSPGraph& graph);

private:
    static std::unique_ptr<DSPStage> createStageFromJSON(const std::string& type, 
                                                        const ParamMap& params);
};

} // namespace aiaudio