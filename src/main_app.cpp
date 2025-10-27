#include "main_app.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace aiaudio {

MainApp::MainApp() 
    : sampleRate_(44100.0)
    , bufferSize_(1024)
    , graph_(std::make_unique<DSPGraph>())
{
    // Initialize with default values
}

MainApp::~MainApp() = default;

bool MainApp::loadPreset(const std::string& presetPath) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    try {
        std::ifstream file(presetPath);
        if (!file.is_open()) {
            setLastError("Cannot open preset file: " + presetPath);
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonData = buffer.str();
        
        auto newGraph = IRParser::parsePreset(jsonData);
        if (!newGraph) {
            setLastError("Failed to parse preset JSON");
            return false;
        }
        
        // Validate the new graph
        auto issues = newGraph->validate();
        if (!issues.empty()) {
            setLastError("Preset validation failed: " + issues[0]);
            return false;
        }
        
        graph_ = std::move(newGraph);
        updateSampleRate();
        setLastError("");
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Error loading preset: " + std::string(e.what()));
        return false;
    }
}

bool MainApp::savePreset(const std::string& presetPath) const {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    try {
        // Create a simple JSON representation
        std::stringstream json;
        json << "{\n";
        json << "  \"stages\": {\n";
        
        auto stageNames = graph_->getStageNames();
        for (size_t i = 0; i < stageNames.size(); ++i) {
            const auto& name = stageNames[i];
            auto* stage = graph_->getStage(name);
            if (!stage) continue;
            
            json << "    \"" << name << "\": {\n";
            json << "      \"type\": \"" << (stage->getType() == StageType::OSCILLATOR ? "oscillator" :
                                           stage->getType() == StageType::FILTER ? "filter" :
                                           stage->getType() == StageType::ENVELOPE ? "envelope" :
                                           stage->getType() == StageType::LFO ? "lfo" : "unknown") << "\",\n";
            json << "      \"parameters\": {\n";
            
            auto paramNames = stage->getParameterNames();
            for (size_t j = 0; j < paramNames.size(); ++j) {
                const auto& paramName = paramNames[j];
                auto value = stage->getParameter(paramName);
                
                json << "        \"" << paramName << "\": ";
                if (std::holds_alternative<double>(value)) {
                    json << std::get<double>(value);
                } else if (std::holds_alternative<std::string>(value)) {
                    json << "\"" << std::get<std::string>(value) << "\"";
                } else if (std::holds_alternative<bool>(value)) {
                    json << (std::get<bool>(value) ? "true" : "false");
                }
                
                if (j < paramNames.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }
            
            json << "      }\n";
            json << "    }";
            if (i < stageNames.size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        
        json << "  },\n";
        json << "  \"connections\": [\n";
        
        auto connections = graph_->getConnections();
        for (size_t i = 0; i < connections.size(); ++i) {
            const auto& conn = connections[i];
            json << "    {\n";
            json << "      \"source\": \"" << conn.source << "\",\n";
            json << "      \"destination\": \"" << conn.destination << "\",\n";
            json << "      \"parameter\": \"" << conn.parameter << "\",\n";
            json << "      \"amount\": " << conn.amount << ",\n";
            json << "      \"enabled\": " << (conn.enabled ? "true" : "false") << "\n";
            json << "    }";
            if (i < connections.size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        
        json << "  ]\n";
        json << "}\n";
        
        std::ofstream file(presetPath);
        if (!file.is_open()) {
            setLastError("Cannot create preset file: " + presetPath);
            return false;
        }
        
        file << json.str();
        file.close();
        
        setLastError("");
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Error saving preset: " + std::string(e.what()));
        return false;
    }
}

void MainApp::processAudio(const AudioBuffer& input, AudioBuffer& output) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (!graph_) {
        output = input;
        return;
    }
    
    graph_->process(input, output);
}

void MainApp::reset() {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        graph_->reset();
    }
}

void MainApp::setParameter(const std::string& stageName, const std::string& paramName, const ParamValue& value) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        auto* stage = graph_->getStage(stageName);
        if (stage) {
            stage->setParameter(paramName, value);
        }
    }
}

ParamValue MainApp::getParameter(const std::string& stageName, const std::string& paramName) const {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        auto* stage = graph_->getStage(stageName);
        if (stage) {
            return stage->getParameter(paramName);
        }
    }
    
    return 0.0;
}

void MainApp::addStage(const std::string& name, std::unique_ptr<DSPStage> stage) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        graph_->addStage(name, std::move(stage));
    }
}

void MainApp::removeStage(const std::string& name) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        graph_->removeStage(name);
    }
}

void MainApp::addConnection(const Connection& connection) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        graph_->addConnection(connection);
    }
}

void MainApp::removeConnection(const std::string& source, const std::string& destination) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        graph_->removeConnection(source, destination);
    }
}

std::vector<std::string> MainApp::validate() const {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        return graph_->validate();
    }
    
    return {"No graph loaded"};
}

std::vector<std::string> MainApp::getStageNames() const {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        return graph_->getStageNames();
    }
    
    return {};
}

std::vector<Connection> MainApp::getConnections() const {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        return graph_->getConnections();
    }
    
    return {};
}

double MainApp::getTotalGain() const {
    std::lock_guard<std::mutex> lock(graphMutex_);
    
    if (graph_) {
        return graph_->getTotalGain();
    }
    
    return 1.0;
}

void MainApp::setSampleRate(double sampleRate) {
    sampleRate_ = sampleRate;
    updateSampleRate();
}

double MainApp::getSampleRate() const {
    return sampleRate_;
}

void MainApp::setBufferSize(size_t bufferSize) {
    bufferSize_ = bufferSize;
}

size_t MainApp::getBufferSize() const {
    return bufferSize_;
}

bool MainApp::isInitialized() const {
    return graph_ != nullptr;
}

std::string MainApp::getLastError() const {
    return lastError_;
}

void MainApp::setLastError(const std::string& error) {
    lastError_ = error;
}

void MainApp::updateSampleRate() {
    // Update sample rate for all stages that need it
    if (graph_) {
        auto stageNames = graph_->getStageNames();
        for (const auto& name : stageNames) {
            auto* stage = graph_->getStage(name);
            if (stage && stage->getType() == StageType::OSCILLATOR) {
                // Oscillators might need sample rate updates
                // This is a simplified approach
            }
        }
    }
}

} // namespace aiaudio