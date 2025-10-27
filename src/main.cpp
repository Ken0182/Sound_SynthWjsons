#include "core_types.h"
#include "dsp_ir.h"
#include <iostream>
#include <string>
#include <vector>

using namespace aiaudio;

int main(int argc, char* argv[]) {
    try {
        std::cout << "AI Audio Generator - Basic Test" << std::endl;
        std::cout << "==============================" << std::endl;
        
        // Test basic DSP functionality
        std::cout << "\nTesting DSP Graph Creation..." << std::endl;
        
        // Create a simple oscillator
        auto osc = std::make_unique<OscillatorStage>();
        osc->setParameter("frequency", 440.0);
        osc->setParameter("amplitude", 0.5);
        osc->setParameter("waveType", std::string("sine"));
        
        std::cout << "Created oscillator: " << osc->getDescription() << std::endl;
        
        // Create a simple filter
        auto filter = std::make_unique<FilterStage>();
        filter->setParameter("cutoff", 1000.0);
        filter->setParameter("resonance", 0.3);
        
        std::cout << "Created filter: " << filter->getDescription() << std::endl;
        
        // Create a DSP graph
        DSPGraph graph;
        graph.addStage("osc1", std::move(osc));
        graph.addStage("filter1", std::move(filter));
        
        std::cout << "Created DSP graph with " << graph.getStageNames().size() << " stages" << std::endl;
        
        // Test JSON parsing
        std::cout << "\nTesting JSON Parsing..." << std::endl;
        
        std::string testJson = R"({
            "stages": {
                "test_osc": {
                    "type": "oscillator",
                    "parameters": {
                        "frequency": 880.0,
                        "amplitude": 0.7,
                        "waveType": "square"
                    }
                }
            },
            "connections": []
        })";
        
        IRParser parser;
        auto parsedGraph = parser.parsePreset(testJson);
        std::cout << "Parsed JSON graph with " << parsedGraph->getStageNames().size() << " stages" << std::endl;
        
        // Test audio processing
        std::cout << "\nTesting Audio Processing..." << std::endl;
        
        AudioBuffer input(1024, 0.0); // Silent input
        AudioBuffer output;
        
        graph.process(input, output);
        std::cout << "Processed " << input.size() << " input samples to " << output.size() << " output samples" << std::endl;
        
        // Calculate some basic statistics
        double rms = 0.0;
        double peak = 0.0;
        for (const auto& sample : output) {
            rms += sample * sample;
            peak = std::max(peak, static_cast<double>(std::abs(sample)));
        }
        rms = std::sqrt(rms / output.size());
        
        std::cout << "Output RMS: " << rms << std::endl;
        std::cout << "Output Peak: " << peak << std::endl;
        
        std::cout << "\n=== Basic Test Complete ===" << std::endl;
        std::cout << "The AI Audio Generator core functionality is working!" << std::endl;
        
    } catch (const AIAudioException& e) {
        std::cerr << "AI Audio Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}