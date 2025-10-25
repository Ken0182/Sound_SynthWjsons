#include "main_app.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace aiaudio;

// Example: Advanced usage of the AI Audio Generator
void demonstrateAdvancedFeatures() {
    std::cout << "=== Advanced AI Audio Generator Demo ===" << std::endl;
    
    // Initialize generator
    AIAudioGenerator generator;
    
    // Example 1: Custom configuration
    std::map<std::string, std::string> config;
    config["semantic_model"] = "advanced";
    config["policy_strictness"] = "high";
    config["quality_threshold"] = "0.8";
    generator.setConfiguration(config);
    
    // Example 2: Load custom preset
    try {
        generator.loadPreset("custom_pad.json");
        std::cout << "Loaded custom preset" << std::endl;
    } catch (const AIAudioException& e) {
        std::cout << "Could not load custom preset: " << e.what() << std::endl;
    }
    
    // Example 3: Generate with specific constraints
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "ethereal pad with slow attack and long release";
    request.role = Role::PAD;
    request.context.tempo = 90.0;
    request.context.key = 5; // F major
    request.context.scale = "major";
    request.constraints.maxCPU = 0.5;
    request.constraints.maxLatency = 5.0;
    request.constraints.lufsTarget = -20.0;
    request.constraints.truePeakLimit = -0.5;
    request.useSemanticSearch = true;
    request.applyPolicies = true;
    request.optimizeForMOO = true;
    
    auto result = generator.generate(request);
    
    std::cout << "Generation Results:" << std::endl;
    std::cout << "  Audio samples: " << result.audio.size() << std::endl;
    std::cout << "  Quality score: " << result.qualityScore << std::endl;
    std::cout << "  Warnings: " << result.warnings.size() << std::endl;
    std::cout << "  Explanation: " << result.explanation << std::endl;
    
    // Example 4: Save generated audio
    std::ofstream audioFile("generated_audio.raw", std::ios::binary);
    if (audioFile.is_open()) {
        audioFile.write(reinterpret_cast<const char*>(result.audio.data()), 
                       result.audio.size() * sizeof(float));
        audioFile.close();
        std::cout << "Saved audio to generated_audio.raw" << std::endl;
    }
    
    // Example 5: Batch processing
    std::vector<std::pair<std::string, Role>> batchRequests = {
        {"warm analog pad", Role::PAD},
        {"deep sub bass", Role::BASS},
        {"bright lead synth", Role::LEAD},
        {"percussive texture", Role::TEXTURE},
        {"atmospheric pad", Role::AMBIENT}
    };
    
    std::cout << "\nBatch Processing:" << std::endl;
    for (const auto& [prompt, role] : batchRequests) {
        AIAudioGenerator::GenerationRequest batchRequest;
        batchRequest.prompt = prompt;
        batchRequest.role = role;
        batchRequest.context.tempo = 120.0;
        batchRequest.context.key = 0;
        batchRequest.context.scale = "major";
        batchRequest.constraints.maxCPU = 0.8;
        batchRequest.constraints.maxLatency = 10.0;
        
        auto batchResult = generator.generate(batchRequest);
        std::cout << "  " << prompt << " -> Quality: " << batchResult.qualityScore << std::endl;
    }
}

// Example: Real-time audio generation
void demonstrateRealtimeGeneration() {
    std::cout << "\n=== Real-time Generation Demo ===" << std::endl;
    
    AIAudioGenerator generator;
    AudioRenderer renderer;
    
    // Create a simple graph for real-time testing
    DSPGraph graph;
    auto osc = std::make_unique<OscillatorStage>();
    osc->setParameter("frequency", 440.0);
    osc->setParameter("amplitude", 0.5);
    graph.addStage("osc1", std::move(osc));
    
    // Test real-time rendering
    const size_t bufferSize = 1024; // Small buffer for real-time
    const double maxLatencyMs = 5.0;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10; ++i) {
        auto audio = renderer.renderRealtime(graph, bufferSize, maxLatencyMs);
        auto stats = renderer.getLastRenderStats();
        
        std::cout << "Buffer " << i << ": " << audio.size() << " samples, "
                  << "Render time: " << stats.renderTime << "ms, "
                  << "Realtime: " << (stats.realtimeSuccess ? "Yes" : "No") << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "Total processing time: " << totalTime.count() << "ms" << std::endl;
}

// Example: Quality assessment
void demonstrateQualityAssessment() {
    std::cout << "\n=== Quality Assessment Demo ===" << std::endl;
    
    AIAudioGenerator generator;
    QualityAssessor assessor;
    
    // Generate test audio
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "test pad";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    
    auto result = generator.generate(request);
    
    // Assess quality
    AudioConstraints constraints;
    auto metrics = assessor.getDetailedMetrics(result.audio, Role::PAD, constraints);
    
    std::cout << "Quality Metrics:" << std::endl;
    std::cout << "  Overall Score: " << metrics.overallScore << std::endl;
    std::cout << "  Semantic Match: " << metrics.semanticMatch << std::endl;
    std::cout << "  Mix Readiness: " << metrics.mixReadiness << std::endl;
    std::cout << "  Perceptual Quality: " << metrics.perceptualQuality << std::endl;
    std::cout << "  Stability: " << metrics.stability << std::endl;
    
    if (!metrics.issues.empty()) {
        std::cout << "  Issues:" << std::endl;
        for (const auto& issue : metrics.issues) {
            std::cout << "    - " << issue << std::endl;
        }
    }
}

// Example: System monitoring
void demonstrateSystemMonitoring() {
    std::cout << "\n=== System Monitoring Demo ===" << std::endl;
    
    AIAudioGenerator generator;
    SystemMonitor monitor;
    
    monitor.startMonitoring();
    
    // Generate multiple samples to monitor performance
    for (int i = 0; i < 20; ++i) {
        AIAudioGenerator::GenerationRequest request;
        request.prompt = "monitoring test " + std::to_string(i);
        request.role = static_cast<Role>(i % 4); // Cycle through roles
        request.context.tempo = 100.0 + (i * 5);
        request.context.key = i % 12;
        request.context.scale = "major";
        
        try {
            auto result = generator.generate(request);
            std::cout << "Generated sample " << i << " (Quality: " << result.qualityScore << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error generating sample " << i << ": " << e.what() << std::endl;
        }
    }
    
    auto metrics = monitor.getMetrics();
    std::cout << "\nFinal Performance Metrics:" << std::endl;
    std::cout << "  CPU Usage: " << metrics.cpuUsage << "%" << std::endl;
    std::cout << "  Memory Usage: " << metrics.memoryUsage << "%" << std::endl;
    std::cout << "  Disk Usage: " << metrics.diskUsage << "%" << std::endl;
    std::cout << "  Active Threads: " << metrics.activeThreads << std::endl;
    std::cout << "  Average Latency: " << metrics.averageLatency << "ms" << std::endl;
    std::cout << "  Total Renders: " << metrics.totalRenders << std::endl;
    std::cout << "  Successful Renders: " << metrics.successfulRenders << std::endl;
    
    monitor.stopMonitoring();
}

int main() {
    try {
        demonstrateAdvancedFeatures();
        demonstrateRealtimeGeneration();
        demonstrateQualityAssessment();
        demonstrateSystemMonitoring();
        
        std::cout << "\n=== All Demos Complete ===" << std::endl;
        
    } catch (const AIAudioException& e) {
        std::cerr << "AI Audio Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}