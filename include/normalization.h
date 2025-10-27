#pragma once

#include "core_types.h"
#include "dsp_ir.h"
#include <map>
#include <functional>

namespace aiaudio {

// Canonical Normalization & Perceptual Mappings

class Normalizer {
public:
    // MIDI to frequency conversion
    static Hz midiToFreq(int midiNote) { return aiaudio::midiToFreq(midiNote); }
    
    // Frequency to MIDI conversion
    static int freqToMidi(Hz freq) { return static_cast<int>(12.0 * std::log2(freq.value / 440.0) + 69); }
    
    // Percent to perceptual mapping (log/ERB transforms)
    static double percentToPerceptual(Percent percent) { return 21.4 * std::log10(1.0 + 0.00437 * percent.value); }
    
    // Perceptual to percent mapping
    static Percent perceptualToPercent(double perceptual) { return Percent{(std::pow(10.0, perceptual / 21.4) - 1.0) / 0.00437}; }
    
    // Amplitude dB to linear conversion
    static double dbToLinear(dB db) { return std::pow(10.0, db.value / 20.0); }
    
    // Linear to dB conversion
    static dB linearToDb(double linear) { return dB{20.0 * std::log10(std::max(linear, 1e-10))}; }
    
    // Safe clamping functions
    static double softLimit(double value, double threshold) { return (std::abs(value) <= threshold) ? value : threshold * std::tanh(value / threshold); }
    
    static double hardLimit(double value, double minVal, double maxVal) { return std::clamp(value, minVal, maxVal); }
    
    // Anti-denormal offset
    static double addAntiDenormal(double value, double offset = 1e-20) { return value + offset; }
    
    // Normalize audio buffer
    static void normalizeBuffer(AudioBuffer& buffer, dB targetLevel = dB{-18.0}) {
        if (buffer.empty()) return;
        
        // Find peak
        double peak = 0.0;
        for (auto sample : buffer) {
            double d = static_cast<double>(sample);
            peak = std::max(peak, std::abs(d));
        }
        
        if (peak > 0.0) {
            double targetLinear = dbToLinear(targetLevel);
            double gain = targetLinear / peak;
            
            for (auto& sample : buffer) {
                sample = static_cast<float>(static_cast<double>(sample) * gain);
            }
        }
    }
    
    // RMS normalization
    static void rmsNormalize(AudioBuffer& buffer, dB targetLevel = dB{-18.0}) {
        if (buffer.empty()) return;
        
        // Calculate RMS
        double sum = 0.0;
        for (auto sample : buffer) {
            double d = static_cast<double>(sample);
            sum += d * d;
        }
        double rms = std::sqrt(sum / buffer.size());
        
        if (rms > 0.0) {
            double targetLinear = dbToLinear(targetLevel);
            double gain = targetLinear / rms;
            
            for (auto& sample : buffer) {
                sample = static_cast<float>(static_cast<double>(sample) * gain);
            }
        }
    }
};

// Preset normalization system
class PresetNormalizer {
public:
    struct NormalizedPreset {
        std::map<std::string, double> parameters;
        std::map<std::string, std::string> metadata;
        std::string version;
        std::chrono::system_clock::time_point timestamp;
    };
    
    // Normalize a DSP preset
    NormalizedPreset normalize(const DSPGraph& graph, Role role) const;
    
    // Validate normalized preset
    std::vector<std::string> validate(const NormalizedPreset& preset) const;
    
    // Load role-specific normalization rules
    void loadRoleRules(Role role, const std::map<std::string, double>& rules);
    
    // Apply perceptual mappings
    void applyPerceptualMappings(NormalizedPreset& preset, Role role) const;
    
    // Ensure parameter stability
    void ensureStability(NormalizedPreset& preset) const;
    
private:
    // Role-specific normalization rules
    std::map<Role, std::map<std::string, double>> roleRules_;
    
    // Parameter mapping functions
    double mapFrequency(double freq, Role role) const;
    double mapAmplitude(double amp, Role role) const;
    double mapTime(double time, Role role) const;
    double mapRatio(double ratio, Role role) const;
    
    // Validation helpers
    bool isValidFrequency(double freq) const;
    bool isValidAmplitude(double amp) const;
    bool isValidTime(double time) const;
    bool isValidRatio(double ratio) const;
    
    // Stability checks
    bool checkFeedbackStability(const NormalizedPreset& preset) const;
    bool checkGainStability(const NormalizedPreset& preset) const;
    bool checkPhaseStability(const NormalizedPreset& preset) const;
};

// Perceptual mapping utilities
class PerceptualMapper {
public:
    // Bark scale conversion
    static double hzToBark(Hz freq) {
        return 13.0 * std::atan(0.00076 * freq.value) + 3.5 * std::atan(std::pow(freq.value / 7500.0, 2));
    }
    
    static Hz barkToHz(double bark) {
        // Inverse of hzToBark (approximate)
        return Hz{7500.0 * std::sinh(bark / 3.5)};
    }
    
    // Mel scale conversion
    static double hzToMel(Hz freq) {
        return 2595.0 * std::log10(1.0 + freq.value / 700.0);
    }
    
    static Hz melToHz(double mel) {
        return Hz{700.0 * (std::pow(10.0, mel / 2595.0) - 1.0)};
    }
    
    // Critical band rate
    static double criticalBandRate(Hz freq) {
        return 21.4 * std::log10(1.0 + 0.00437 * freq.value);
    }
    
    // Loudness weighting (A-weighting)
    static double aWeighting(Hz freq) {
        double f = freq.value;
        double f2 = f * f;
        double f4 = f2 * f2;
        return (12194.0 * 12194.0 * f4) / 
               ((f2 + 20.6 * 20.6) * std::sqrt((f2 + 107.7 * 107.7) * (f2 + 737.9 * 737.9)) * (f2 + 12194.0 * 12194.0));
    }
    
    // C-weighting
    static double cWeighting(Hz freq) {
        double f = freq.value;
        double f2 = f * f;
        return (12194.0 * 12194.0 * f2) / ((f2 + 20.6 * 20.6) * (f2 + 12194.0 * 12194.0));
    }
    
    // K-weighting (for LUFS)
    static double kWeighting(Hz freq) {
        double f = freq.value;
        double f2 = f * f;
        double f4 = f2 * f2;
        return (12194.0 * 12194.0 * f4) / 
               ((f2 + 20.6 * 20.6) * std::sqrt((f2 + 107.7 * 107.7) * (f2 + 737.9 * 737.9)) * (f2 + 12194.0 * 12194.0));
    }
};

// Snapshot testing for stability
class SnapshotTester {
public:
    struct Snapshot {
        std::map<std::string, double> parameters;
        std::string hash;
        std::chrono::system_clock::time_point timestamp;
    };
    
    // Create snapshot of current state
    Snapshot createSnapshot(const PresetNormalizer::NormalizedPreset& preset) const;
    
    // Compare snapshots
    bool compareSnapshots(const Snapshot& a, const Snapshot& b, double tolerance = 1e-6) const;
    
    // Load reference snapshots
    void loadReferenceSnapshots(const std::string& path);
    
    // Save snapshots
    void saveSnapshots(const std::vector<Snapshot>& snapshots, const std::string& path) const;
    
    // Generate hash for stability checking
    std::string generateHash(const std::map<std::string, double>& parameters) const;
    
private:
    std::vector<Snapshot> referenceSnapshots_;
    
    // Hash generation
    std::string hashParameters(const std::map<std::string, double>& params) const;
};

// Musical context normalization
class MusicalNormalizer {
public:
    // Tempo-based time normalization
    static Seconds tempoToTime(double beats, FrequencyHz tempo) {
        return Seconds{beats * 60.0 / tempo};
    }
    
    // Time to tempo-based beats
    static double timeToTempo(Seconds time, FrequencyHz tempo) {
        return time.value * tempo / 60.0;
    }
    
    // Musical division snapping
    static double snapToMusicalDivision(double time, FrequencyHz tempo, double division) {
        double beatTime = 60.0 / tempo;
        double divisionTime = beatTime / division;
        return std::round(time / divisionTime) * divisionTime;
    }
    
    // Key-aware pitch shifting
    static Hz shiftPitchInKey(Hz originalFreq, int semitones, int key, const std::string& scale) {
        int midiNote = Normalizer::freqToMidi(originalFreq);
        int shiftedMidi = midiNote + semitones;
        
        // Ensure note is in key/scale
        if (!isNoteInKey(shiftedMidi, key, scale)) {
            // Find closest note in key
            shiftedMidi = findClosestNoteInKey(shiftedMidi, key, scale);
        }
        
        return midiToFreq(shiftedMidi);
    }
    
    // Check if note is in key/scale
    static bool isNoteInKey(int midiNote, int key, const std::string& scale) {
        int noteInOctave = midiNote % 12;
        int keyInOctave = key % 12;
        
        if (scale == "major") {
            int majorScale[] = {0, 2, 4, 5, 7, 9, 11};
            for (int interval : majorScale) {
                if ((noteInOctave - keyInOctave + 12) % 12 == interval) {
                    return true;
                }
            }
        } else if (scale == "minor") {
            int minorScale[] = {0, 2, 3, 5, 7, 8, 10};
            for (int interval : minorScale) {
                if ((noteInOctave - keyInOctave + 12) % 12 == interval) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    // Find closest note in key
    static int findClosestNoteInKey(int midiNote, int key, const std::string& scale) {
        int noteInOctave = midiNote % 12;
        int keyInOctave = key % 12;
        
        std::vector<int> scaleIntervals;
        if (scale == "major") {
            scaleIntervals = {0, 2, 4, 5, 7, 9, 11};
        } else if (scale == "minor") {
            scaleIntervals = {0, 2, 3, 5, 7, 8, 10};
        }
        
        int closestInterval = scaleIntervals[0];
        int minDistance = std::abs((noteInOctave - keyInOctave + 12) % 12 - scaleIntervals[0]);
        
        for (int interval : scaleIntervals) {
            int distance = std::abs((noteInOctave - keyInOctave + 12) % 12 - interval);
            if (distance < minDistance) {
                minDistance = distance;
                closestInterval = interval;
            }
        }
        
        int octave = midiNote / 12;
        return octave * 12 + keyInOctave + closestInterval;
    }
};

} // namespace aiaudio