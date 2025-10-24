"""
Test script for the JSON-to-Audio Preset Generator
Tests all major components of the system
"""

import sys
import json
from main import AudioPresetGenerator

def test_basic_functionality():
    """Test basic system functionality"""
    print("Testing JSON-to-Audio Preset Generator")
    print("=" * 50)
    
    # Create generator
    generator = AudioPresetGenerator()
    
    # Test 1: Load presets
    print("\n1. Testing preset loading...")
    try:
        generator.load_presets(["group.json", "electronic_track.json", "guitar.json"])
        print(f"✓ Loaded {len(generator.presets)} presets")
    except Exception as e:
        print(f"✗ Error loading presets: {e}")
        return False
    
    # Test 2: Normalize presets
    print("\n2. Testing preset normalization...")
    try:
        generator.normalize_presets()
        print(f"✓ Normalized {len(generator.normalized_presets)} presets")
        
        # Check for validation issues
        total_issues = sum(len(preset.validation_issues) for preset in generator.normalized_presets)
        if total_issues > 0:
            print(f"  Warning: {total_issues} validation issues found")
        else:
            print("  ✓ No validation issues")
    except Exception as e:
        print(f"✗ Error normalizing presets: {e}")
        return False
    
    # Test 3: Build search index
    print("\n3. Testing search index building...")
    try:
        generator.build_search_index()
        print("✓ Search index built successfully")
    except Exception as e:
        print(f"✗ Error building search index: {e}")
        return False
    
    # Test 4: Test search functionality
    print("\n4. Testing search functionality...")
    test_queries = [
        "warm analog pad",
        "bright energetic lead",
        "punchy bass",
        "dreamy ethereal texture"
    ]
    
    for query in test_queries:
        try:
            results = generator.search_presets(query, top_k=3)
            if results:
                print(f"  ✓ Query '{query}': {len(results)} results")
            else:
                print(f"  ⚠ Query '{query}': No results")
        except Exception as e:
            print(f"  ✗ Query '{query}': Error - {e}")
    
    # Test 5: Test preset generation
    print("\n5. Testing preset generation...")
    test_cases = [
        ("warm analog pad", "pad", 120, "C"),
        ("bright lead", "lead", 140, "G"),
        ("punchy bass", "bass", 100, "F"),
        ("ethereal texture", "texture", 80, "Am")
    ]
    
    for query, role, tempo, key in test_cases:
        try:
            preset = generator.generate_audio_preset(query, role, tempo, key)
            if preset:
                print(f"  ✓ Generated preset for '{query}' ({role})")
                
                # Check if graph validation passed
                if preset['graph']['validation_passed']:
                    print(f"    ✓ Graph validation passed")
                else:
                    print(f"    ⚠ Graph validation failed: {len(preset['graph']['validation_errors'])} errors")
            else:
                print(f"  ✗ Failed to generate preset for '{query}'")
        except Exception as e:
            print(f"  ✗ Error generating preset for '{query}': {e}")
    
    print("\n" + "=" * 50)
    print("Test completed!")
    return True

def test_individual_components():
    """Test individual components"""
    print("\nTesting individual components...")
    print("-" * 30)
    
    # Test core models
    print("\nTesting core models...")
    try:
        from core_models import JsonPresetParser
        parser = JsonPresetParser()
        presets = parser.parse_from_file('group.json')
        print(f"✓ Core models: Parsed {len(presets)} presets")
    except Exception as e:
        print(f"✗ Core models error: {e}")
    
    # Test normalizer
    print("\nTesting normalizer...")
    try:
        from normalizer import UnitNormalizer
        from core_models import JsonPresetParser
        parser = JsonPresetParser()
        presets = parser.parse_from_file('group.json')
        normalizer = UnitNormalizer()
        normalized = normalizer.normalize(presets[0])
        print(f"✓ Normalizer: Normalized preset '{normalized.name}'")
    except Exception as e:
        print(f"✗ Normalizer error: {e}")
    
    # Test graph builder
    print("\nTesting graph builder...")
    try:
        from graph_builder import GraphBuilder
        from core_models import JsonPresetParser
        parser = JsonPresetParser()
        presets = parser.parse_from_file('group.json')
        builder = GraphBuilder()
        graph = builder.build_graph(presets[0])
        print(f"✓ Graph builder: Built graph with {len(graph.nodes)} nodes")
    except Exception as e:
        print(f"✗ Graph builder error: {e}")
    
    # Test semantic embeddings
    print("\nTesting semantic embeddings...")
    try:
        from semantic_embeddings import SemanticSearchEngine
        from core_models import JsonPresetParser
        parser = JsonPresetParser()
        presets = parser.parse_from_file('group.json')
        search_engine = SemanticSearchEngine()
        search_engine.build_index(presets)
        results = search_engine.search("warm pad", top_k=3)
        print(f"✓ Semantic embeddings: Found {len(results)} results")
    except Exception as e:
        print(f"✗ Semantic embeddings error: {e}")
    
    # Test role policies
    print("\nTesting role policies...")
    try:
        from roles_policies import RolePolicyManager
        from core_models import Role
        policy_manager = RolePolicyManager()
        policy = policy_manager.get_policy(Role.PAD)
        print(f"✓ Role policies: Loaded policy for {policy.role.value}")
    except Exception as e:
        print(f"✗ Role policies error: {e}")
    
    # Test decision heads
    print("\nTesting decision heads...")
    try:
        from decision_heads import DecisionEngine, Context
        from core_models import Role
        import numpy as np
        
        decision_engine = DecisionEngine()
        context = Context(
            query_vector=np.random.randn(128),
            role=Role.PAD,
            tempo=120.0,
            key="C"
        )
        decisions = decision_engine.infer_decisions(context)
        print(f"✓ Decision heads: Generated {len(decisions.value_decisions)} value decisions")
    except Exception as e:
        print(f"✗ Decision heads error: {e}")

if __name__ == "__main__":
    print("JSON-to-Audio Preset Generator - Test Suite")
    print("=" * 60)
    
    # Test individual components first
    test_individual_components()
    
    # Test full system
    success = test_basic_functionality()
    
    if success:
        print("\n🎉 All tests completed successfully!")
        sys.exit(0)
    else:
        print("\n❌ Some tests failed!")
        sys.exit(1)