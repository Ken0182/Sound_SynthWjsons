/**
 * Python bindings for AI Audio Generator
 * Exposes C++ audio rendering engine to Python via pybind11
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "main_app.h"
#include "core_types.h"

namespace py = pybind11;
using namespace aiaudio;

/**
 * Helper function to convert Python dict to GenerationRequest
 */
AIAudioGenerator::GenerationRequest dictToRequest(const py::dict& preset_dict, 
                                                   const py::dict& context_dict,
                                                   double duration) {
    AIAudioGenerator::GenerationRequest request;
    
    // Extract prompt and role
    if (preset_dict.contains("prompt")) {
        request.prompt = preset_dict["prompt"].cast<std::string>();
    }
    if (preset_dict.contains("description")) {
        request.prompt = preset_dict["description"].cast<std::string>();
    }
    
    // Parse role
    if (preset_dict.contains("role")) {
        std::string role_str = preset_dict["role"].cast<std::string>();
        if (role_str == "pad") request.role = Role::PAD;
        else if (role_str == "bass") request.role = Role::BASS;
        else if (role_str == "lead") request.role = Role::LEAD;
        else if (role_str == "drum") request.role = Role::DRUM;
        else if (role_str == "fx") request.role = Role::FX;
        else request.role = Role::UNKNOWN;
    }
    
    // Extract musical context
    if (context_dict.contains("tempo")) {
        request.context.tempo = context_dict["tempo"].cast<double>();
    }
    if (context_dict.contains("key")) {
        request.context.key = context_dict["key"].cast<int>();
    }
    if (context_dict.contains("scale")) {
        request.context.scale = context_dict["scale"].cast<std::string>();
    }
    if (context_dict.contains("time_signature")) {
        auto ts = context_dict["time_signature"].cast<std::vector<int>>();
        if (ts.size() == 2) {
            request.context.timeSignature = {ts[0], ts[1]};
        }
    }
    
    // Set constraints
    if (preset_dict.contains("constraints")) {
        auto constraints = preset_dict["constraints"].cast<py::dict>();
        if (constraints.contains("max_cpu")) {
            request.constraints.maxCPU = constraints["max_cpu"].cast<double>();
        }
        if (constraints.contains("max_latency")) {
            request.constraints.maxLatency = constraints["max_latency"].cast<double>();
        }
        if (constraints.contains("lufs_target")) {
            request.constraints.lufsTarget = constraints["lufs_target"].cast<double>();
        }
        if (constraints.contains("true_peak_limit")) {
            request.constraints.truePeakLimit = constraints["true_peak_limit"].cast<double>();
        }
    } else {
        // Set sensible defaults
        request.constraints.maxCPU = 0.8;
        request.constraints.maxLatency = 10.0;
        request.constraints.lufsTarget = -18.0;
        request.constraints.truePeakLimit = -1.0;
    }
    
    return request;
}

/**
 * Helper function to convert audio buffer to NumPy array
 */
py::array_t<float> audioBufferToNumPy(const AudioBuffer& buffer) {
    if (buffer.empty()) {
        return py::array_t<float>({0, 2});
    }
    
    // Calculate number of frames (stereo)
    size_t numFrames = buffer.size();
    
    // Create NumPy array with shape (numFrames, 2) for stereo
    py::array_t<float> result({static_cast<py::ssize_t>(numFrames), 2});
    
    // Get buffer pointer
    auto buf = result.request();
    float* ptr = static_cast<float*>(buf.ptr);
    
    // Copy mono to stereo
    for (size_t i = 0; i < numFrames; ++i) {
        ptr[i * 2 + 0] = static_cast<float>(buffer[i]); // Left channel
        ptr[i * 2 + 1] = static_cast<float>(buffer[i]); // Right channel
    }
    
    return result;
}

/**
 * Helper function to convert NumPy array to audio buffer
 */
AudioBuffer numpyToAudioBuffer(py::array_t<float> array) {
    auto buf = array.request();
    
    if (buf.ndim != 2 || buf.shape[1] != 2) {
        throw std::runtime_error("Expected audio array with shape (n, 2)");
    }
    
    size_t numFrames = buf.shape[0];
    float* ptr = static_cast<float*>(buf.ptr);
    
    AudioBuffer buffer(numFrames);
    
    // Convert stereo to mono by averaging channels
    for (size_t i = 0; i < numFrames; ++i) {
        buffer[i] = (ptr[i * 2 + 0] + ptr[i * 2 + 1]) / 2.0;
    }
    
    return buffer;
}

/**
 * C++ Engine wrapper class for Python
 */
class CPPAudioEngine {
public:
    CPPAudioEngine() {
        generator_ = std::make_unique<AIAudioGenerator>();
    }
    
    py::array_t<float> render_audio(const py::dict& preset_dict,
                                     const py::dict& context_dict,
                                     double duration = 2.0) {
        // Release GIL during C++ processing
        py::gil_scoped_release release;
        
        // Convert Python dict to C++ request
        auto request = dictToRequest(preset_dict, context_dict, duration);
        
        // Generate audio
        auto result = generator_->generate(request);
        
        // Reacquire GIL before returning to Python
        py::gil_scoped_acquire acquire;
        
        // Convert to NumPy array
        return audioBufferToNumPy(result.audio);
    }
    
    py::dict assess_quality(py::array_t<float> audio,
                           const std::string& role,
                           const py::dict& context_dict) {
        // Release GIL during C++ processing
        py::gil_scoped_release release;
        
        // Convert NumPy to audio buffer
        AudioBuffer buffer = numpyToAudioBuffer(audio);
        
        // Parse role
        Role roleEnum = Role::UNKNOWN;
        if (role == "pad") roleEnum = Role::PAD;
        else if (role == "bass") roleEnum = Role::BASS;
        else if (role == "lead") roleEnum = Role::LEAD;
        else if (role == "drum") roleEnum = Role::DRUM;
        else if (role == "fx") roleEnum = Role::FX;
        
        // Create request for quality assessment
        AIAudioGenerator::GenerationRequest request;
        request.role = roleEnum;
        
        if (context_dict.contains("tempo")) {
            request.context.tempo = context_dict["tempo"].cast<double>();
        }
        if (context_dict.contains("key")) {
            request.context.key = context_dict["key"].cast<int>();
        }
        if (context_dict.contains("scale")) {
            request.context.scale = context_dict["scale"].cast<std::string>();
        }
        
        // Assess quality
        double quality = generator_->assessQuality(buffer, request);
        
        // Reacquire GIL before creating Python dict
        py::gil_scoped_acquire acquire;
        
        // Create result dict
        py::dict result;
        result["overall_score"] = quality;
        result["semantic_match"] = quality * 0.9;
        result["mix_readiness"] = quality * 1.1;
        result["perceptual_quality"] = quality;
        result["stability"] = quality * 1.05;
        
        // Add empty violations and warnings lists
        result["violations"] = py::list();
        result["warnings"] = py::list();
        
        return result;
    }
    
    py::dict get_status() const {
        auto status = generator_->getStatus();
        
        py::dict result;
        result["initialized"] = status.initialized;
        result["loaded_presets"] = status.loadedPresets;
        result["cpu_usage"] = status.cpuUsage;
        result["memory_usage"] = status.memoryUsage;
        
        py::list features;
        for (const auto& feature : status.activeFeatures) {
            features.append(feature);
        }
        result["active_features"] = features;
        
        return result;
    }
    
    void load_preset(const std::string& preset_path) {
        generator_->loadPreset(preset_path);
    }
    
    py::list get_available_presets() const {
        auto presets = generator_->getAvailablePresets();
        
        py::list result;
        for (const auto& preset : presets) {
            result.append(preset);
        }
        return result;
    }
    
    void set_configuration(const py::dict& config) {
        std::map<std::string, std::string> cpp_config;
        for (auto item : config) {
            std::string key = py::str(item.first);
            std::string value = py::str(item.second);
            cpp_config[key] = value;
        }
        generator_->setConfiguration(cpp_config);
    }
    
private:
    std::unique_ptr<AIAudioGenerator> generator_;
};

/**
 * pybind11 module definition
 */
PYBIND11_MODULE(aiaudio_cpp, m) {
    m.doc() = "AI Audio Generator C++ Engine - Python bindings";
    
    // Main engine class
    py::class_<CPPAudioEngine>(m, "CPPAudioEngine")
        .def(py::init<>())
        .def("render_audio", &CPPAudioEngine::render_audio,
             py::arg("preset_dict"),
             py::arg("context_dict"),
             py::arg("duration") = 2.0,
             "Render audio from a preset using the C++ engine")
        .def("assess_quality", &CPPAudioEngine::assess_quality,
             py::arg("audio"),
             py::arg("role"),
             py::arg("context_dict"),
             "Assess audio quality using MOO optimizer")
        .def("get_status", &CPPAudioEngine::get_status,
             "Get system status")
        .def("load_preset", &CPPAudioEngine::load_preset,
             py::arg("preset_path"),
             "Load preset from file")
        .def("get_available_presets", &CPPAudioEngine::get_available_presets,
             "Get list of available presets")
        .def("set_configuration", &CPPAudioEngine::set_configuration,
             py::arg("config"),
             "Set system configuration");
    
    // Expose Role enum
    py::enum_<Role>(m, "Role")
        .value("UNKNOWN", Role::UNKNOWN)
        .value("PAD", Role::PAD)
        .value("BASS", Role::BASS)
        .value("LEAD", Role::LEAD)
        .value("DRUM", Role::DRUM)
        .value("FX", Role::FX)
        .export_values();
    
    // Module-level functions
    m.def("get_version", []() { return "1.0.0"; }, "Get version string");
    m.def("get_sample_rate", []() { return 44100; }, "Get default sample rate");
}
