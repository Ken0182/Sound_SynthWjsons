#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <memory>
#include <complex>
#include <chrono>
#include <random>
#include <functional>
#include <variant>
#include <optional>

namespace aiaudio {

// Core audio types
using Sample = float;
using AudioBuffer = std::vector<Sample>;
using StereoBuffer = std::array<AudioBuffer, 2>;
using ComplexSample = std::complex<Sample>;

// Time and frequency types
using TimeMs = double;
using FrequencyHz = double;
using Decibels = double;
using Ratio = double;
using Percentage = double;

// Strong typing for units
struct Hz { double value; };
struct dB { double value; };
struct Seconds { double value; };
struct Percent { double value; };

// Objective vector for MOO
struct ObjectiveVector {
    double semMatch = 0.0;        // s: Semantic match score
    double mixReadiness = 0.0;    // m: Mix readiness score
    double perceptualQuality = 0.0; // q: Perceptual quality score
    double stability = 0.0;       // σ: Stability score
    double preferenceWin = 0.0;   // p: Preference win probability
    
    double& operator[](size_t idx) {
        switch(idx) {
            case 0: return semMatch;
            case 1: return mixReadiness;
            case 2: return perceptualQuality;
            case 3: return stability;
            case 4: return preferenceWin;
            default: throw std::out_of_range("Invalid objective index");
        }
    }
    
    const double& operator[](size_t idx) const {
        return const_cast<ObjectiveVector*>(this)->operator[](idx);
    }
    
    static constexpr size_t size() { return 5; }
};

// Audio constraints
struct AudioConstraints {
    double maxCPU = 1.0;          // CPU budget (0-1)
    TimeMs maxLatency = 10.0;     // Max latency in ms
    bool noHardClips = true;      // No hard clipping allowed
    Decibels truePeakLimit = -1.0; // True peak limit in dBTP
    Decibels lufsTarget = -18.0;   // LUFS target
    double crestFactorMin = 6.0;   // Min crest factor in dB
    double crestFactorMax = 14.0;  // Max crest factor in dB
};

// Role definitions
enum class Role {
    PAD,
    BASS,
    LEAD,
    DRUM,
    PERCUSSION,
    AMBIENT,
    TEXTURE,
    UNKNOWN
};

// Musical context
struct MusicalContext {
    FrequencyHz tempo = 120.0;    // BPM
    int key = 0;                  // MIDI key (0-11, C=0)
    std::string scale = "major";  // Scale type
    TimeMs timeSignature = 4.0;   // Time signature numerator
};

// Decision vector for ML heads
struct DecisionVector {
    std::vector<double> values;   // μ values [0,1]
    std::vector<bool> routes;     // Routing mask
    double confidence = 0.0;      // Overall confidence
};

// Trace for reproducibility
struct Trace {
    std::string prompt;
    std::string queryHash;
    std::string entryId;
    DecisionVector decisions;
    std::string policyVersion;
    std::string budgetTier;
    std::map<std::string, double> meters;
    uint32_t seed = 0;
    std::chrono::system_clock::time_point timestamp;
};

// Error handling
class AIAudioException : public std::exception {
public:
    explicit AIAudioException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override { return message_.c_str(); }
private:
    std::string message_;
};

// Utility functions
inline Hz midiToFreq(int midiNote) {
    return Hz{440.0 * std::pow(2.0, (midiNote - 69) / 12.0)};
}

inline dB linearToDb(double linear) {
    return dB{20.0 * std::log10(std::max(linear, 1e-10))};
}

inline double dbToLinear(dB db) {
    return std::pow(10.0, db.value / 20.0);
}

inline Percent percentToLinear(Percent p) {
    return Percent{p.value / 100.0};
}

} // namespace aiaudio