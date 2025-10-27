#pragma once

#include "core_types.h"
#include "dsp_ir.h"
#include "semantic_fusion.h"
#include <vector>
#include <memory>
#include <map>
#include <random>
#include <iostream>

namespace aiaudio {

// Decision Heads (μ) + Routing Masks (R)

// MLP model for decision making
class DecisionMLP {
public:
    struct Layer {
        std::vector<std::vector<double>> weights;
        std::vector<double> biases;
        std::string activation; // "relu", "sigmoid", "tanh", "linear"
    };
    
    // Model architecture
    std::vector<Layer> layers;
    size_t inputSize;
    size_t outputSize;
    
    // Initialize model
    DecisionMLP(size_t inputSize, const std::vector<size_t>& hiddenSizes, size_t outputSize);
    
    // Forward pass
    std::vector<double> forward(const std::vector<double>& input) const;
    
    // Load from ONNX (placeholder)
    void loadFromONNX(const std::string& modelPath);
    
    // Save to ONNX (placeholder)
    void saveToONNX(const std::string& modelPath) const;
    
    // Quantize to int8
    void quantizeToInt8();
    
private:
    // Activation functions
    double relu(double x) const;
    double sigmoid(double x) const;
    double tanh(double x) const;
    double linear(double x) const;
    
    // Apply activation
    double applyActivation(double x, const std::string& activation) const;
    
    // Initialize weights
    void initializeWeights();
};

// Decision context
struct DecisionContext {
    std::vector<double> queryVector;      // Semantic query embedding
    Role role;                           // Target role
    FrequencyHz tempo;                   // Musical tempo
    int key;                            // Musical key
    std::vector<double> entryStats;      // Entry statistics
    std::map<std::string, double> metadata; // Additional metadata
    
    // Convert to MLP input vector
    std::vector<double> toInputVector() const;
};

// Decision output
struct DecisionOutput {
    std::vector<double> values;          // μ values [0,1]
    std::vector<bool> routes;            // Routing mask
    double confidence;                   // Overall confidence
    std::map<std::string, double> parameterValues; // Mapped parameter values
    std::map<std::string, bool> routingMask;       // Mapped routing mask
};

// Decision heads system
class DecisionHeads {
public:
    // Initialize with model
    explicit DecisionHeads(std::unique_ptr<DecisionMLP> model);
    
    // Make decisions from context
    DecisionOutput infer(const DecisionContext& context) const;
    
    // Apply decisions to DSP graph
    void applyDecisions(DSPGraph& graph, const DecisionOutput& decisions) const;
    
    // Add jitter for regularization
    DecisionOutput addJitter(const DecisionOutput& decisions, double sigma = 0.01) const;
    
    // Validate decisions
    std::vector<std::string> validateDecisions(const DecisionOutput& decisions) const;
    
    // Get parameter mapping
    std::map<std::string, double> mapValuesToParameters(const std::vector<double>& values, 
                                                       Role role) const;
    
    // Get routing mapping
    std::map<std::string, bool> mapRoutesToTargets(const std::vector<bool>& routes, 
                                                   const DSPGraph& graph) const;
    
private:
    std::unique_ptr<DecisionMLP> model_;
    
    // Parameter mapping functions
    double mapValueToParameter(double value, const std::string& paramName, Role role) const;
    std::string getParameterName(size_t valueIndex, Role role) const;
    
    // Routing mapping functions
    std::string getRouteTarget(size_t routeIndex, const DSPGraph& graph) const;
    bool isValidRoute(const std::string& source, const std::string& target) const;
    
    // Role-specific mappings
    std::map<std::string, std::pair<double, double>> getParameterRanges(Role role) const;
    std::vector<std::string> getAvailableRoutes(const DSPGraph& graph) const;
};

// ONNX model interface
class ONNXModel {
public:
    // Load model from file
    bool loadModel(const std::string& modelPath);
    
    // Run inference
    std::vector<double> runInference(const std::vector<double>& input) const;
    
    // Get input/output shapes
    std::pair<size_t, size_t> getInputShape() const;
    std::pair<size_t, size_t> getOutputShape() const;
    
    // Quantize model
    void quantize(int8_t targetPrecision = 8);
    
private:
    // ONNX runtime would be used here
    // This is a placeholder implementation
    std::vector<std::vector<double>> weights_;
    std::vector<double> biases_;
    size_t inputSize_ = 0;
    size_t outputSize_ = 0;
};

// Decision training system
class DecisionTrainer {
public:
    struct TrainingData {
        std::vector<DecisionContext> contexts;
        std::vector<DecisionOutput> targets;
        std::vector<double> weights;
    };
    
    // Train decision model
    void trainModel(DecisionMLP& model, const TrainingData& data, 
                   size_t epochs = 100, double learningRate = 0.001) const;
    
    // Generate training data from rules
    TrainingData generateFromRules(Role role, size_t numSamples = 1000) const;
    
    // Validate training data
    std::vector<std::string> validateTrainingData(const TrainingData& data) const;
    
    // Data augmentation
    TrainingData augmentData(const TrainingData& data, double noiseLevel = 0.1) const;
    
private:
    // Training helpers
    double computeLoss(const std::vector<double>& prediction, 
                      const std::vector<double>& target) const;
    std::vector<double> computeGradient(const std::vector<double>& prediction,
                                       const std::vector<double>& target) const;
    void updateWeights(DecisionMLP& model, const std::vector<double>& gradient, 
                      double learningRate) const;
    
    // Data generation
    DecisionContext generateRandomContext(Role role) const;
    DecisionOutput generateTargetFromContext(const DecisionContext& context) const;
};

// Decision validation and testing
class DecisionValidator {
public:
    // Validate decision ranges
    bool validateRanges(const DecisionOutput& decisions) const;
    
    // Validate routing targets
    bool validateRouting(const DecisionOutput& decisions, const DSPGraph& graph) const;
    
    // Test decision consistency
    bool testConsistency(const std::vector<DecisionOutput>& decisions) const;
    
    // Benchmark decision performance
    double benchmarkDecisions(const DecisionHeads& heads, 
                             const std::vector<DecisionContext>& contexts) const;
    
    // Test latency requirements
    bool testLatency(const DecisionHeads& heads, double maxLatencyMs = 1.0) const;
    
private:
    // Validation helpers
    bool isValueInRange(double value, double min, double max) const;
    bool isRouteValid(const std::string& source, const std::string& target, 
                     const DSPGraph& graph) const;
    
    // Performance measurement
    double measureInferenceTime(const DecisionHeads& heads, 
                               const DecisionContext& context) const;
};

// Decision explanation system
class DecisionExplainer {
public:
    struct Explanation {
        std::string summary;
        std::vector<std::string> contributingFactors;
        std::map<std::string, double> parameterInfluences;
        std::map<std::string, double> routingInfluences;
        double confidence;
    };
    
    // Explain decision
    Explanation explainDecision(const DecisionContext& context, 
                               const DecisionOutput& decision) const;
    
    // Get parameter influence
    std::map<std::string, double> getParameterInfluence(const DecisionContext& context) const;
    
    // Get routing influence
    std::map<std::string, double> getRoutingInfluence(const DecisionContext& context) const;
    
    // Generate human-readable explanation
    std::string generateExplanation(const Explanation& explanation) const;
    
private:
    // Explanation helpers
    std::vector<std::string> identifyContributingFactors(const DecisionContext& context) const;
    double computeParameterInfluence(const std::string& param, const DecisionContext& context) const;
    double computeRoutingInfluence(const std::string& route, const DecisionContext& context) const;
};

// Decision caching and optimization
class DecisionCache {
public:
    // Cache decision
    void cacheDecision(const DecisionContext& context, const DecisionOutput& decision);
    
    // Get cached decision
    std::optional<DecisionOutput> getCachedDecision(const DecisionContext& context) const;
    
    // Clear cache
    void clearCache();
    
    // Get cache statistics
    struct CacheStats {
        size_t hits;
        size_t misses;
        size_t size;
        double hitRate;
    };
    CacheStats getCacheStats() const;
    
private:
    std::map<std::string, DecisionOutput> cache_;
    mutable size_t hits_ = 0;
    mutable size_t misses_ = 0;
    
    // Cache key generation
    std::string generateCacheKey(const DecisionContext& context) const;
};

} // namespace aiaudio