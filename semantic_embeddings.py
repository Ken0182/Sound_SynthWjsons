"""
Semantic Tokenization & Embeddings System
Implements Step 4: Semantic tokenization & embeddings
"""

from typing import Dict, List, Tuple, Optional, Set
from dataclasses import dataclass
import numpy as np
import json
import re
from collections import defaultdict, Counter
import hashlib

@dataclass
class TokenEmbedding:
    """Token with its embedding vector"""
    token: str
    vector: np.ndarray
    frequency: int = 1
    idf: float = 0.0

@dataclass
class QueryEmbedding:
    """Query with its embedding and context"""
    text: str
    vector: np.ndarray
    tokens: List[str]
    role: Optional[str] = None
    tempo: Optional[float] = None
    key: Optional[str] = None

@dataclass
class EntryEmbedding:
    """Database entry with its embedding"""
    preset_id: str
    vector: np.ndarray
    tags: List[str]
    description: str
    role: str
    metadata: Dict

class SemanticTokenizer:
    """Handles tokenization and canonicalization"""
    
    def __init__(self):
        self.kb_aliases = self._load_kb_aliases()
        self.stop_words = self._load_stop_words()
        self.token_cache = {}
    
    def tokenize(self, text: str) -> List[str]:
        """Tokenize text with canonicalization"""
        if text in self.token_cache:
            return self.token_cache[text]
        
        # Normalize text
        text = text.lower().strip()
        
        # Split into tokens
        tokens = re.findall(r'\b\w+\b', text)
        
        # Canonicalize tokens
        canonical_tokens = []
        for token in tokens:
            if token in self.stop_words:
                continue
            
            # Check for KB aliases
            canonical = self.kb_aliases.get(token, token)
            canonical_tokens.append(canonical)
        
        # Remove duplicates while preserving order
        unique_tokens = []
        seen = set()
        for token in unique_tokens:
            if token not in seen:
                unique_tokens.append(token)
                seen.add(token)
        
        self.token_cache[text] = unique_tokens
        return unique_tokens
    
    def _load_kb_aliases(self) -> Dict[str, str]:
        """Load knowledge base aliases for canonicalization"""
        return {
            # Sound characteristics
            'warm': 'warm',
            'hot': 'warm',
            'cozy': 'warm',
            'bright': 'bright',
            'shiny': 'bright',
            'sparkly': 'bright',
            'dark': 'dark',
            'moody': 'dark',
            'ominous': 'dark',
            'fat': 'fat',
            'thick': 'fat',
            'punchy': 'punchy',
            'tight': 'punchy',
            'snappy': 'punchy',
            'soft': 'soft',
            'gentle': 'soft',
            'smooth': 'soft',
            'harsh': 'harsh',
            'aggressive': 'harsh',
            'rough': 'harsh',
            
            # Materials
            'analog': 'analog',
            'vintage': 'analog',
            'retro': 'analog',
            'digital': 'digital',
            'modern': 'digital',
            'clean': 'digital',
            'wood': 'wood',
            'organic': 'wood',
            'natural': 'wood',
            'metal': 'metal',
            'metallic': 'metal',
            'steel': 'metal',
            'glass': 'glass',
            'glassy': 'glass',
            'crystalline': 'glass',
            'plastic': 'plastic',
            'synthetic': 'plastic',
            'artificial': 'plastic',
            
            # Dynamics
            'sustained': 'sustained',
            'long': 'sustained',
            'held': 'sustained',
            'percussive': 'percussive',
            'short': 'percussive',
            'punchy': 'percussive',
            'evolving': 'evolving',
            'changing': 'evolving',
            'morphing': 'evolving',
            'static': 'static',
            'stable': 'static',
            'fixed': 'static',
            
            # Emotional tags
            'dreamy': 'dreamy',
            'ethereal': 'dreamy',
            'floating': 'dreamy',
            'energetic': 'energetic',
            'exciting': 'energetic',
            'upbeat': 'energetic',
            'calm': 'calm',
            'peaceful': 'calm',
            'serene': 'calm',
            'aggressive': 'aggressive',
            'intense': 'aggressive',
            'powerful': 'aggressive',
            'lush': 'lush',
            'rich': 'lush',
            'full': 'lush',
            'minimal': 'minimal',
            'sparse': 'minimal',
            'simple': 'minimal',
            
            # Roles
            'pad': 'pad',
            'atmosphere': 'pad',
            'background': 'pad',
            'bass': 'bass',
            'low': 'bass',
            'foundation': 'bass',
            'lead': 'lead',
            'melody': 'lead',
            'solo': 'lead',
            'fx': 'fx',
            'effect': 'fx',
            'processing': 'fx',
            'texture': 'texture',
            'ambient': 'texture',
            'soundscape': 'texture'
        }
    
    def _load_stop_words(self) -> Set[str]:
        """Load stop words to filter out"""
        return {
            'a', 'an', 'and', 'are', 'as', 'at', 'be', 'by', 'for', 'from',
            'has', 'he', 'in', 'is', 'it', 'its', 'of', 'on', 'that', 'the',
            'to', 'was', 'will', 'with', 'or', 'but', 'not', 'this', 'these',
            'they', 'them', 'their', 'there', 'then', 'than', 'so', 'if',
            'very', 'much', 'more', 'most', 'some', 'any', 'all', 'each',
            'every', 'no', 'other', 'another', 'such', 'only', 'own', 'same'
        }

class EmbeddingGenerator:
    """Generates embeddings for tokens and queries"""
    
    def __init__(self, embedding_dim: int = 128):
        self.embedding_dim = embedding_dim
        self.token_embeddings: Dict[str, TokenEmbedding] = {}
        self.entry_embeddings: Dict[str, EntryEmbedding] = {}
        self.idf_scores: Dict[str, float] = {}
        self.total_documents = 0
    
    def build_vocabulary(self, presets: List) -> None:
        """Build vocabulary from all presets"""
        token_frequencies = defaultdict(int)
        all_tokens = set()
        
        # Collect all tokens from presets
        for preset in presets:
            # Extract text from various fields
            text_fields = []
            
            # Sound characteristics
            if hasattr(preset, 'sound_characteristics') and preset.sound_characteristics:
                text_fields.extend([
                    preset.sound_characteristics.timbral,
                    preset.sound_characteristics.material,
                    preset.sound_characteristics.dynamic
                ])
                
                # Emotional tags
                if preset.sound_characteristics.emotional:
                    for emotion in preset.sound_characteristics.emotional:
                        if isinstance(emotion, dict) and 'tag' in emotion:
                            text_fields.append(emotion['tag'])
            
            # Topological metadata
            if hasattr(preset, 'topological_metadata') and preset.topological_metadata:
                text_fields.extend([
                    preset.topological_metadata.damping,
                    preset.topological_metadata.spectral_complexity,
                    preset.topological_metadata.manifold_position
                ])
            
            # Preset name
            text_fields.append(preset.name)
            
            # Tokenize all text
            tokenizer = SemanticTokenizer()
            for text in text_fields:
                if text:
                    tokens = tokenizer.tokenize(str(text))
                    all_tokens.update(tokens)
                    for token in tokens:
                        token_frequencies[token] += 1
        
        # Create token embeddings
        for token in all_tokens:
            vector = self._generate_token_vector(token)
            frequency = token_frequencies[token]
            
            self.token_embeddings[token] = TokenEmbedding(
                token=token,
                vector=vector,
                frequency=frequency
            )
        
        # Calculate IDF scores
        self.total_documents = len(presets)
        for token, embedding in self.token_embeddings.items():
            # Count documents containing this token
            doc_count = 0
            for preset in presets:
                text_fields = self._extract_text_fields(preset)
                tokenizer = SemanticTokenizer()
                all_tokens_in_preset = set()
                for text in text_fields:
                    if text:
                        tokens = tokenizer.tokenize(str(text))
                        all_tokens_in_preset.update(tokens)
                
                if token in all_tokens_in_preset:
                    doc_count += 1
            
            # Calculate IDF
            if doc_count > 0:
                self.idf_scores[token] = np.log(self.total_documents / doc_count)
            else:
                self.idf_scores[token] = 0.0
            
            embedding.idf = self.idf_scores[token]
    
    def generate_query_embedding(self, text: str, role: str = None, tempo: float = None, key: str = None) -> QueryEmbedding:
        """Generate embedding for a query"""
        tokenizer = SemanticTokenizer()
        tokens = tokenizer.tokenize(text)
        
        # Generate base embedding
        query_vector = self._encode_text(text)
        
        # Apply contrastive learning if we have positive/negative examples
        if role:
            # Add role context
            role_vector = self._encode_text(role)
            query_vector = self._normalize_vector(query_vector + 0.1 * role_vector)
        
        if tempo:
            # Add tempo context (normalize to [0,1])
            tempo_norm = min(max(tempo / 200.0, 0.0), 1.0)  # Assume max tempo 200 BPM
            tempo_vector = np.zeros(self.embedding_dim)
            tempo_vector[0] = tempo_norm  # Use first dimension for tempo
            query_vector = self._normalize_vector(query_vector + 0.05 * tempo_vector)
        
        if key:
            # Add key context
            key_vector = self._encode_text(key)
            query_vector = self._normalize_vector(query_vector + 0.05 * key_vector)
        
        return QueryEmbedding(
            text=text,
            vector=query_vector,
            tokens=tokens,
            role=role,
            tempo=tempo,
            key=key
        )
    
    def generate_entry_embedding(self, preset) -> EntryEmbedding:
        """Generate embedding for a database entry"""
        # Extract all text fields
        text_fields = self._extract_text_fields(preset)
        
        # Tokenize and get tag vectors
        tokenizer = SemanticTokenizer()
        all_tokens = set()
        for text in text_fields:
            if text:
                tokens = tokenizer.tokenize(str(text))
                all_tokens.update(tokens)
        
        # Get tag vectors
        tag_vectors = []
        for token in all_tokens:
            if token in self.token_embeddings:
                tag_vectors.append(self.token_embeddings[token].vector)
        
        # Generate description embedding
        description = f"{preset.name} {preset.sound_characteristics.timbral if preset.sound_characteristics else ''} {preset.sound_characteristics.material if preset.sound_characteristics else ''}"
        description_vector = self._encode_text(description)
        
        # Combine tag vectors and description
        if tag_vectors:
            avg_tag_vector = np.mean(tag_vectors, axis=0)
            entry_vector = self._normalize_vector(avg_tag_vector + description_vector)
        else:
            entry_vector = self._normalize_vector(description_vector)
        
        # Extract tags
        tags = list(all_tokens)
        
        return EntryEmbedding(
            preset_id=preset.name,
            vector=entry_vector,
            tags=tags,
            description=description,
            role=getattr(preset, 'role', 'unknown'),
            metadata={}
        )
    
    def compute_similarity(self, query: QueryEmbedding, entry: EntryEmbedding) -> float:
        """Compute similarity between query and entry"""
        # Check for zero vectors
        query_norm = np.linalg.norm(query.vector)
        entry_norm = np.linalg.norm(entry.vector)
        
        if query_norm == 0 or entry_norm == 0:
            return 0.0
        
        # Cosine similarity
        cosine_sim = np.dot(query.vector, entry.vector) / (query_norm * entry_norm)
        
        # Boost by shared tags
        shared_tags = set(query.tokens) & set(entry.tags)
        boost = 0.0
        for tag in shared_tags:
            if tag in self.idf_scores:
                boost += self.idf_scores[tag] * 0.1  # λ = 0.1
        
        # Clamp final score
        final_score = min(max(cosine_sim + boost, 0.0), 1.0)
        return final_score
    
    def search(self, query: QueryEmbedding, top_k: int = 10) -> List[Tuple[str, float]]:
        """Search for most similar entries"""
        similarities = []
        
        for entry_id, entry in self.entry_embeddings.items():
            similarity = self.compute_similarity(query, entry)
            similarities.append((entry_id, similarity))
        
        # Sort by similarity (descending)
        similarities.sort(key=lambda x: x[1], reverse=True)
        
        return similarities[:top_k]
    
    def _generate_token_vector(self, token: str) -> np.ndarray:
        """Generate embedding vector for a token"""
        # Use hash-based deterministic embedding
        hash_obj = hashlib.md5(token.encode())
        hash_bytes = hash_obj.digest()
        
        # Convert to float vector
        vector = np.zeros(self.embedding_dim)
        for i in range(min(len(hash_bytes), self.embedding_dim)):
            vector[i] = (hash_bytes[i] - 128) / 128.0  # Normalize to [-1, 1]
        
        # Add some semantic structure based on token patterns
        if any(word in token for word in ['warm', 'hot', 'cozy']):
            vector[0] = 1.0  # Warmth dimension
        elif any(word in token for word in ['bright', 'shiny', 'sparkly']):
            vector[1] = 1.0  # Brightness dimension
        elif any(word in token for word in ['dark', 'moody', 'ominous']):
            vector[1] = -1.0  # Dark dimension
        
        if any(word in token for word in ['fat', 'thick', 'punchy']):
            vector[2] = 1.0  # Weight dimension
        elif any(word in token for word in ['thin', 'light', 'delicate']):
            vector[2] = -1.0  # Lightness dimension
        
        return self._normalize_vector(vector)
    
    def _encode_text(self, text: str) -> np.ndarray:
        """Encode text into embedding vector"""
        tokenizer = SemanticTokenizer()
        tokens = tokenizer.tokenize(text)
        
        if not tokens:
            return np.zeros(self.embedding_dim)
        
        # Average token embeddings
        vectors = []
        for token in tokens:
            if token in self.token_embeddings:
                vectors.append(self.token_embeddings[token].vector)
            else:
                # Generate on-the-fly for unknown tokens
                vectors.append(self._generate_token_vector(token))
        
        if vectors:
            return self._normalize_vector(np.mean(vectors, axis=0))
        else:
            return np.zeros(self.embedding_dim)
    
    def _extract_text_fields(self, preset) -> List[str]:
        """Extract all text fields from a preset"""
        text_fields = [preset.name]
        
        if hasattr(preset, 'sound_characteristics') and preset.sound_characteristics:
            text_fields.extend([
                preset.sound_characteristics.timbral,
                preset.sound_characteristics.material,
                preset.sound_characteristics.dynamic
            ])
            
            if preset.sound_characteristics.emotional:
                for emotion in preset.sound_characteristics.emotional:
                    if isinstance(emotion, dict) and 'tag' in emotion:
                        text_fields.append(emotion['tag'])
        
        if hasattr(preset, 'topological_metadata') and preset.topological_metadata:
            text_fields.extend([
                preset.topological_metadata.damping,
                preset.topological_metadata.spectral_complexity,
                preset.topological_metadata.manifold_position
            ])
        
        return [str(field) for field in text_fields if field]
    
    def _normalize_vector(self, vector: np.ndarray) -> np.ndarray:
        """Normalize vector to unit length"""
        norm = np.linalg.norm(vector)
        if norm > 0:
            return vector / norm
        else:
            return vector

class SemanticSearchEngine:
    """Main search engine combining tokenization and embeddings"""
    
    def __init__(self, embedding_dim: int = 128):
        self.embedding_generator = EmbeddingGenerator(embedding_dim)
        self.entry_embeddings: Dict[str, EntryEmbedding] = {}
    
    def build_index(self, presets: List) -> None:
        """Build search index from presets"""
        # Build vocabulary
        self.embedding_generator.build_vocabulary(presets)
        
        # Generate entry embeddings
        for preset in presets:
            entry = self.embedding_generator.generate_entry_embedding(preset)
            self.entry_embeddings[preset.name] = entry
            self.embedding_generator.entry_embeddings[preset.name] = entry
    
    def search(self, query_text: str, role: str = None, tempo: float = None, key: str = None, top_k: int = 10) -> List[Tuple[str, float]]:
        """Search for presets matching query"""
        query = self.embedding_generator.generate_query_embedding(query_text, role, tempo, key)
        return self.embedding_generator.search(query, top_k)
    
    def get_entry(self, preset_id: str) -> Optional[EntryEmbedding]:
        """Get entry by ID"""
        return self.entry_embeddings.get(preset_id)

if __name__ == "__main__":
    from core_models import JsonPresetParser
    
    # Test the semantic search system
    parser = JsonPresetParser()
    presets = parser.parse_from_file('group.json')
    
    if presets:
        # Build search index
        search_engine = SemanticSearchEngine()
        search_engine.build_index(presets)
        
        print(f"Built index with {len(search_engine.entry_embeddings)} entries")
        
        # Test searches
        test_queries = [
            "warm analog pad",
            "bright energetic lead",
            "punchy bass",
            "dreamy ethereal texture"
        ]
        
        for query in test_queries:
            print(f"\nQuery: '{query}'")
            results = search_engine.search(query, top_k=3)
            for preset_id, score in results:
                print(f"  {preset_id}: {score:.3f}")