#include "semantic_fusion.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <random>

namespace aiaudio {

// TagSystem implementation
void TagSystem::addTag(const std::string& name, const SemanticEmbedding::EmbeddingVector& embedding, 
                      const std::string& category, double weight) {
    Tag tag;
    tag.name = name;
    tag.embedding = embedding;
    tag.category = category;
    tag.weight = weight;
    
    tags_[name] = tag;
    categoryIndex_[category].push_back(name);
}

const TagSystem::Tag* TagSystem::getTag(const std::string& name) const {
    auto it = tags_.find(name);
    return (it != tags_.end()) ? &it->second : nullptr;
}

std::vector<const TagSystem::Tag*> TagSystem::getTagsByCategory(const std::string& category) const {
    std::vector<const TagSystem::Tag*> result;
    
    auto it = categoryIndex_.find(category);
    if (it != categoryIndex_.end()) {
        for (const auto& tagName : it->second) {
            auto tagIt = tags_.find(tagName);
            if (tagIt != tags_.end()) {
                result.push_back(&tagIt->second);
            }
        }
    }
    
    return result;
}

std::vector<const TagSystem::Tag*> TagSystem::getAllTags() const {
    std::vector<const TagSystem::Tag*> result;
    for (const auto& [name, tag] : tags_) {
        result.push_back(&tag);
    }
    return result;
}

void TagSystem::removeTag(const std::string& name) {
    auto it = tags_.find(name);
    if (it != tags_.end()) {
        // Remove from category index
        auto catIt = categoryIndex_.find(it->second.category);
        if (catIt != categoryIndex_.end()) {
            auto& categoryTags = catIt->second;
            categoryTags.erase(std::remove(categoryTags.begin(), categoryTags.end(), name),
                             categoryTags.end());
        }
        
        tags_.erase(it);
    }
}

void TagSystem::clear() {
    tags_.clear();
    categoryIndex_.clear();
}

// SemanticFusionEngine implementation
SemanticFusionEngine::SemanticFusionEngine(std::unique_ptr<SemanticEmbedding> embedding)
    : embedding_(std::move(embedding)) {
}

SemanticEmbedding::EmbeddingVector SemanticFusionEngine::composeContrastive(const std::string& query,
                                                        const std::vector<std::string>& positiveTags,
                                                        const std::vector<std::string>& negativeTags,
                                                        double alpha,
                                                        double beta) const {
    // Encode base query
    SemanticEmbedding::EmbeddingVector queryVec = embedding_->encode(query);
    
    // Add positive tags
    SemanticEmbedding::EmbeddingVector posVec = queryVec;
    if (!positiveTags.empty()) {
        std::vector<SemanticEmbedding::EmbeddingVector> posEmbeddings;
        for (const auto& tag : positiveTags) {
            posEmbeddings.push_back(embedding_->encode(tag));
        }
        SemanticEmbedding::EmbeddingVector avgPos = averageEmbeddings(posEmbeddings);
        
        // Weighted combination
        for (size_t i = 0; i < queryVec.size(); ++i) {
            posVec[i] = beta * queryVec[i] + (1.0 - beta) * avgPos[i];
        }
    }
    
    // Subtract negative tags
    SemanticEmbedding::EmbeddingVector result = posVec;
    if (!negativeTags.empty()) {
        std::vector<SemanticEmbedding::EmbeddingVector> negEmbeddings;
        for (const auto& tag : negativeTags) {
            negEmbeddings.push_back(embedding_->encode(tag));
        }
        SemanticEmbedding::EmbeddingVector avgNeg = averageEmbeddings(negEmbeddings);
        
        // Subtract negative influence
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = result[i] - alpha * avgNeg[i];
        }
    }
    
    return SemanticEmbedding::normalize(result);
}

double SemanticFusionEngine::semanticScore(const SemanticEmbedding::EmbeddingVector& query, 
                                         const SemanticEmbedding::EmbeddingVector& entry) const {
    double cosineSim = SemanticEmbedding::cosineSimilarity(query, entry);
    return std::clamp(cosineSim, 0.0, 1.0);
}

double SemanticFusionEngine::semanticScoreWeighted(const SemanticEmbedding::EmbeddingVector& query, 
                                                 const SemanticEmbedding::EmbeddingVector& entry,
                                                  const std::vector<double>& weights) const {
    if (query.size() != entry.size() || query.size() != weights.size()) {
        return 0.0;
    }
    
    double weightedDot = 0.0;
    double queryNorm = 0.0;
    double entryNorm = 0.0;
    
    for (size_t i = 0; i < query.size(); ++i) {
        double weightedQuery = query[i] * weights[i];
        double weightedEntry = entry[i] * weights[i];
        
        weightedDot += weightedQuery * weightedEntry;
        queryNorm += weightedQuery * weightedQuery;
        entryNorm += weightedEntry * weightedEntry;
    }
    
    if (queryNorm == 0.0 || entryNorm == 0.0) return 0.0;
    
    return weightedDot / (std::sqrt(queryNorm) * std::sqrt(entryNorm));
}

SemanticEmbedding::EmbeddingVector SemanticFusionEngine::processEntry(const std::vector<std::string>& tags,
                                                 const std::string& description,
                                                 double delta) const {
    std::vector<SemanticEmbedding::EmbeddingVector> tagEmbeddings;
    
    // Encode tags
    for (const auto& tag : tags) {
        tagEmbeddings.push_back(embedding_->encode(tag));
    }
    
    // Average tag embeddings
    SemanticEmbedding::EmbeddingVector tagVec = averageEmbeddings(tagEmbeddings);
    
    // Encode description
    SemanticEmbedding::EmbeddingVector descVec = embedding_->encode(description);
    
    // Combine with delta weighting
    SemanticEmbedding::EmbeddingVector result = tagVec;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = result[i] + delta * descVec[i];
    }
    
    return SemanticEmbedding::normalize(result);
}

std::vector<std::string> SemanticFusionEngine::getContributingTags(const SemanticEmbedding::EmbeddingVector& query,
                                                                const SemanticEmbedding::EmbeddingVector& entry,
                                                                 double threshold) const {
    std::vector<std::string> contributing;
    
    // This is a simplified implementation
    // In practice, would need to track which tags contributed to the entry vector
    
    return contributing;
}

void SemanticFusionEngine::setRoleWeights(Role role, const std::vector<double>& weights) {
    roleWeights_[role] = weights;
}

std::vector<double> SemanticFusionEngine::getRoleWeights(Role role) const {
    auto it = roleWeights_.find(role);
    if (it != roleWeights_.end()) {
        return it->second;
    }
    
    // Return default weights (all 1.0)
    return std::vector<double>(embedding_->getDimension(), 1.0);
}

SemanticEmbedding::EmbeddingVector SemanticFusionEngine::averageEmbeddings(const std::vector<SemanticEmbedding::EmbeddingVector>& embeddings) const {
    if (embeddings.empty()) {
        return SemanticEmbedding::EmbeddingVector(embedding_->getDimension(), 0.0);
    }
    
    SemanticEmbedding::EmbeddingVector result(embeddings[0].size(), 0.0);
    
    for (const auto& embedding : embeddings) {
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] += embedding[i];
        }
    }
    
    for (double& val : result) {
        val /= embeddings.size();
    }
    
    return SemanticEmbedding::normalize(result);
}

double SemanticFusionEngine::computeIDF(const std::string& tag) const {
    // Simplified IDF computation
    // In practice, would need document frequency statistics
    return 1.0;
}

std::vector<std::string> SemanticFusionEngine::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string token;
    
    while (ss >> token) {
        // Simple tokenization (in practice would be more sophisticated)
        tokens.push_back(token);
    }
    
    return tokens;
}

// EntryVectorBuilder implementation
SemanticEmbedding::EmbeddingVector EntryVectorBuilder::buildEntryVector(const EntryData& data, 
                                                    const SemanticFusionEngine& engine) const {
    // Process tags and description
    SemanticEmbedding::EmbeddingVector baseVector = engine.processEntry(data.tags, data.description);
    
    // Add metadata
    SemanticEmbedding::EmbeddingVector withMetadata = addMetadata(baseVector, data.metadata);
    
    // Apply role weighting
    SemanticEmbedding::EmbeddingVector finalVector = applyRoleWeighting(withMetadata, data.role);
    
    return finalVector;
}

SemanticEmbedding::EmbeddingVector EntryVectorBuilder::addMetadata(const SemanticEmbedding::EmbeddingVector& baseVector,
                                               const std::map<std::string, double>& metadata) const {
    // Simple metadata encoding (in practice would be more sophisticated)
    SemanticEmbedding::EmbeddingVector result = baseVector;
    
    // Add metadata as additional dimensions or modify existing ones
    // This is a simplified implementation
    
    return result;
}

SemanticEmbedding::EmbeddingVector EntryVectorBuilder::applyRoleWeighting(const SemanticEmbedding::EmbeddingVector& vector, Role role) const {
    // Apply role-specific weighting
    // This would use learned weights for each role
    
    return vector;
}

SemanticEmbedding::EmbeddingVector EntryVectorBuilder::encodeMetadata(const std::map<std::string, double>& metadata) const {
    // Encode metadata into embedding space
    // This is a placeholder implementation
    
    return SemanticEmbedding::EmbeddingVector(384, 0.0); // Default dimension
}

SemanticEmbedding::EmbeddingVector EntryVectorBuilder::processForRole(const SemanticEmbedding::EmbeddingVector& vector, Role role) const {
    // Role-specific processing
    // This would apply role-specific transformations
    
    return vector;
}

// SemanticSearchEngine implementation
SemanticSearchEngine::SemanticSearchEngine(std::unique_ptr<SemanticFusionEngine> engine)
    : fusionEngine_(std::move(engine)) {
}

void SemanticSearchEngine::addEntry(const EntryVectorBuilder::EntryData& data) {
    entries_[data.id] = data;
    entryVectors_[data.id] = fusionEngine_->processEntry(data.tags, data.description);
}

std::vector<SemanticSearchEngine::SearchResult> SemanticSearchEngine::search(const std::string& query,
                                                                            Role role,
                                                                            size_t maxResults) const {
    SemanticEmbedding::EmbeddingVector queryVec = fusionEngine_->composeContrastive(query, {}, {}, 0.0, 1.0);
    
    std::vector<std::string> entryIds;
    for (const auto& [id, data] : entries_) {
        if (role == Role::UNKNOWN || data.role == role) {
            entryIds.push_back(id);
        }
    }
    
    return rankResults(entryIds, queryVec, role);
}

std::vector<SemanticSearchEngine::SearchResult> SemanticSearchEngine::searchContrastive(
    const std::string& query,
    const std::vector<std::string>& positiveTags,
    const std::vector<std::string>& negativeTags,
    Role role,
    size_t maxResults) const {
    
    SemanticEmbedding::EmbeddingVector queryVec = fusionEngine_->composeContrastive(query, positiveTags, negativeTags);
    
    std::vector<std::string> entryIds;
    for (const auto& [id, data] : entries_) {
        if (role == Role::UNKNOWN || data.role == role) {
            entryIds.push_back(id);
        }
    }
    
    return rankResults(entryIds, queryVec, role);
}

std::string SemanticSearchEngine::explainResult(const SearchResult& result) const {
    std::stringstream explanation;
    explanation << "Entry: " << result.entryId << "\n";
    explanation << "Score: " << result.score << "\n";
    explanation << "Confidence: " << result.confidence << "\n";
    explanation << "Matching tags: ";
    for (const auto& tag : result.matchingTags) {
        explanation << tag << " ";
    }
    explanation << "\n" << result.explanation;
    
    return explanation.str();
}

void SemanticSearchEngine::updateEntry(const std::string& entryId, const EntryVectorBuilder::EntryData& data) {
    entries_[entryId] = data;
    entryVectors_[entryId] = fusionEngine_->processEntry(data.tags, data.description);
}

void SemanticSearchEngine::removeEntry(const std::string& entryId) {
    entries_.erase(entryId);
    entryVectors_.erase(entryId);
}

void SemanticSearchEngine::clear() {
    entries_.clear();
    entryVectors_.clear();
}

size_t SemanticSearchEngine::getEntryCount() const {
    return entries_.size();
}

std::vector<SemanticSearchEngine::SearchResult> SemanticSearchEngine::rankResults(
    const std::vector<std::string>& entryIds,
    const SemanticEmbedding::EmbeddingVector& queryVector,
    Role role) const {
    
    std::vector<SearchResult> results;
    
    for (const auto& entryId : entryIds) {
        auto it = entryVectors_.find(entryId);
        if (it != entryVectors_.end()) {
            SearchResult result;
            result.entryId = entryId;
            result.score = fusionEngine_->semanticScore(queryVector, it->second);
            result.confidence = result.score; // Simplified
            result.matchingTags = {}; // Would be populated with actual matching tags
            result.explanation = generateExplanation(result, queryVector);
            
            results.push_back(result);
        }
    }
    
    // Sort by score (descending)
    std::sort(results.begin(), results.end(),
              [](const SearchResult& a, const SearchResult& b) {
                  return a.score > b.score;
              });
    
    return results;
}

std::string SemanticSearchEngine::generateExplanation(const SearchResult& result,
                                                    const SemanticEmbedding::EmbeddingVector& queryVector) const {
    // Generate human-readable explanation
    std::stringstream explanation;
    explanation << "This entry matches your query with a semantic similarity of " 
                << std::fixed << std::setprecision(2) << result.score * 100 << "%";
    
    return explanation.str();
}

// AdvancedSemanticFeatures implementation
SemanticEmbedding::EmbeddingVector AdvancedSemanticFeatures::applyDimensionWeights(const SemanticEmbedding::EmbeddingVector& vector,
                                                               const std::vector<double>& weights) {
    if (vector.size() != weights.size()) return vector;
    
    SemanticEmbedding::EmbeddingVector result = vector;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] *= weights[i];
    }
    
    return SemanticEmbedding::normalize(result);
}

SemanticEmbedding::EmbeddingVector AdvancedSemanticFeatures::applyDiagonalMatrix(const SemanticEmbedding::EmbeddingVector& vector,
                                                           const std::vector<double>& diagonal) {
    return applyDimensionWeights(vector, diagonal);
}

double AdvancedSemanticFeatures::intersectionScore(const std::vector<std::string>& queryTags,
                                                  const std::vector<std::string>& entryTags) {
    std::unordered_set<std::string> querySet(queryTags.begin(), queryTags.end());
    std::unordered_set<std::string> entrySet(entryTags.begin(), entryTags.end());
    
    std::vector<std::string> intersection;
    std::set_intersection(querySet.begin(), querySet.end(),
                         entrySet.begin(), entrySet.end(),
                         std::back_inserter(intersection));
    
    if (querySet.empty() && entrySet.empty()) return 1.0;
    if (querySet.empty() || entrySet.empty()) return 0.0;
    
    return static_cast<double>(intersection.size()) / 
           std::max(querySet.size(), entrySet.size());
}

double AdvancedSemanticFeatures::computeIDFWeight(const std::string& term,
                                                 const std::map<std::string, int>& termFrequencies,
                                                 int totalDocuments) {
    auto it = termFrequencies.find(term);
    if (it == termFrequencies.end()) return 0.0;
    
    int docFreq = it->second;
    return std::log(static_cast<double>(totalDocuments) / docFreq);
}

std::vector<std::vector<std::string>> AdvancedSemanticFeatures::clusterTags(
    const std::vector<std::string>& tags,
    const SemanticEmbedding& embedding,
    double threshold) {
    
    std::vector<std::vector<std::string>> clusters;
    std::vector<bool> assigned(tags.size(), false);
    
    for (size_t i = 0; i < tags.size(); ++i) {
        if (assigned[i]) continue;
        
        std::vector<std::string> cluster;
        cluster.push_back(tags[i]);
        assigned[i] = true;
        
        auto embeddingI = embedding.encode(tags[i]);
        
        for (size_t j = i + 1; j < tags.size(); ++j) {
            if (assigned[j]) continue;
            
            auto embeddingJ = embedding.encode(tags[j]);
            double similarity = SemanticEmbedding::cosineSimilarity(embeddingI, embeddingJ);
            
            if (similarity >= threshold) {
                cluster.push_back(tags[j]);
                assigned[j] = true;
            }
        }
        
        clusters.push_back(cluster);
    }
    
    return clusters;
}

std::map<std::string, double> AdvancedSemanticFeatures::computeTagImportance(
    const std::vector<std::string>& tags,
    const SemanticEmbedding& embedding) {
    
    std::map<std::string, double> importance;
    
    // Simple importance based on embedding magnitude
    for (const auto& tag : tags) {
        auto embeddingVec = embedding.encode(tag);
        double magnitude = 0.0;
        for (double val : embeddingVec) {
            magnitude += val * val;
        }
        importance[tag] = std::sqrt(magnitude);
    }
    
    return importance;
}

// SemanticTester implementation
bool SemanticTester::testMonotonicDemotion(const SemanticFusionEngine& engine,
                                          const std::vector<std::string>& testPrompts) const {
    // Test that adding negative tags decreases similarity
    for (const auto& prompt : testPrompts) {
        std::vector<std::string> positiveTags = {"good", "positive"};
        std::vector<std::string> negativeTags = {"bad", "negative"};
        
        auto queryPos = engine.composeContrastive(prompt, positiveTags, {}, 0.0, 0.5);
        auto queryNeg = engine.composeContrastive(prompt, positiveTags, negativeTags, 0.3, 0.5);
        
        // Create test entry
        std::vector<std::string> entryTags = {"good", "positive", "test"};
        auto entryVec = engine.processEntry(entryTags, "test description");
        
        double scorePos = engine.semanticScore(queryPos, entryVec);
        double scoreNeg = engine.semanticScore(queryNeg, entryVec);
        
        if (scoreNeg >= scorePos) {
            return false; // Monotonicity violated
        }
    }
    
    return true;
}

bool SemanticTester::testSemanticConsistency(const SemanticFusionEngine& engine) const {
    // Test that similar queries produce similar results
    std::vector<std::string> similarQueries = {
        "dreamy pad sound",
        "atmospheric pad",
        "ethereal pad"
    };
    
std::vector<SemanticEmbedding::EmbeddingVector> queryVectors;
    for (const auto& query : similarQueries) {
        queryVectors.push_back(engine.composeContrastive(query, {}, {}, 0.0, 1.0));
    }
    
    double consistency = computeSemanticConsistency(queryVectors);
    return consistency > 0.7; // Threshold for consistency
}

std::vector<std::string> SemanticTester::generateSyntheticPrompts(size_t count) const {
    std::vector<std::string> prompts;
    
    std::vector<std::string> adjectives = {"dreamy", "dark", "bright", "warm", "cold", "soft", "harsh"};
    std::vector<std::string> nouns = {"pad", "bass", "lead", "drum", "texture", "atmosphere"};
    std::vector<std::string> modifiers = {"with reverb", "distorted", "filtered", "modulated"};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> adjDist(0, adjectives.size() - 1);
    std::uniform_int_distribution<> nounDist(0, nouns.size() - 1);
    std::uniform_int_distribution<> modDist(0, modifiers.size() - 1);
    
    for (size_t i = 0; i < count; ++i) {
        std::string prompt = adjectives[adjDist(gen)] + " " + nouns[nounDist(gen)];
        if (i % 3 == 0) { // Add modifier sometimes
            prompt += " " + modifiers[modDist(gen)];
        }
        prompts.push_back(prompt);
    }
    
    return prompts;
}

bool SemanticTester::validateEmbeddingQuality(const SemanticEmbedding& embedding) const {
    // Test embedding properties
    auto testVec1 = embedding.encode("test");
    auto testVec2 = embedding.encode("test");
    
    // Check normalization
    double norm = 0.0;
    for (double val : testVec1) {
        norm += val * val;
    }
    norm = std::sqrt(norm);
    
    if (std::abs(norm - 1.0) > 1e-6) {
        return false; // Not normalized
    }
    
    // Check consistency
    double similarity = SemanticEmbedding::cosineSimilarity(testVec1, testVec2);
    if (similarity < 0.99) {
        return false; // Not consistent
    }
    
    return true;
}

bool SemanticTester::checkMonotonicity(const std::vector<double>& scores) const {
    for (size_t i = 1; i < scores.size(); ++i) {
        if (scores[i] > scores[i-1]) {
            return false;
        }
    }
    return true;
}

double SemanticTester::computeSemanticConsistency(const std::vector<SemanticEmbedding::EmbeddingVector>& vectors) const {
    if (vectors.size() < 2) return 1.0;
    
    double totalSimilarity = 0.0;
    int comparisons = 0;
    
    for (size_t i = 0; i < vectors.size(); ++i) {
        for (size_t j = i + 1; j < vectors.size(); ++j) {
            totalSimilarity += SemanticEmbedding::cosineSimilarity(vectors[i], vectors[j]);
            comparisons++;
        }
    }
    
    return totalSimilarity / comparisons;
}

} // namespace aiaudio