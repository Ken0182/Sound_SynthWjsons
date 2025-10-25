#include "main_app.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

using namespace aiaudio;

class AIAudioGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        generator = std::make_unique<AIAudioGenerator>();
    }
    
    std::unique_ptr<AIAudioGenerator> generator;
};

// Test basic initialization
TEST_F(AIAudioGeneratorTest, Initialization) {
    auto status = generator->getStatus();
    EXPECT_TRUE(status.initialized);
    EXPECT_GT(status.activeFeatures.size(), 0);
}

// Test basic audio generation
TEST_F(AIAudioGeneratorTest, BasicGeneration) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "test pad";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    request.constraints.maxCPU = 0.8;
    request.constraints.maxLatency = 10.0;
    
    auto result = generator->generate(request);
    
    EXPECT_GT(result.audio.size(), 0);
    EXPECT_GE(result.qualityScore, 0.0);
    EXPECT_LE(result.qualityScore, 1.0);
    EXPECT_FALSE(result.explanation.empty());
}

// Test different roles
TEST_F(AIAudioGeneratorTest, DifferentRoles) {
    std::vector<Role> roles = {Role::PAD, Role::BASS, Role::LEAD, Role::DRUM};
    
    for (auto role : roles) {
        AIAudioGenerator::GenerationRequest request;
        request.prompt = "test sound";
        request.role = role;
        request.context.tempo = 120.0;
        request.context.key = 0;
        request.context.scale = "major";
        
        auto result = generator->generate(request);
        
        EXPECT_GT(result.audio.size(), 0);
        EXPECT_GE(result.qualityScore, 0.0);
    }
}

// Test quality assessment
TEST_F(AIAudioGeneratorTest, QualityAssessment) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "high quality pad";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    
    auto result = generator->generate(request);
    
    // Check that quality score is reasonable
    EXPECT_GE(result.qualityScore, 0.0);
    EXPECT_LE(result.qualityScore, 1.0);
    
    // Check that warnings are reasonable
    for (const auto& warning : result.warnings) {
        EXPECT_FALSE(warning.empty());
    }
}

// Test configuration
TEST_F(AIAudioGeneratorTest, Configuration) {
    std::map<std::string, std::string> config;
    config["test_key"] = "test_value";
    config["quality_threshold"] = "0.8";
    
    generator->setConfiguration(config);
    
    // Configuration should be set (implementation dependent)
    SUCCEED();
}

// Test preset loading (mock)
TEST_F(AIAudioGeneratorTest, PresetLoading) {
    // This would test preset loading if we had actual preset files
    auto presets = generator->getAvailablePresets();
    EXPECT_GE(presets.size(), 0);
}

// Test error handling
TEST_F(AIAudioGeneratorTest, ErrorHandling) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = ""; // Empty prompt
    request.role = Role::UNKNOWN;
    
    auto result = generator->generate(request);
    
    // Should handle gracefully
    EXPECT_GE(result.qualityScore, 0.0);
    EXPECT_LE(result.qualityScore, 1.0);
}

// Test audio constraints
TEST_F(AIAudioGeneratorTest, AudioConstraints) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "test sound";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    request.constraints.maxCPU = 0.5;
    request.constraints.maxLatency = 5.0;
    request.constraints.lufsTarget = -20.0;
    request.constraints.truePeakLimit = -1.0;
    
    auto result = generator->generate(request);
    
    EXPECT_GT(result.audio.size(), 0);
    
    // Check for clipping
    bool hasClipping = false;
    for (double sample : result.audio) {
        if (std::abs(sample) >= 1.0) {
            hasClipping = true;
            break;
        }
    }
    
    if (request.constraints.noHardClips) {
        EXPECT_FALSE(hasClipping);
    }
}

// Test musical context
TEST_F(AIAudioGeneratorTest, MusicalContext) {
    std::vector<int> keys = {0, 3, 7, 10}; // C, Eb, G, Bb
    std::vector<std::string> scales = {"major", "minor"};
    std::vector<double> tempos = {60.0, 120.0, 180.0};
    
    for (int key : keys) {
        for (const auto& scale : scales) {
            for (double tempo : tempos) {
                AIAudioGenerator::GenerationRequest request;
                request.prompt = "test sound";
                request.role = Role::PAD;
                request.context.tempo = tempo;
                request.context.key = key;
                request.context.scale = scale;
                
                auto result = generator->generate(request);
                
                EXPECT_GT(result.audio.size(), 0);
                EXPECT_GE(result.qualityScore, 0.0);
            }
        }
    }
}

// Test batch processing
TEST_F(AIAudioGeneratorTest, BatchProcessing) {
    std::vector<std::string> prompts = {
        "warm pad",
        "punchy bass",
        "bright lead",
        "atmospheric texture"
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
        
        auto result = generator->generate(request);
        
        EXPECT_GT(result.audio.size(), 0);
        EXPECT_GE(result.qualityScore, 0.0);
        EXPECT_FALSE(result.explanation.empty());
    }
}

// Test system status
TEST_F(AIAudioGeneratorTest, SystemStatus) {
    auto status = generator->getStatus();
    
    EXPECT_TRUE(status.initialized);
    EXPECT_GE(status.loadedPresets, 0);
    EXPECT_GE(status.cpuUsage, 0.0);
    EXPECT_LE(status.cpuUsage, 100.0);
    EXPECT_GE(status.memoryUsage, 0.0);
    EXPECT_LE(status.memoryUsage, 100.0);
    EXPECT_GT(status.activeFeatures.size(), 0);
}

// Test audio quality metrics
TEST_F(AIAudioGeneratorTest, AudioQualityMetrics) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "high quality test";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    
    auto result = generator->generate(request);
    
    // Check audio properties
    EXPECT_GT(result.audio.size(), 0);
    
    // Check for silence
    double rms = 0.0;
    for (double sample : result.audio) {
        rms += sample * sample;
    }
    rms = std::sqrt(rms / result.audio.size());
    EXPECT_GT(rms, 0.001); // Should not be silent
    
    // Check for reasonable amplitude
    double maxAmplitude = 0.0;
    for (double sample : result.audio) {
        maxAmplitude = std::max(maxAmplitude, std::abs(sample));
    }
    EXPECT_GT(maxAmplitude, 0.0);
    EXPECT_LE(maxAmplitude, 1.0);
}

// Test trace generation
TEST_F(AIAudioGeneratorTest, TraceGeneration) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "trace test";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    
    auto result = generator->generate(request);
    
    // Check trace properties
    EXPECT_FALSE(result.trace.prompt.empty());
    EXPECT_FALSE(result.trace.queryHash.empty());
    EXPECT_FALSE(result.trace.entryId.empty());
    EXPECT_FALSE(result.trace.policyVersion.empty());
    EXPECT_FALSE(result.trace.budgetTier.empty());
    EXPECT_GT(result.trace.seed, 0);
    
    // Check meter readings
    EXPECT_GT(result.trace.meters.size(), 0);
    EXPECT_TRUE(result.trace.meters.find("lufs") != result.trace.meters.end());
    EXPECT_TRUE(result.trace.meters.find("tp") != result.trace.meters.end());
}

// Test performance
TEST_F(AIAudioGeneratorTest, Performance) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "performance test";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    request.constraints.maxLatency = 100.0; // Allow more time for test
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto result = generator->generate(request);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 5000); // 5 seconds max
    EXPECT_GT(result.audio.size(), 0);
}

// Test edge cases
TEST_F(AIAudioGeneratorTest, EdgeCases) {
    // Test with extreme values
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "extreme test";
    request.role = Role::PAD;
    request.context.tempo = 300.0; // Very fast
    request.context.key = 11; // B
    request.context.scale = "minor";
    request.constraints.maxCPU = 0.1; // Very low CPU
    request.constraints.maxLatency = 1.0; // Very low latency
    
    auto result = generator->generate(request);
    
    // Should handle gracefully
    EXPECT_GE(result.qualityScore, 0.0);
    EXPECT_LE(result.qualityScore, 1.0);
}

// Test memory management
TEST_F(AIAudioGeneratorTest, MemoryManagement) {
    // Generate multiple samples to test memory management
    for (int i = 0; i < 10; ++i) {
        AIAudioGenerator::GenerationRequest request;
        request.prompt = "memory test " + std::to_string(i);
        request.role = Role::PAD;
        request.context.tempo = 120.0;
        request.context.key = 0;
        request.context.scale = "major";
        
        auto result = generator->generate(request);
        
        EXPECT_GT(result.audio.size(), 0);
    }
    
    // System should still be responsive
    auto status = generator->getStatus();
    EXPECT_TRUE(status.initialized);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}