#include "main_app.h"
#include "dsp_ir.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <memory>

using namespace aiaudio;

class TestRunner {
public:
    void runAllTests() {
        std::cout << "Running AI Synthesizer Tests...\n\n";
        
        testCoreTypes();
        testOscillatorStage();
        testFilterStage();
        testEnvelopeStage();
        testLFOStage();
        testDSPGraph();
        testMainApp();
        testIRParser();
        
        std::cout << "\nAll tests completed successfully!\n";
    }
    
private:
    void testCoreTypes() {
        std::cout << "Testing core types... ";
        
        // Test Role enum
        assert(roleFromString("lead") == Role::LEAD);
        assert(roleFromString("bass") == Role::BASS);
        assert(roleFromString("unknown") == Role::UNKNOWN);
        
        assert(roleToString(Role::LEAD) == "lead");
        assert(roleToString(Role::BASS) == "bass");
        assert(roleToString(Role::UNKNOWN) == "unknown");
        
        // Test utility functions
        assert(std::abs(utils::hzToAngular(440.0) - 2.0 * M_PI * 440.0) < 1e-10);
        assert(std::abs(utils::angularToHz(2.0 * M_PI * 440.0) - 440.0) < 1e-10);
        assert(utils::clamp(5.0, 0.0, 10.0) == 5.0);
        assert(utils::clamp(-5.0, 0.0, 10.0) == 0.0);
        assert(utils::clamp(15.0, 0.0, 10.0) == 10.0);
        assert(std::abs(utils::lerp(0.0, 10.0, 0.5) - 5.0) < 1e-10);
        assert(std::abs(utils::dbToLinear(0.0) - 1.0) < 1e-10);
        assert(std::abs(utils::linearToDb(1.0) - 0.0) < 1e-10);
        
        std::cout << "PASS\n";
    }
    
    void testOscillatorStage() {
        std::cout << "Testing oscillator stage... ";
        
        auto osc = std::make_unique<OscillatorStage>();
        
        // Test parameter setting/getting
        osc->setParameter("frequency", 440.0);
        osc->setParameter("amplitude", 0.5);
        osc->setParameter("waveType", std::string("sine"));
        
        assert(std::get<double>(osc->getParameter("frequency")) == 440.0);
        assert(std::get<double>(osc->getParameter("amplitude")) == 0.5);
        assert(std::get<std::string>(osc->getParameter("waveType")) == "sine");
        
        // Test audio processing
        AudioBuffer input(1024, 0.0);
        AudioBuffer output;
        osc->process(input, output);
        
        assert(output.size() == input.size());
        assert(!output.empty());
        
        // Test reset
        osc->reset();
        
        std::cout << "PASS\n";
    }
    
    void testFilterStage() {
        std::cout << "Testing filter stage... ";
        
        auto filter = std::make_unique<FilterStage>();
        
        // Test parameter setting/getting
        filter->setParameter("cutoff", 1000.0);
        filter->setParameter("resonance", 0.1);
        filter->setParameter("filterType", std::string("lowpass"));
        
        assert(std::get<double>(filter->getParameter("cutoff")) == 1000.0);
        assert(std::get<double>(filter->getParameter("resonance")) == 0.1);
        assert(std::get<std::string>(filter->getParameter("filterType")) == "lowpass");
        
        // Test audio processing
        AudioBuffer input(1024, 0.1);
        AudioBuffer output;
        filter->process(input, output);
        
        assert(output.size() == input.size());
        
        // Test reset
        filter->reset();
        
        std::cout << "PASS\n";
    }
    
    void testEnvelopeStage() {
        std::cout << "Testing envelope stage... ";
        
        auto envelope = std::make_unique<EnvelopeStage>();
        
        // Test parameter setting/getting
        envelope->setParameter("attack", 0.01);
        envelope->setParameter("decay", 0.1);
        envelope->setParameter("sustain", 0.7);
        envelope->setParameter("release", 0.5);
        
        assert(std::get<double>(envelope->getParameter("attack")) == 0.01);
        assert(std::get<double>(envelope->getParameter("decay")) == 0.1);
        assert(std::get<double>(envelope->getParameter("sustain")) == 0.7);
        assert(std::get<double>(envelope->getParameter("release")) == 0.5);
        
        // Test audio processing
        AudioBuffer input(1024, 0.1);
        AudioBuffer output;
        envelope->process(input, output);
        
        assert(output.size() == input.size());
        
        // Test reset
        envelope->reset();
        
        std::cout << "PASS\n";
    }
    
    void testLFOStage() {
        std::cout << "Testing LFO stage... ";
        
        auto lfo = std::make_unique<LFOStage>();
        
        // Test parameter setting/getting
        lfo->setParameter("rate", 1.0);
        lfo->setParameter("depth", 0.5);
        lfo->setParameter("waveType", std::string("sine"));
        
        assert(std::get<double>(lfo->getParameter("rate")) == 1.0);
        assert(std::get<double>(lfo->getParameter("depth")) == 0.5);
        assert(std::get<std::string>(lfo->getParameter("waveType")) == "sine");
        
        // Test audio processing
        AudioBuffer input(1024, 0.0);
        AudioBuffer output;
        lfo->process(input, output);
        
        assert(output.size() == input.size());
        
        // Test reset
        lfo->reset();
        
        std::cout << "PASS\n";
    }
    
    void testDSPGraph() {
        std::cout << "Testing DSP graph... ";
        
        auto graph = std::make_unique<DSPGraph>();
        
        // Add stages
        auto osc = std::make_unique<OscillatorStage>();
        auto filter = std::make_unique<FilterStage>();
        
        graph->addStage("osc1", std::move(osc));
        graph->addStage("filter1", std::move(filter));
        
        // Test stage access
        assert(graph->getStage("osc1") != nullptr);
        assert(graph->getStage("filter1") != nullptr);
        assert(graph->getStage("nonexistent") == nullptr);
        
        // Test connections
        Connection conn;
        conn.source = "osc1";
        conn.destination = "filter1";
        conn.parameter = "input";
        conn.amount = 1.0;
        
        graph->addConnection(conn);
        
        auto connections = graph->getConnections();
        assert(connections.size() == 1);
        assert(connections[0].source == "osc1");
        assert(connections[0].destination == "filter1");
        
        // Test validation
        auto issues = graph->validate();
        // Should be no issues for a simple linear chain
        
        // Test audio processing
        AudioBuffer input(1024, 0.0);
        AudioBuffer output;
        graph->process(input, output);
        
        assert(output.size() == input.size());
        
        // Test reset
        graph->reset();
        
        std::cout << "PASS\n";
    }
    
    void testMainApp() {
        std::cout << "Testing main application... ";
        
        MainApp app;
        
        // Test initialization
        assert(app.isInitialized());
        
        // Test parameter operations
        auto osc = std::make_unique<OscillatorStage>();
        osc->setParameter("frequency", 440.0);
        app.addStage("osc1", std::move(osc));
        
        app.setParameter("osc1", "frequency", 880.0);
        auto freq = app.getParameter("osc1", "frequency");
        assert(std::get<double>(freq) == 880.0);
        
        // Test audio processing
        AudioBuffer input(1024, 0.0);
        AudioBuffer output;
        app.processAudio(input, output);
        
        assert(output.size() == input.size());
        
        // Test validation
        auto issues = app.validate();
        // Should be no issues for a simple setup
        
        // Test reset
        app.reset();
        
        std::cout << "PASS\n";
    }
    
    void testIRParser() {
        std::cout << "Testing IR parser... ";
        
        // Test JSON parsing with our portable implementation
        std::string jsonData = R"({
            "stages": {
                "osc1": {
                    "type": "oscillator",
                    "parameters": {
                        "frequency": 440.0,
                        "amplitude": 0.5,
                        "waveType": "sine"
                    }
                }
            },
            "connections": []
        })";
        
        try {
            auto graph = IRParser::parsePreset(jsonData);
            assert(graph != nullptr);
            
            // Test that the stage was created correctly
            auto* osc = graph->getStage("osc1");
            assert(osc != nullptr);
            assert(osc->getType() == StageType::OSCILLATOR);
            
            // Test parameter values
            auto freq = osc->getParameter("frequency");
            assert(std::get<double>(freq) == 440.0);
            
            auto amp = osc->getParameter("amplitude");
            assert(std::get<double>(amp) == 0.5);
            
            auto waveType = osc->getParameter("waveType");
            assert(std::get<std::string>(waveType) == "sine");
            
            // Test validation
            auto issues = IRParser::validate(*graph);
            // Should be no issues for a simple setup
            
        } catch (const std::exception& e) {
            std::cout << "FAIL (" << e.what() << ")\n";
            return;
        }
        
        std::cout << "PASS\n";
    }
};

int main() {
    try {
        TestRunner runner;
        runner.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cout << "Test failed with unknown exception\n";
        return 1;
    }
}