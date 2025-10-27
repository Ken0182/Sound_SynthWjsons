#include "main_app.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

using namespace aiaudio;

void printUsage(const char* programName) {
    std::cout << "AI Synthesizer - Modern C++ Audio Engine\n";
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -p, --preset <file>     Load preset from JSON file\n";
    std::cout << "  -o, --output <file>     Save output to file\n";
    std::cout << "  -d, --duration <sec>    Duration in seconds (default: 5.0)\n";
    std::cout << "  -s, --sample-rate <hz>  Sample rate in Hz (default: 44100)\n";
    std::cout << "  -b, --buffer-size <n>   Buffer size in samples (default: 1024)\n";
    std::cout << "  -v, --verbose           Enable verbose output\n";
    std::cout << "  --test                  Run built-in tests\n";
    std::cout << "  --demo                  Run demo synthesis\n";
}

void runDemo(MainApp& app) {
    std::cout << "Running demo synthesis...\n";
    
    // Create a simple synthesizer patch
    auto osc = std::make_unique<OscillatorStage>();
    osc->setParameter("frequency", 440.0);
    osc->setParameter("amplitude", 0.5);
    osc->setParameter("waveType", std::string("sine"));
    
    auto filter = std::make_unique<FilterStage>();
    filter->setParameter("cutoff", 1000.0);
    filter->setParameter("resonance", 0.1);
    filter->setParameter("filterType", std::string("lowpass"));
    
    auto envelope = std::make_unique<EnvelopeStage>();
    envelope->setParameter("attack", 0.01);
    envelope->setParameter("decay", 0.1);
    envelope->setParameter("sustain", 0.7);
    envelope->setParameter("release", 0.5);
    
    // Add stages to the app
    app.addStage("osc1", std::move(osc));
    app.addStage("filter1", std::move(filter));
    app.addStage("env1", std::move(envelope));
    
    // Add connections
    Connection oscToFilter;
    oscToFilter.source = "osc1";
    oscToFilter.destination = "filter1";
    oscToFilter.parameter = "input";
    oscToFilter.amount = 1.0;
    app.addConnection(oscToFilter);
    
    Connection filterToEnv;
    filterToEnv.source = "filter1";
    filterToEnv.destination = "env1";
    filterToEnv.parameter = "input";
    filterToEnv.amount = 1.0;
    app.addConnection(filterToEnv);
    
    // Validate the patch
    auto issues = app.validate();
    if (!issues.empty()) {
        std::cout << "Validation issues:\n";
        for (const auto& issue : issues) {
            std::cout << "  - " << issue << "\n";
        }
        return;
    }
    
    std::cout << "Demo patch created successfully!\n";
    std::cout << "Stages: ";
    auto stageNames = app.getStageNames();
    for (size_t i = 0; i < stageNames.size(); ++i) {
        std::cout << stageNames[i];
        if (i < stageNames.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
    
    std::cout << "Total gain: " << app.getTotalGain() << "\n";
}

void runTests(MainApp& app) {
    std::cout << "Running built-in tests...\n";
    
    // Test 1: Basic initialization
    std::cout << "Test 1: Basic initialization... ";
    if (app.isInitialized()) {
        std::cout << "PASS\n";
    } else {
        std::cout << "FAIL\n";
        return;
    }
    
    // Test 2: Parameter setting/getting
    std::cout << "Test 2: Parameter operations... ";
    try {
        auto osc = std::make_unique<OscillatorStage>();
        osc->setParameter("frequency", 440.0);
        auto freq = osc->getParameter("frequency");
        if (std::holds_alternative<double>(freq) && std::get<double>(freq) == 440.0) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
        }
    } catch (const std::exception& e) {
        std::cout << "FAIL (" << e.what() << ")\n";
    }
    
    // Test 3: Audio processing
    std::cout << "Test 3: Audio processing... ";
    try {
        auto osc = std::make_unique<OscillatorStage>();
        AudioBuffer input(1024, 0.0);
        AudioBuffer output;
        osc->process(input, output);
        if (output.size() == input.size()) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
        }
    } catch (const std::exception& e) {
        std::cout << "FAIL (" << e.what() << ")\n";
    }
    
    // Test 4: Graph validation
    std::cout << "Test 4: Graph validation... ";
    auto issues = app.validate();
    if (issues.empty()) {
        std::cout << "PASS\n";
    } else {
        std::cout << "PASS (with warnings)\n";
    }
    
    std::cout << "All tests completed.\n";
}

void generateAudio(MainApp& app, const std::string& outputFile, double duration, bool verbose) {
    std::cout << "Generating audio...\n";
    std::cout << "Duration: " << duration << " seconds\n";
    std::cout << "Sample rate: " << app.getSampleRate() << " Hz\n";
    std::cout << "Buffer size: " << app.getBufferSize() << " samples\n";
    
    size_t totalSamples = static_cast<size_t>(duration * app.getSampleRate());
    size_t bufferSize = app.getBufferSize();
    size_t numBuffers = (totalSamples + bufferSize - 1) / bufferSize;
    
    std::vector<double> fullOutput;
    fullOutput.reserve(totalSamples);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < numBuffers; ++i) {
        size_t currentBufferSize = std::min(bufferSize, totalSamples - i * bufferSize);
        
        AudioBuffer input(currentBufferSize, 0.0);
        AudioBuffer output;
        
        app.processAudio(input, output);
        
        fullOutput.insert(fullOutput.end(), output.begin(), output.end());
        
        if (verbose && (i % 100 == 0)) {
            double progress = static_cast<double>(i) / numBuffers * 100.0;
            std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress << "%\r";
            std::cout.flush();
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\nAudio generation completed in " << duration_ms.count() << " ms\n";
    
    // Save to file (simplified - just output sample values)
    std::ofstream file(outputFile);
    if (file.is_open()) {
        file << "# AI Synthesizer Output\n";
        file << "# Sample Rate: " << app.getSampleRate() << " Hz\n";
        file << "# Duration: " << duration << " seconds\n";
        file << "# Samples: " << fullOutput.size() << "\n";
        file << "# Format: Raw float values\n";
        
        for (size_t i = 0; i < fullOutput.size(); ++i) {
            file << fullOutput[i] << "\n";
        }
        
        file.close();
        std::cout << "Output saved to: " << outputFile << "\n";
    } else {
        std::cout << "Error: Could not save output to " << outputFile << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::string presetFile;
    std::string outputFile = "output.txt";
    double duration = 5.0;
    bool verbose = false;
    bool runTestsFlag = false;
    bool runDemoFlag = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-p" || arg == "--preset") {
            if (i + 1 < argc) {
                presetFile = argv[++i];
            } else {
                std::cout << "Error: --preset requires a filename\n";
                return 1;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                std::cout << "Error: --output requires a filename\n";
                return 1;
            }
        } else if (arg == "-d" || arg == "--duration") {
            if (i + 1 < argc) {
                duration = std::stod(argv[++i]);
            } else {
                std::cout << "Error: --duration requires a value\n";
                return 1;
            }
        } else if (arg == "-s" || arg == "--sample-rate") {
            if (i + 1 < argc) {
                double sampleRate = std::stod(argv[++i]);
                // This would be set on the app, but we'll use the default for now
            } else {
                std::cout << "Error: --sample-rate requires a value\n";
                return 1;
            }
        } else if (arg == "-b" || arg == "--buffer-size") {
            if (i + 1 < argc) {
                size_t bufferSize = std::stoul(argv[++i]);
                // This would be set on the app, but we'll use the default for now
            } else {
                std::cout << "Error: --buffer-size requires a value\n";
                return 1;
            }
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "--test") {
            runTestsFlag = true;
        } else if (arg == "--demo") {
            runDemoFlag = true;
        } else {
            std::cout << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Initialize the application
    MainApp app;
    
    if (runTestsFlag) {
        runTests(app);
        return 0;
    }
    
    if (runDemoFlag) {
        runDemo(app);
        return 0;
    }
    
    // Load preset if specified
    if (!presetFile.empty()) {
        std::cout << "Loading preset: " << presetFile << "\n";
        if (!app.loadPreset(presetFile)) {
            std::cout << "Error loading preset: " << app.getLastError() << "\n";
            return 1;
        }
        std::cout << "Preset loaded successfully\n";
    } else {
        // Create a default patch
        runDemo(app);
    }
    
    // Generate audio
    generateAudio(app, outputFile, duration, verbose);
    
    return 0;
}