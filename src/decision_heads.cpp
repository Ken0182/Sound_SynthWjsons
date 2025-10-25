#include "decision_heads.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

namespace aiaudio {

// DecisionMLP implementation
DecisionMLP::DecisionMLP(size_t inputSize, const std::vector<size_t>& hiddenSizes, size_t outputSize)
    : inputSize_(inputSize), outputSize_(outputSize) {
    
    // Create layers
    size_t prevSize = inputSize;
    for (size_t hiddenSize : hiddenSizes) {
        Layer layer;
        layer.weights.resize(hiddenSize, std::vector<double>(prevSize));
        layer.biases.resize(hiddenSize);
        layer.activation = "relu";
        layers.push_back(layer);
        prevSize = hiddenSize;
    }
    
    // Output layer
    Layer outputLayer;
    outputLayer.weights.resize(outputSize, std::vector<double>(prevSize));
    outputLayer.biases.resize(outputSize);
    outputLayer.activation = "sigmoid";
    layers.push_back(outputLayer);
    
    initializeWeights();
}

std::vector<double> DecisionMLP::forward(const std::vector<double>& input) const {
    if (input.size() != inputSize_) {
        throw AIAudioException("Input size mismatch");
    }
    
    std::vector<double> current = input;
    
    for (const auto& layer : layers) {
        std::vector<double> next(layer.weights.size());
        
        for (size_t i = 0; i < layer.weights.size(); ++i) {
            double sum = layer.biases[i];
            for (size_t j = 0; j < layer.weights[i].size(); ++j) {
                sum += layer.weights[i][j] * current[j];
            }
            next[i] = applyActivation(sum, layer.activation);
        }
        
        current = next;
    }
    
    return current;
}

void DecisionMLP::loadFromONNX(const std::string& modelPath) {
    // Placeholder for ONNX loading
    // In practice, would use ONNX Runtime
}

void DecisionMLP::saveToONNX(const std::string& modelPath) const {
    // Placeholder for ONNX saving
    // In practice, would use ONNX Runtime
}

void DecisionMLP::quantizeToInt8() {
    // Placeholder for int8 quantization
    // In practice, would implement proper quantization
}

double DecisionMLP::relu(double x) const {
    return std::max(0.0, x);
}

double DecisionMLP::sigmoid(double x) const {
    return 1.0 / (1.0 + std::exp(-x));
}

double DecisionMLP::tanh(double x) const {
    return std::tanh(x);
}

double DecisionMLP::linear(double x) const {
    return x;
}

double DecisionMLP::applyActivation(double x, const std::string& activation) const {
    if (activation == "relu") return relu(x);
    if (activation == "sigmoid") return sigmoid(x);
    if (activation == "tanh") return tanh(x);
    if (activation == "linear") return linear(x);
    return x;
}

void DecisionMLP::initializeWeights() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, 0.1);
    
    for (auto& layer : layers) {
        for (auto& row : layer.weights) {
            for (double& weight : row) {
                weight = dist(gen);
            }
        }
        
        for (double& bias : layer.biases) {
            bias = dist(gen);
        }
    }
}

// DecisionContext implementation
std::vector<double> DecisionContext::toInputVector() const {
    std::vector<double> input;
    
    // Add query vector
    input.insert(input.end(), queryVector.begin(), queryVector.end());
    
    // Add role one-hot encoding
    std::vector<double> roleOneHot(8, 0.0); // 8 roles
    roleOneHot[static_cast<int>(role)] = 1.0;
    input.insert(input.end(), roleOneHot.begin(), roleOneHot.end());
    
    // Add tempo (normalized)
    input.push_back(tempo / 200.0); // Normalize to [0, 1]
    
    // Add key (normalized)
    input.push_back(key / 12.0); // Normalize to [0, 1]
    
    // Add entry stats
    input.insert(input.end(), entryStats.begin(), entryStats.end());
    
    // Add metadata (simplified)
    input.push_back(metadata.size() / 10.0); // Normalize metadata count
    
    return input;
}

// DecisionHeads implementation
DecisionHeads::DecisionHeads(std::unique_ptr<DecisionMLP> model) : model_(std::move(model)) {
}

DecisionOutput DecisionHeads::infer(const DecisionContext& context) const {
    std::vector<double> input = context.toInputVector();
    std::vector<double> output = model_->forward(input);
    
    DecisionOutput result;
    
    // Split output into values and routes
    size_t numValues = output.size() / 2; // Assume half for values, half for routes
    size_t numRoutes = output.size() - numValues;
    
    // Extract values (μ)
    for (size_t i = 0; i < numValues; ++i) {
        result.values.push_back(output[i]);
    }
    
    // Extract routes (sigmoid -> threshold)
    for (size_t i = numValues; i < output.size(); ++i) {
        result.routes.push_back(output[i] > 0.5);
    }
    
    // Compute confidence (average of values)
    result.confidence = 0.0;
    for (double value : result.values) {
        result.confidence += value;
    }
    result.confidence /= result.values.size();
    
    // Map to parameters and routes
    result.parameterValues = mapValuesToParameters(result.values, context.role);
    result.routingMask = mapRoutesToTargets(result.routes, DSPGraph{}); // Would need actual graph
    
    return result;
}

void DecisionHeads::applyDecisions(DSPGraph& graph, const DecisionOutput& decisions) const {
    // Apply parameter values to graph stages
    auto stageNames = graph.getStageNames();
    for (const auto& [paramName, value] : decisions.parameterValues) {
        // Find stage containing this parameter
        for (const auto& stageName : stageNames) {
            auto* stage = graph.getStage(stageName);
            if (stage) {
                auto paramNames = stage->getParameterNames();
                if (std::find(paramNames.begin(), paramNames.end(), paramName) != paramNames.end()) {
                    stage->setParameter(paramName, value);
                    break;
                }
            }
        }
    }
    
    // Apply routing decisions
    for (const auto& [target, enabled] : decisions.routingMask) {
        if (enabled) {
            // Enable routing to target
            // This would involve modifying graph connections
        }
    }
}

DecisionOutput DecisionHeads::addJitter(const DecisionOutput& decisions, double sigma) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, sigma);
    
    DecisionOutput jittered = decisions;
    
    // Add jitter to values
    for (double& value : jittered.values) {
        double jitter = dist(gen);
        value = std::clamp(value + jitter, 0.0, 1.0);
    }
    
    // Add jitter to routes (with lower probability)
    for (bool& route : jittered.routes) {
        if (dist(gen) < sigma) {
            route = !route;
        }
    }
    
    return jittered;
}

std::vector<std::string> DecisionHeads::validateDecisions(const DecisionOutput& decisions) const {
    std::vector<std::string> issues;
    
    // Validate value ranges
    for (size_t i = 0; i < decisions.values.size(); ++i) {
        if (decisions.values[i] < 0.0 || decisions.values[i] > 1.0) {
            issues.push_back("Value " + std::to_string(i) + " out of range [0,1]");
        }
    }
    
    // Validate confidence
    if (decisions.confidence < 0.0 || decisions.confidence > 1.0) {
        issues.push_back("Confidence out of range [0,1]");
    }
    
    return issues;
}

std::map<std::string, double> DecisionHeads::mapValuesToParameters(const std::vector<double>& values, 
                                                                 Role role) const {
    std::map<std::string, double> parameters;
    
    auto ranges = getParameterRanges(role);
    size_t paramIndex = 0;
    
    for (const auto& [paramName, range] : ranges) {
        if (paramIndex < values.size()) {
            double value = values[paramIndex];
            double minVal = range.first;
            double maxVal = range.second;
            
            // Map from [0,1] to [min,max]
            double mappedValue = minVal + value * (maxVal - minVal);
            parameters[paramName] = mappedValue;
        }
        paramIndex++;
    }
    
    return parameters;
}

std::map<std::string, bool> DecisionHeads::mapRoutesToTargets(const std::vector<bool>& routes, 
                                                             const DSPGraph& graph) const {
    std::map<std::string, bool> routingMask;
    
    auto availableRoutes = getAvailableRoutes(graph);
    for (size_t i = 0; i < routes.size() && i < availableRoutes.size(); ++i) {
        routingMask[availableRoutes[i]] = routes[i];
    }
    
    return routingMask;
}

double DecisionHeads::mapValueToParameter(double value, const std::string& paramName, Role role) const {
    auto ranges = getParameterRanges(role);
    auto it = ranges.find(paramName);
    if (it != ranges.end()) {
        double minVal = it->second.first;
        double maxVal = it->second.second;
        return minVal + value * (maxVal - minVal);
    }
    return value;
}

std::string DecisionHeads::getParameterName(size_t valueIndex, Role role) const {
    auto ranges = getParameterRanges(role);
    size_t index = 0;
    for (const auto& [paramName, range] : ranges) {
        if (index == valueIndex) {
            return paramName;
        }
        index++;
    }
    return "";
}

std::string DecisionHeads::getRouteTarget(size_t routeIndex, const DSPGraph& graph) const {
    auto routes = getAvailableRoutes(graph);
    if (routeIndex < routes.size()) {
        return routes[routeIndex];
    }
    return "";
}

bool DecisionHeads::isValidRoute(const std::string& source, const std::string& target) const {
    // Simplified validation - in practice would check graph structure
    return !source.empty() && !target.empty() && source != target;
}

std::map<std::string, std::pair<double, double>> DecisionHeads::getParameterRanges(Role role) const {
    std::map<std::string, std::pair<double, double>> ranges;
    
    switch (role) {
        case Role::PAD:
            ranges["frequency"] = {100.0, 2000.0};
            ranges["amplitude"] = {0.2, 0.8};
            ranges["attack"] = {0.1, 2.0};
            ranges["decay"] = {0.2, 3.0};
            ranges["sustain"] = {0.3, 0.9};
            ranges["release"] = {0.5, 5.0};
            break;
        case Role::BASS:
            ranges["frequency"] = {20.0, 500.0};
            ranges["amplitude"] = {0.4, 1.0};
            ranges["attack"] = {0.001, 0.1};
            ranges["decay"] = {0.01, 0.5};
            ranges["sustain"] = {0.5, 1.0};
            ranges["release"] = {0.1, 2.0};
            break;
        case Role::LEAD:
            ranges["frequency"] = {200.0, 4000.0};
            ranges["amplitude"] = {0.5, 1.0};
            ranges["attack"] = {0.01, 0.5};
            ranges["decay"] = {0.05, 1.0};
            ranges["sustain"] = {0.6, 1.0};
            ranges["release"] = {0.2, 3.0};
            break;
        default:
            // Default ranges
            ranges["frequency"] = {20.0, 20000.0};
            ranges["amplitude"] = {0.0, 1.0};
            ranges["attack"] = {0.001, 10.0};
            ranges["decay"] = {0.001, 10.0};
            ranges["sustain"] = {0.0, 1.0};
            ranges["release"] = {0.001, 10.0};
            break;
    }
    
    return ranges;
}

std::vector<std::string> DecisionHeads::getAvailableRoutes(const DSPGraph& graph) const {
    std::vector<std::string> routes;
    
    // Get all possible routing targets from graph
    auto stageNames = graph.getStageNames();
    for (const auto& stageName : stageNames) {
        auto* stage = graph.getStage(stageName);
        if (stage) {
            auto paramNames = stage->getParameterNames();
            for (const auto& paramName : paramNames) {
                routes.push_back(stageName + "." + paramName);
            }
        }
    }
    
    return routes;
}

// DecisionTrainer implementation
void DecisionTrainer::trainModel(DecisionMLP& model, const TrainingData& data, 
                                size_t epochs, double learningRate) const {
    if (data.contexts.size() != data.targets.size()) {
        throw AIAudioException("Context and target size mismatch");
    }
    
    for (size_t epoch = 0; epoch < epochs; ++epoch) {
        double totalLoss = 0.0;
        
        for (size_t i = 0; i < data.contexts.size(); ++i) {
            const auto& context = data.contexts[i];
            const auto& target = data.targets[i];
            
            // Forward pass
            std::vector<double> input = context.toInputVector();
            std::vector<double> prediction = model.forward(input);
            
            // Compute loss
            std::vector<double> targetValues = target.values;
            targetValues.insert(targetValues.end(), target.routes.begin(), target.routes.end());
            
            double loss = computeLoss(prediction, targetValues);
            totalLoss += loss;
            
            // Compute gradient and update weights
            std::vector<double> gradient = computeGradient(prediction, targetValues);
            updateWeights(model, gradient, learningRate);
        }
        
        // Print progress
        if (epoch % 10 == 0) {
            std::cout << "Epoch " << epoch << ", Loss: " << totalLoss / data.contexts.size() << std::endl;
        }
    }
}

DecisionTrainer::TrainingData DecisionTrainer::generateFromRules(Role role, size_t numSamples) const {
    TrainingData data;
    
    for (size_t i = 0; i < numSamples; ++i) {
        DecisionContext context = generateRandomContext(role);
        DecisionOutput target = generateTargetFromContext(context);
        
        data.contexts.push_back(context);
        data.targets.push_back(target);
        data.weights.push_back(1.0);
    }
    
    return data;
}

std::vector<std::string> DecisionTrainer::validateTrainingData(const TrainingData& data) const {
    std::vector<std::string> issues;
    
    if (data.contexts.size() != data.targets.size()) {
        issues.push_back("Context and target count mismatch");
    }
    
    if (data.contexts.size() != data.weights.size()) {
        issues.push_back("Context and weight count mismatch");
    }
    
    for (size_t i = 0; i < data.contexts.size(); ++i) {
        const auto& context = data.contexts[i];
        const auto& target = data.targets[i];
        
        if (context.queryVector.empty()) {
            issues.push_back("Empty query vector at index " + std::to_string(i));
        }
        
        if (target.values.empty()) {
            issues.push_back("Empty target values at index " + std::to_string(i));
        }
        
        for (double value : target.values) {
            if (value < 0.0 || value > 1.0) {
                issues.push_back("Target value out of range at index " + std::to_string(i));
            }
        }
    }
    
    return issues;
}

DecisionTrainer::TrainingData DecisionTrainer::augmentData(const TrainingData& data, double noiseLevel) const {
    TrainingData augmented = data;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, noiseLevel);
    
    // Add noise to contexts
    for (auto& context : augmented.contexts) {
        for (double& value : context.queryVector) {
            value += dist(gen);
        }
        
        for (double& value : context.entryStats) {
            value += dist(gen);
        }
    }
    
    // Add noise to targets
    for (auto& target : augmented.targets) {
        for (double& value : target.values) {
            value = std::clamp(value + dist(gen), 0.0, 1.0);
        }
    }
    
    return augmented;
}

DecisionContext DecisionTrainer::generateRandomContext(Role role) const {
    DecisionContext context;
    
    // Generate random query vector
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    
    context.queryVector.resize(384); // Default embedding size
    for (double& value : context.queryVector) {
        value = dist(gen);
    }
    
    context.role = role;
    context.tempo = 60.0 + (gen() % 140); // 60-200 BPM
    context.key = gen() % 12; // 0-11
    
    // Generate random entry stats
    context.entryStats.resize(10);
    for (double& value : context.entryStats) {
        value = dist(gen);
    }
    
    return context;
}

DecisionOutput DecisionTrainer::generateTargetFromContext(const DecisionContext& context) const {
    DecisionOutput target;
    
    // Generate target values based on context
    // This is a simplified implementation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    // Generate values based on role
    switch (context.role) {
        case Role::PAD:
            target.values = {0.7, 0.5, 0.8, 0.6, 0.7, 0.9}; // Pad characteristics
            break;
        case Role::BASS:
            target.values = {0.3, 0.8, 0.2, 0.4, 0.8, 0.3}; // Bass characteristics
            break;
        case Role::LEAD:
            target.values = {0.8, 0.9, 0.6, 0.7, 0.8, 0.6}; // Lead characteristics
            break;
        default:
            target.values.resize(6, 0.5);
            break;
    }
    
    // Generate routes
    target.routes.resize(10);
    for (bool& route : target.routes) {
        route = dist(gen) > 0.5;
    }
    
    target.confidence = 0.8; // Default confidence
    
    return target;
}

double DecisionTrainer::computeLoss(const std::vector<double>& prediction, 
                                   const std::vector<double>& target) const {
    if (prediction.size() != target.size()) {
        return 1.0; // Maximum loss for size mismatch
    }
    
    double loss = 0.0;
    for (size_t i = 0; i < prediction.size(); ++i) {
        double diff = prediction[i] - target[i];
        loss += diff * diff;
    }
    
    return loss / prediction.size();
}

std::vector<double> DecisionTrainer::computeGradient(const std::vector<double>& prediction,
                                                    const std::vector<double>& target) const {
    std::vector<double> gradient(prediction.size());
    
    for (size_t i = 0; i < prediction.size(); ++i) {
        gradient[i] = 2.0 * (prediction[i] - target[i]);
    }
    
    return gradient;
}

void DecisionTrainer::updateWeights(DecisionMLP& model, const std::vector<double>& gradient, 
                                   double learningRate) const {
    // Simplified weight update
    // In practice, would implement proper backpropagation
    for (auto& layer : model.layers) {
        for (auto& row : layer.weights) {
            for (double& weight : row) {
                weight -= learningRate * 0.01; // Simplified update
            }
        }
        
        for (double& bias : layer.biases) {
            bias -= learningRate * 0.01; // Simplified update
        }
    }
}

} // namespace aiaudio