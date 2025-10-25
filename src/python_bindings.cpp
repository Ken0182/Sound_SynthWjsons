#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "main_app.h"
#include "core_types.h"

namespace py = pybind11;
using namespace aiaudio;

namespace {

Role role_from_string(const std::string& roleStr) {
    std::string r = roleStr;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    if (r == "pad") return Role::PAD;
    if (r == "bass") return Role::BASS;
    if (r == "lead") return Role::LEAD;
    if (r == "drum") return Role::DRUM;
    if (r == "percussion") return Role::PERCUSSION;
    if (r == "ambient") return Role::AMBIENT;
    if (r == "texture") return Role::TEXTURE;
    return Role::UNKNOWN;
}

AIAudioGenerator::GenerationRequest request_from_dict(const py::dict& d) {
    AIAudioGenerator::GenerationRequest req;
    if (d.contains("prompt")) req.prompt = py::cast<std::string>(d["prompt"]);

    // role may be string or int index
    if (d.contains("role")) {
        if (py::isinstance<py::str>(d["role"])) {
            req.role = role_from_string(py::cast<std::string>(d["role"]));
        } else if (py::isinstance<py::int_>(d["role"])) {
            int v = py::cast<int>(d["role"]);
            req.role = static_cast<Role>(v);
        }
    }

    // context
    if (d.contains("context")) {
        py::dict ctx = py::cast<py::dict>(d["context"]);
        if (ctx.contains("tempo")) req.context.tempo = py::cast<double>(ctx["tempo"]);
        if (ctx.contains("key")) req.context.key = py::cast<int>(ctx["key"]);
        if (ctx.contains("scale")) req.context.scale = py::cast<std::string>(ctx["scale"]);
    }

    // constraints
    if (d.contains("constraints")) {
        py::dict c = py::cast<py::dict>(d["constraints"]);
        if (c.contains("maxCPU")) req.constraints.maxCPU = py::cast<double>(c["maxCPU"]);
        if (c.contains("maxLatency")) req.constraints.maxLatency = py::cast<double>(c["maxLatency"]);
        if (c.contains("noHardClips")) req.constraints.noHardClips = py::cast<bool>(c["noHardClips"]);
        if (c.contains("truePeakLimit")) req.constraints.truePeakLimit = py::cast<double>(c["truePeakLimit"]);
        if (c.contains("lufsTarget")) req.constraints.lufsTarget = py::cast<double>(c["lufsTarget"]);
        if (c.contains("crestFactorMin")) req.constraints.crestFactorMin = py::cast<double>(c["crestFactorMin"]);
        if (c.contains("crestFactorMax")) req.constraints.crestFactorMax = py::cast<double>(c["crestFactorMax"]);
    }

    if (d.contains("useSemanticSearch")) req.useSemanticSearch = py::cast<bool>(d["useSemanticSearch"]);
    if (d.contains("applyPolicies")) req.applyPolicies = py::cast<bool>(d["applyPolicies"]);
    if (d.contains("optimizeForMOO")) req.optimizeForMOO = py::cast<bool>(d["optimizeForMOO"]);

    return req;
}

}

PYBIND11_MODULE(aiaudio_python, m) {
    m.doc() = "Python bindings for AIAudioGenerator (preview rendering)";

    py::class_<AIAudioGenerator>(m, "Engine")
        .def(py::init<>())
        .def("generate", [](AIAudioGenerator& self, const py::dict& request) {
            auto req = request_from_dict(request);
            auto result = self.generate(req);

            // Create numpy array (mono float32)
            size_t n = result.audio.size();
            auto out = py::array_t<float>(n);
            auto buf = out.mutable_unchecked<1>();
            for (size_t i = 0; i < n; ++i) {
                buf(i) = static_cast<float>(result.audio[i]);
            }

            py::dict meta;
            meta["qualityScore"] = result.qualityScore;
            meta["warnings"] = result.warnings;
            meta["explanation"] = result.explanation;

            py::dict trace;
            trace["prompt"] = result.trace.prompt;
            trace["queryHash"] = result.trace.queryHash;
            trace["entryId"] = result.trace.entryId;
            trace["policyVersion"] = result.trace.policyVersion;
            trace["budgetTier"] = result.trace.budgetTier;
            trace["seed"] = result.trace.seed;
            meta["trace"] = trace;

            return py::make_tuple(out, meta);
        }, py::arg("request"),
        R"pbdoc(Returns (audio: np.ndarray[float32], meta: dict))pbdoc");

    // Convenience free function
    m.def("render_sync", [](const py::dict& request) {
        AIAudioGenerator engine;
        auto req = request_from_dict(request);
        auto result = engine.generate(req);
        size_t n = result.audio.size();
        auto out = py::array_t<float>(n);
        auto buf = out.mutable_unchecked<1>();
        for (size_t i = 0; i < n; ++i) buf(i) = static_cast<float>(result.audio[i]);

        py::dict meta;
        meta["qualityScore"] = result.qualityScore;
        meta["warnings"] = result.warnings;
        meta["explanation"] = result.explanation;
        return py::make_tuple(out, meta);
    }, py::arg("request"));
}
