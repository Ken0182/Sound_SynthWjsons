#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <variant>
#include <cmath>

namespace aiaudio {

// Forward declarations
class DSPStage;
class DSPGraph;

// Audio buffer type
using AudioBuffer = std::vector<double>;

// Parameter value type
using ParamValue = std::variant<double, std::string, bool>;

// Parameter map
using ParamMap = std::map<std::string, ParamValue>;

// Musical context
struct MusicalContext {
    double tempo = 120.0;        // BPM
    int key = 0;                 // MIDI key (0-11)
    std::string scale = "major"; // Scale type
    double timeSignature = 4.0;  // Beats per measure
};

// Audio constraints
struct AudioConstraints {
    double maxCPU = 0.8;         // Maximum CPU usage (0.0-1.0)
    double maxLatency = 10.0;    // Maximum latency in ms
    bool noHardClips = true;     // Prevent hard clipping
    double truePeakLimit = -1.0; // True peak limit in dB
    double lufsTarget = -18.0;   // LUFS target
    double crestFactorMin = 1.0; // Minimum crest factor
    double crestFactorMax = 20.0; // Maximum crest factor
};

// Role enumeration
enum class Role {
    LEAD,
    BASS,
    PAD,
    DRUM,
    PERCUSSION,
    AMBIENT,
    TEXTURE,
    UNKNOWN
};

// Role from string conversion
inline Role roleFromString(const std::string& roleStr) {
    if (roleStr == "lead") return Role::LEAD;
    if (roleStr == "bass") return Role::BASS;
    if (roleStr == "pad") return Role::PAD;
    if (roleStr == "drum") return Role::DRUM;
    if (roleStr == "percussion") return Role::PERCUSSION;
    if (roleStr == "ambient") return Role::AMBIENT;
    if (roleStr == "texture") return Role::TEXTURE;
    return Role::UNKNOWN;
}

// Role to string conversion
inline std::string roleToString(Role role) {
    switch (role) {
        case Role::LEAD: return "lead";
        case Role::BASS: return "bass";
        case Role::PAD: return "pad";
        case Role::DRUM: return "drum";
        case Role::PERCUSSION: return "percussion";
        case Role::AMBIENT: return "ambient";
        case Role::TEXTURE: return "texture";
        default: return "unknown";
    }
}

// Stage types
enum class StageType {
    OSCILLATOR,
    FILTER,
    ENVELOPE,
    LFO,
    EFFECT,
    UNKNOWN
};

// Connection structure
struct Connection {
    std::string source;
    std::string destination;
    std::string parameter;
    double amount = 1.0;
    bool enabled = true;
};

// Exception class
class AIAudioException : public std::runtime_error {
public:
    explicit AIAudioException(const std::string& message) 
        : std::runtime_error(message) {}
};

// Utility functions
namespace utils {
    // Convert Hz to angular frequency
    inline double hzToAngular(double hz) {
        return 2.0 * M_PI * hz;
    }
    
    // Convert angular frequency to Hz
    inline double angularToHz(double angular) {
        return angular / (2.0 * M_PI);
    }
    
    // Clamp value between min and max
    template<typename T>
    T clamp(T value, T min, T max) {
        return std::max(min, std::min(max, value));
    }
    
    // Linear interpolation
    template<typename T>
    T lerp(T a, T b, double t) {
        return a + t * (b - a);
    }
    
    // Convert dB to linear
    inline double dbToLinear(double db) {
        return std::pow(10.0, db / 20.0);
    }
    
    // Convert linear to dB
    inline double linearToDb(double linear) {
        return 20.0 * std::log10(std::max(linear, 1e-10));
    }
}

} // namespace aiaudio