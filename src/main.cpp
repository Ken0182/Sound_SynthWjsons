#include "main_app.h"
#include <iostream>
#include <string>
#include <vector>

using namespace aiaudio;

int main(int /* argc */, char* /* argv */[]) {
    try {
        // Initialize the AI Audio Generator
        std::cout << "Initializing AI Audio Generator..." << std::endl;
        AIAudioGenerator generator;
        
        // Check system status
        auto status = generator.getStatus();
        std::cout << "System Status:" << std::endl;
        std::cout << "  Initialized: " << (status.initialized ? "Yes" : "No") << std::endl;
        std::cout << "  Loaded Presets: " << status.loadedPresets << std::endl;
        std::cout << "  Active Features: ";
        for (const auto& feature : status.activeFeatures) {
            std::cout << feature << " ";
        }
        std::cout << std::endl;
        
        // Example 1: Generate a pad sound
        std::cout << "\n=== Example 1: Dreamy Pad ===" << std::endl;
        AIAudioGenerator::GenerationRequest padRequest;
        padRequest.prompt = "dreamy atmospheric pad with reverb";
        padRequest.role = Role::PAD;
        padRequest.context.tempo = 120.0;
        padRequest.context.key = 0; // C major
        padRequest.context.scale = "major";
        padRequest.constraints.maxCPU = 0.8;
        padRequest.constraints.maxLatency = 10.0;
        padRequest.useSemanticSearch = true;
        padRequest.applyPolicies = true;
        
        auto padResult = generator.generate(padRequest);
        std::cout << "Generated pad audio: " << padResult.audio.size() << " samples" << std::endl;
        std::cout << "Quality Score: " << padResult.qualityScore << std::endl;
        std::cout << "Explanation: " << padResult.explanation << std::endl;
        
        if (!padResult.warnings.empty()) {
            std::cout << "Warnings:" << std::endl;
            for (const auto& warning : padResult.warnings) {
                std::cout << "  - " << warning << std::endl;
            }
        }
        
        // Example 2: Generate a bass sound
        std::cout << "\n=== Example 2: Punchy Bass ===" << std::endl;
        AIAudioGenerator::GenerationRequest bassRequest;
        bassRequest.prompt = "punchy bass with tight envelope";
        bassRequest.role = Role::BASS;
        bassRequest.context.tempo = 140.0;
        bassRequest.context.key = 7; // G major
        bassRequest.context.scale = "major";
        bassRequest.constraints.maxCPU = 0.6;
        bassRequest.constraints.maxLatency = 5.0;
        
        auto bassResult = generator.generate(bassRequest);
        std::cout << "Generated bass audio: " << bassResult.audio.size() << " samples" << std::endl;
        std::cout << "Quality Score: " << bassResult.qualityScore << std::endl;
        std::cout << "Explanation: " << bassResult.explanation << std::endl;
        
        // Example 3: Generate a lead sound
        std::cout << "\n=== Example 3: Bright Lead ===" << std::endl;
        AIAudioGenerator::GenerationRequest leadRequest;
        leadRequest.prompt = "bright lead with modulation";
        leadRequest.role = Role::LEAD;
        leadRequest.context.tempo = 128.0;
        leadRequest.context.key = 2; // D major
        leadRequest.context.scale = "major";
        leadRequest.constraints.maxCPU = 0.7;
        leadRequest.constraints.maxLatency = 8.0;
        
        auto leadResult = generator.generate(leadRequest);
        std::cout << "Generated lead audio: " << leadResult.audio.size() << " samples" << std::endl;
        std::cout << "Quality Score: " << leadResult.qualityScore << std::endl;
        std::cout << "Explanation: " << leadResult.explanation << std::endl;
        
        // Example 4: Batch generation
        std::cout << "\n=== Example 4: Batch Generation ===" << std::endl;
        std::vector<std::string> prompts = {
            "warm pad",
            "aggressive bass",
            "melodic lead",
            "percussive texture"
        };
        
        std::vector<Role> roles = {
            Role::PAD,
            Role::BASS,
            Role::LEAD,
            Role::TEXTURE
        };
        
        for (size_t i = 0; i < prompts.size(); ++i) {
            AIAudioGenerator::GenerationRequest request;
            request.prompt = prompts[i];
            request.role = roles[i];
            request.context.tempo = 120.0;
            request.context.key = 0;
            request.context.scale = "major";
            request.constraints.maxCPU = 0.8;
            request.constraints.maxLatency = 10.0;
            
            auto result = generator.generate(request);
            std::cout << "  " << prompts[i] << " -> Quality: " << result.qualityScore << std::endl;
        }
        
        // Example 5: System monitoring
        std::cout << "\n=== Example 5: System Monitoring ===" << std::endl;
        SystemMonitor monitor;
        monitor.startMonitoring();
        
        // Generate some audio to monitor
        for (int i = 0; i < 5; ++i) {
            auto result = generator.generate(padRequest);
            std::cout << "Generated sample " << (i + 1) << std::endl;
        }
        
        auto metrics = monitor.getMetrics();
        std::cout << "Performance Metrics:" << std::endl;
        std::cout << "  CPU Usage: " << metrics.cpuUsage << "%" << std::endl;
        std::cout << "  Memory Usage: " << metrics.memoryUsage << "%" << std::endl;
        std::cout << "  Total Renders: " << metrics.totalRenders << std::endl;
        std::cout << "  Successful Renders: " << metrics.successfulRenders << std::endl;
        
        monitor.stopMonitoring();
        
        std::cout << "\n=== AI Audio Generation Complete ===" << std::endl;
        
    } catch (const AIAudioException& e) {
        std::cerr << "AI Audio Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}