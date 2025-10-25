#pragma once

#include "core_types.h"
#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <memory>

namespace aiaudio {

// Semantic Fusion: Query + Tags + Descriptions

class SemanticEmbedding {
public:
    using EmbeddingVector = std::vector<double>;
    
    // Encode text to embedding vector
    virtual EmbeddingVector encode(const std::string& text) const = 0;
    
    // Get embedding dimension
    virtual size_t getDimension() const = 0;
    
    // Compute cosine similarity
    static double cosineSimilarity(const EmbeddingVector& a, const EmbeddingVector& b) {
        if (a.size() != b.size()) return 0.0;
        
        double dotProduct = 0.0;
        double normA = 0.0;
        double normB = 0.0;
        
        for (size_t i = 0; i < a.size(); ++i) {
            dotProduct += a[i] * b[i];
            normA += a[i] * a[i];
            normB += b[i] * b[i];
        }
        
        if (normA == 0.0 || normB == 0.0) return 0.0;
        
        return dotProduct / (std::sqrt(normA) * std::sqrt(normB));
    }
    
    // Normalize vector
    static EmbeddingVector normalize(const EmbeddingVector& vec) {
        double norm = 0.0;
        for (double val : vec) {
            norm += val * val;
        }
        norm = std::sqrt(norm);
        
        if (norm == 0.0) return vec;
        
        EmbeddingVector normalized = vec;
        for (double& val : normalized) {
            val /= norm;
        }
        return normalized;
    }
};

// Simple embedding model (placeholder for real implementation)
class SimpleEmbedding : public SemanticEmbedding {
public:
    SimpleEmbedding(size_t dimension = 384) : dimension_(dimension) {}
    
    EmbeddingVector encode(const std::string& text) const override {
        // Simple bag-of-words embedding
        EmbeddingVector embedding(dimension_, 0.0);
        
        // Simple hash-based embedding
        std::hash<std::string> hasher;
        size_t hash = hasher(text);
        
        for (size_t i = 0; i < dimension_; ++i) {
            hash = hash * 31 + i;
            embedding[i] = (hash % 1000) / 1000.0 - 0.5;
        }
        
        return normalize(embedding);
    }
    
    size_t getDimension() const override { return dimension_; }
    
private:
    size_t dimension_;
};

// Tag system
class TagSystem {
public:
    struct Tag {
        std::string name;
        EmbeddingVector embedding;
        double weight = 1.0;
        std::string category;
    };
    
    // Add tag
    void addTag(const std::string& name, const EmbeddingVector& embedding, 
                const std::string& category = "", double weight = 1.0);
    
    // Get tag by name
    const Tag* getTag(const std::string& name) const;
    
    // Get tags by category
    std::vector<const Tag*> getTagsByCategory(const std::string& category) const;
    
    // Get all tags
    std::vector<const Tag*> getAllTags() const;
    
    // Remove tag
    void removeTag(const std::string& name);
    
    // Clear all tags
    void clear();
    
private:
    std::map<std::string, Tag> tags_;
    std::map<std::string, std::vector<std::string>> categoryIndex_;
};

// Semantic Fusion Engine
class SemanticFusionEngine {
public:
    struct FusionResult {
        EmbeddingVector queryVector;
        EmbeddingVector entryVector;
        double semanticScore;
        std::vector<std::string> contributingTags;
        std::vector<std::string> excludedTags;
        double confidence;
    };
    
    // Initialize with embedding model
    explicit SemanticFusionEngine(std::unique_ptr<SemanticEmbedding> embedding);
    
    // Compose contrastive query vector
    EmbeddingVector composeContrastive(const std::string& query,
                                      const std::vector<std::string>& positiveTags,
                                      const std::vector<std::string>& negativeTags,
                                      double alpha = 0.3,
                                      double beta = 0.7) const;
    
    // Compute semantic score
    double semanticScore(const EmbeddingVector& query, const EmbeddingVector& entry) const;
    
    // Advanced scoring with learned weights
    double semanticScoreWeighted(const EmbeddingVector& query, 
                                const EmbeddingVector& entry,
                                const std::vector<double>& weights) const;
    
    // Process entry with tags and description
    EmbeddingVector processEntry(const std::vector<std::string>& tags,
                                const std::string& description,
                                double delta = 0.5) const;
    
    // Get contributing tags for explanation
    std::vector<std::string> getContributingTags(const EmbeddingVector& query,
                                                const EmbeddingVector& entry,
                                                double threshold = 0.1) const;
    
    // Set learned weights for role-specific scoring
    void setRoleWeights(Role role, const std::vector<double>& weights);
    
    // Get role weights
    std::vector<double> getRoleWeights(Role role) const;
    
private:
    std::unique_ptr<SemanticEmbedding> embedding_;
    TagSystem tagSystem_;
    std::map<Role, std::vector<double>> roleWeights_;
    
    // Helper functions
    EmbeddingVector averageEmbeddings(const std::vector<EmbeddingVector>& embeddings) const;
    double computeIDF(const std::string& tag) const;
    std::vector<std::string> tokenize(const std::string& text) const;
};

// Entry Vector Builder
class EntryVectorBuilder {
public:
    struct EntryData {
        std::string id;
        std::vector<std::string> tags;
        std::string description;
        std::map<std::string, double> metadata;
        Role role;
    };
    
    // Build entry vector from data
    EmbeddingVector buildEntryVector(const EntryData& data, 
                                    const SemanticFusionEngine& engine) const;
    
    // Add metadata to vector
    EmbeddingVector addMetadata(const EmbeddingVector& baseVector,
                               const std::map<std::string, double>& metadata) const;
    
    // Apply role-specific weighting
    EmbeddingVector applyRoleWeighting(const EmbeddingVector& vector, Role role) const;
    
private:
    // Metadata encoding
    EmbeddingVector encodeMetadata(const std::map<std::string, double>& metadata) const;
    
    // Role-specific processing
    EmbeddingVector processForRole(const EmbeddingVector& vector, Role role) const;
};

// Semantic Search Engine
class SemanticSearchEngine {
public:
    struct SearchResult {
        std::string entryId;
        double score;
        std::vector<std::string> matchingTags;
        std::string explanation;
        double confidence;
    };
    
    // Initialize with fusion engine
    explicit SemanticSearchEngine(std::unique_ptr<SemanticFusionEngine> engine);
    
    // Add entry to search index
    void addEntry(const EntryVectorBuilder::EntryData& data);
    
    // Search for entries
    std::vector<SearchResult> search(const std::string& query,
                                    Role role = Role::UNKNOWN,
                                    size_t maxResults = 10) const;
    
    // Search with contrastive query
    std::vector<SearchResult> searchContrastive(const std::string& query,
                                               const std::vector<std::string>& positiveTags,
                                               const std::vector<std::string>& negativeTags,
                                               Role role = Role::UNKNOWN,
                                               size_t maxResults = 10) const;
    
    // Get explanation for result
    std::string explainResult(const SearchResult& result) const;
    
    // Update entry
    void updateEntry(const std::string& entryId, const EntryVectorBuilder::EntryData& data);
    
    // Remove entry
    void removeEntry(const std::string& entryId);
    
    // Clear all entries
    void clear();
    
    // Get entry count
    size_t getEntryCount() const;
    
private:
    std::unique_ptr<SemanticFusionEngine> fusionEngine_;
    std::map<std::string, EntryVectorBuilder::EntryData> entries_;
    std::map<std::string, EmbeddingVector> entryVectors_;
    
    // Search helpers
    std::vector<SearchResult> rankResults(const std::vector<std::string>& entryIds,
                                         const EmbeddingVector& queryVector,
                                         Role role) const;
    
    std::string generateExplanation(const SearchResult& result,
                                   const EmbeddingVector& queryVector) const;
};

// Advanced Semantic Features
class AdvancedSemanticFeatures {
public:
    // Dimension-wise reweighting
    static EmbeddingVector applyDimensionWeights(const EmbeddingVector& vector,
                                                const std::vector<double>& weights);
    
    // Learned diagonal matrix multiplication
    static EmbeddingVector applyDiagonalMatrix(const EmbeddingVector& vector,
                                              const std::vector<double>& diagonal);
    
    // Intersection-based scoring
    static double intersectionScore(const std::vector<std::string>& queryTags,
                                   const std::vector<std::string>& entryTags);
    
    // IDF-based term weighting
    static double computeIDFWeight(const std::string& term,
                                  const std::map<std::string, int>& termFrequencies,
                                  int totalDocuments);
    
    // Semantic clustering
    static std::vector<std::vector<std::string>> clusterTags(
        const std::vector<std::string>& tags,
        const SemanticEmbedding& embedding,
        double threshold = 0.7);
    
    // Tag importance scoring
    static std::map<std::string, double> computeTagImportance(
        const std::vector<std::string>& tags,
        const SemanticEmbedding& embedding);
};

// Testing and Validation
class SemanticTester {
public:
    // Test monotonic demotion for excluded terms
    bool testMonotonicDemotion(const SemanticFusionEngine& engine,
                               const std::vector<std::string>& testPrompts) const;
    
    // Test semantic consistency
    bool testSemanticConsistency(const SemanticFusionEngine& engine) const;
    
    // Generate synthetic test data
    std::vector<std::string> generateSyntheticPrompts(size_t count) const;
    
    // Validate embedding quality
    bool validateEmbeddingQuality(const SemanticEmbedding& embedding) const;
    
private:
    // Test helpers
    bool checkMonotonicity(const std::vector<double>& scores) const;
    double computeSemanticConsistency(const std::vector<EmbeddingVector>& vectors) const;
};

} // namespace aiaudio