#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <memory>
#include <vector>
#include <string>
#include <map>

#include "main_app.h"
#include "core_types.h"
#include "dsp_ir.h"
#include "normalization.h"
#include "semantic_fusion.h"
#include "roles_policies.h"
#include "decision_heads.h"
#include "moo_optimization.h"

namespace py = pybind11;
using namespace aiaudio;

// Python-friendly wrapper for AIAudioGenerator
class PythonAIAudioGenerator {
public:
    PythonAIAudioGenerator() : generator_(std::make_unique<AIAudioGenerator>()) {}
    
    // Generate audio from a text prompt
    py::array_t<float> generate_audio(
        const std::string& prompt,
        const std::string& role = "UNKNOWN",
        double tempo = 120.0,
        const std::string& key = "C",
        const std::string& scale = "major",
        double max_cpu = 0.8,
        double max_latency = 10.0,
        bool use_semantic_search = true,
        bool apply_policies = true
    ) {
        // Convert string role to enum
        Role role_enum = Role::UNKNOWN;
        if (role == "PAD") role_enum = Role::PAD;
        else if (role == "BASS") role_enum = Role::BASS;
        else if (role == "LEAD") role_enum = Role::LEAD;
        else if (role == "FX") role_enum = Role::FX;
        else if (role == "TEXTURE") role_enum = Role::TEXTURE;
        else if (role == "ARP") role_enum = Role::ARP;
        else if (role == "DRONE") role_enum = Role::DRONE;
        else if (role == "RHYTHM") role_enum = Role::RHYTHM;
        else if (role == "BELL") role_enum = Role::BELL;
        else if (role == "CHORD") role_enum = Role::CHORD;
        else if (role == "PLUCK") role_enum = Role::PLUCK;
        
        // Create generation request
        AIAudioGenerator::GenerationRequest request;
        request.prompt = prompt;
        request.role = role_enum;
        request.context.tempo = tempo;
        request.context.key = 0; // Convert key string to int if needed
        request.context.scale = scale;
        request.constraints.maxCPU = max_cpu;
        request.constraints.maxLatency = max_latency;
        request.useSemanticSearch = use_semantic_search;
        request.applyPolicies = apply_policies;
        
        // Generate audio
        auto result = generator_->generate(request);
        
        // Convert to numpy array
        if (result.audio.empty()) {
            return py::array_t<float>(0);
        }
        
        // Create numpy array from audio buffer
        py::array_t<float> audio_array(result.audio.size());
        auto buf = audio_array.mutable_unchecked<1>();
        for (size_t i = 0; i < result.audio.size(); ++i) {
            buf(i) = result.audio[i];
        }
        
        return audio_array;
    }
    
    // Generate audio from preset parameters
    py::array_t<float> generate_from_preset(
        const std::map<std::string, py::object>& preset_params,
        double duration = 2.0,
        double sample_rate = 44100.0
    ) {
        // Create a simple DSP graph from preset parameters
        DSPGraph graph;
        
        // Add oscillator node
        DSPNode oscillator;
        oscillator.type = "oscillator";
        oscillator.parameters["frequency"] = 440.0; // Default frequency
        oscillator.parameters["waveform"] = "sine";
        
        // Extract frequency from preset if available
        if (preset_params.find("frequency") != preset_params.end()) {
            try {
                oscillator.parameters["frequency"] = preset_params.at("frequency").cast<double>();
            } catch (...) {
                // Use default if conversion fails
            }
        }
        
        graph.nodes.push_back(oscillator);
        
        // Add output node
        DSPNode output;
        output.type = "output";
        output.parameters["gain"] = 0.5;
        graph.nodes.push_back(output);
        
        // Connect oscillator to output
        DSPConnection connection;
        connection.from_node = 0;
        connection.to_node = 1;
        connection.from_port = "output";
        connection.to_port = "input";
        graph.connections.push_back(connection);
        
        // Render the graph
        size_t num_samples = static_cast<size_t>(duration * sample_rate);
        AudioBuffer audio = render_graph_simple(graph, num_samples, sample_rate);
        
        // Convert to numpy array
        py::array_t<float> audio_array(audio.size());
        auto buf = audio_array.mutable_unchecked<1>();
        for (size_t i = 0; i < audio.size(); ++i) {
            buf(i) = audio[i];
        }
        
        return audio_array;
    }
    
    // Get system status
    py::dict get_status() {
        auto status = generator_->getStatus();
        py::dict result;
        result["initialized"] = status.initialized;
        result["loaded_presets"] = status.loadedPresets;
        result["cpu_usage"] = status.cpuUsage;
        result["memory_usage"] = status.memoryUsage;
        result["active_features"] = status.activeFeatures;
        return result;
    }
    
    // Load preset from file
    bool load_preset(const std::string& file_path) {
        try {
            generator_->loadPreset(file_path);
            return true;
        } catch (...) {
            return false;
        }
    }
    
    // Get available presets
    std::vector<std::string> get_available_presets() {
        return generator_->getAvailablePresets();
    }
    
private:
    std::unique_ptr<AIAudioGenerator> generator_;
    
    // Simple graph renderer for preset-based generation
    AudioBuffer render_graph_simple(const DSPGraph& graph, size_t num_samples, double sample_rate) {
        AudioBuffer audio(num_samples);
        
        // Simple sine wave generation for now
        double frequency = 440.0;
        if (!graph.nodes.empty() && graph.nodes[0].parameters.find("frequency") != graph.nodes[0].parameters.end()) {
            frequency = graph.nodes[0].parameters.at("frequency");
        }
        
        for (size_t i = 0; i < num_samples; ++i) {
            double t = static_cast<double>(i) / sample_rate;
            audio[i] = 0.5 * std::sin(2.0 * M_PI * frequency * t);
        }
        
        return audio;
    }
};

// Python-friendly wrapper for preset data
class PythonPresetData {
public:
    PythonPresetData(const std::string& name, const std::string& category, 
                    const std::string& description, const py::dict& parameters)
        : name_(name), category_(category), description_(description) {
        // Convert Python dict to C++ map
        for (auto item : parameters) {
            std::string key = item.first.cast<std::string>();
            py::object value = item.second;
            
            // Try to convert to different types
            try {
                double dval = value.cast<double>();
                parameters_[key] = dval;
            } catch (...) {
                try {
                    std::string sval = value.cast<std::string>();
                    parameters_[key] = sval;
                } catch (...) {
                    // Store as string if all else fails
                    parameters_[key] = py::str(value);
                }
            }
        }
    }
    
    std::string get_name() const { return name_; }
    std::string get_category() const { return category_; }
    std::string get_description() const { return description_; }
    py::dict get_parameters() const {
        py::dict result;
        for (const auto& param : parameters_) {
            if (std::holds_alternative<double>(param.second)) {
                result[param.first.c_str()] = std::get<double>(param.second);
            } else {
                result[param.first.c_str()] = std::get<std::string>(param.second);
            }
        }
        return result;
    }
    
private:
    std::string name_;
    std::string category_;
    std::string description_;
    std::map<std::string, std::variant<double, std::string>> parameters_;
};

PYBIND11_MODULE(aiaudio_python, m) {
    m.doc() = "AI Audio Generator Python Bindings";
    
    // PythonAIAudioGenerator class
    py::class_<PythonAIAudioGenerator>(m, "AIAudioGenerator")
        .def(py::init<>())
        .def("generate_audio", &PythonAIAudioGenerator::generate_audio,
             py::arg("prompt"),
             py::arg("role") = "UNKNOWN",
             py::arg("tempo") = 120.0,
             py::arg("key") = "C",
             py::arg("scale") = "major",
             py::arg("max_cpu") = 0.8,
             py::arg("max_latency") = 10.0,
             py::arg("use_semantic_search") = true,
             py::arg("apply_policies") = true,
             "Generate audio from text prompt")
        .def("generate_from_preset", &PythonAIAudioGenerator::generate_from_preset,
             py::arg("preset_params"),
             py::arg("duration") = 2.0,
             py::arg("sample_rate") = 44100.0,
             "Generate audio from preset parameters")
        .def("get_status", &PythonAIAudioGenerator::get_status,
             "Get system status")
        .def("load_preset", &PythonAIAudioGenerator::load_preset,
             py::arg("file_path"),
             "Load preset from file")
        .def("get_available_presets", &PythonAIAudioGenerator::get_available_presets,
             "Get list of available presets");
    
    // PythonPresetData class
    py::class_<PythonPresetData>(m, "PresetData")
        .def(py::init<const std::string&, const std::string&, const std::string&, const py::dict&>(),
             py::arg("name"),
             py::arg("category"),
             py::arg("description"),
             py::arg("parameters"))
        .def("get_name", &PythonPresetData::get_name)
        .def("get_category", &PythonPresetData::get_category)
        .def("get_description", &PythonPresetData::get_description)
        .def("get_parameters", &PythonPresetData::get_parameters);
    
    // Enums
    py::enum_<Role>(m, "Role")
        .value("UNKNOWN", Role::UNKNOWN)
        .value("PAD", Role::PAD)
        .value("BASS", Role::BASS)
        .value("LEAD", Role::LEAD)
        .value("FX", Role::FX)
        .value("TEXTURE", Role::TEXTURE)
        .value("ARP", Role::ARP)
        .value("DRONE", Role::DRONE)
        .value("RHYTHM", Role::RHYTHM)
        .value("BELL", Role::BELL)
        .value("CHORD", Role::CHORD)
        .value("PLUCK", Role::PLUCK);
    
    // Utility functions
    m.def("create_preset_data", [](const std::string& name, const std::string& category,
                                  const std::string& description, const py::dict& parameters) {
        return std::make_unique<PythonPresetData>(name, category, description, parameters);
    }, "Create preset data object");
    
    m.def("get_version", []() {
        return "1.0.0";
    }, "Get library version");
}