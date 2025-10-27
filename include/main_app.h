#pragma once

#include "core_types.h"
#include "dsp_ir.h"
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

namespace aiaudio {

class MainApp {
public:
    MainApp();
    ~MainApp();
    
    // Core functionality
    bool loadPreset(const std::string& presetPath);
    bool savePreset(const std::string& presetPath) const;
    void processAudio(const AudioBuffer& input, AudioBuffer& output);
    void reset();
    
    // Real-time control
    void setParameter(const std::string& stageName, const std::string& paramName, const ParamValue& value);
    ParamValue getParameter(const std::string& stageName, const std::string& paramName) const;
    
    // Graph management
    void addStage(const std::string& name, std::unique_ptr<DSPStage> stage);
    void removeStage(const std::string& name);
    void addConnection(const Connection& connection);
    void removeConnection(const std::string& source, const std::string& destination);
    
    // Validation and diagnostics
    std::vector<std::string> validate() const;
    std::vector<std::string> getStageNames() const;
    std::vector<Connection> getConnections() const;
    double getTotalGain() const;
    
    // Audio settings
    void setSampleRate(double sampleRate);
    double getSampleRate() const;
    void setBufferSize(size_t bufferSize);
    size_t getBufferSize() const;
    
    // Status
    bool isInitialized() const;
    std::string getLastError() const;
    
private:
    std::unique_ptr<DSPGraph> graph_;
    double sampleRate_;
    size_t bufferSize_;
    std::string lastError_;
    mutable std::mutex graphMutex_;
    
    void setLastError(const std::string& error);
    void updateSampleRate();
};

} // namespace aiaudio