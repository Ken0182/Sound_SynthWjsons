#!/usr/bin/env python3
"""
Schema Validator for Audio Presets
Validates presets against the canonical schema defined in config/preset_schema.yaml
"""

import yaml
import json
from typing import Dict, List, Any, Optional, Set
from pathlib import Path
import logging

logger = logging.getLogger(__name__)


class PresetSchemaValidator:
    """Validates audio presets against the canonical schema."""
    
    def __init__(self, schema_path: str = "config/preset_schema.yaml"):
        """
        Initialize the validator with a schema.
        
        Args:
            schema_path: Path to the schema YAML file
        """
        self.schema_path = schema_path
        self.schema = self._load_schema()
        self.errors: List[str] = []
        self.warnings: List[str] = []
    
    def _load_schema(self) -> Dict[str, Any]:
        """Load the schema from YAML file."""
        try:
            with open(self.schema_path, 'r') as f:
                return yaml.safe_load(f)
        except Exception as e:
            logger.error(f"Failed to load schema: {e}")
            return {}
    
    def validate(self, preset: Dict[str, Any]) -> bool:
        """
        Validate a preset against the schema.
        
        Args:
            preset: Preset dictionary to validate
            
        Returns:
            True if valid, False otherwise
        """
        self.errors = []
        self.warnings = []
        
        # Check required fields
        self._validate_required_fields(preset)
        
        # Validate preset structure
        self._validate_preset_structure(preset)
        
        # Validate stages
        if "stages" in preset:
            self._validate_stages(preset["stages"])
        
        # Validate connections
        if "connections" in preset:
            stage_ids = {s["id"] for s in preset.get("stages", [])}
            self._validate_connections(preset["connections"], stage_ids)
        
        # Validate context (if present)
        if "context" in preset:
            self._validate_context(preset["context"])
        
        # Validate constraints (if present)
        if "constraints" in preset:
            self._validate_constraints(preset["constraints"])
        
        return len(self.errors) == 0
    
    def _validate_required_fields(self, preset: Dict[str, Any]):
        """Validate that all required fields are present."""
        required_fields = self.schema.get("validation", {}).get("required_fields", [])
        
        for field in required_fields:
            if field not in preset:
                self.errors.append(f"Missing required field: {field}")
    
    def _validate_preset_structure(self, preset: Dict[str, Any]):
        """Validate the preset structure."""
        # Validate name
        if "name" in preset and not isinstance(preset["name"], str):
            self.errors.append("Preset name must be a string")
        
        # Validate role
        if "role" in preset:
            valid_roles = ["pad", "bass", "lead", "drum", "fx", "unknown"]
            if preset["role"] not in valid_roles:
                self.errors.append(f"Invalid role: {preset['role']}, must be one of {valid_roles}")
        
        # Validate tags (if present)
        if "tags" in preset:
            if not isinstance(preset["tags"], list):
                self.errors.append("Tags must be an array")
            elif not all(isinstance(tag, str) for tag in preset["tags"]):
                self.errors.append("All tags must be strings")
    
    def _validate_stages(self, stages: List[Dict[str, Any]]):
        """Validate DSP stages."""
        if not isinstance(stages, list):
            self.errors.append("Stages must be an array")
            return
        
        # Check stage count
        min_stages = self.schema.get("validation", {}).get("min_stages", 1)
        max_stages = self.schema.get("validation", {}).get("max_stages", 32)
        
        if len(stages) < min_stages:
            self.errors.append(f"Must have at least {min_stages} stage(s)")
        if len(stages) > max_stages:
            self.errors.append(f"Cannot exceed {max_stages} stages")
        
        # Validate each stage
        stage_ids: Set[str] = set()
        valid_stage_types = self.schema.get("stages", {}).get("item_schema", {}).get("type", {}).get("values", [])
        
        for i, stage in enumerate(stages):
            # Check required fields
            if "id" not in stage:
                self.errors.append(f"Stage {i} missing required field: id")
                continue
            
            if "type" not in stage:
                self.errors.append(f"Stage {i} ({stage['id']}) missing required field: type")
                continue
            
            # Check for duplicate IDs
            stage_id = stage["id"]
            if stage_id in stage_ids:
                self.errors.append(f"Duplicate stage ID: {stage_id}")
            stage_ids.add(stage_id)
            
            # Validate stage type
            stage_type = stage["type"]
            if stage_type not in valid_stage_types:
                self.errors.append(f"Invalid stage type '{stage_type}' for stage {stage_id}")
                continue
            
            # Validate parameters
            if "parameters" in stage:
                self._validate_stage_parameters(stage_id, stage_type, stage["parameters"])
    
    def _validate_stage_parameters(self, stage_id: str, stage_type: str, params: Dict[str, Any]):
        """Validate stage-specific parameters."""
        schema_params = self.schema.get("stage_parameters", {}).get(stage_type, {})
        
        for param_name, param_value in params.items():
            if param_name not in schema_params:
                self.warnings.append(f"Unknown parameter '{param_name}' for {stage_type} stage {stage_id}")
                continue
            
            param_schema = schema_params[param_name]
            param_type = param_schema.get("type")
            
            # Validate type
            if param_type == "float":
                if not isinstance(param_value, (int, float)):
                    self.errors.append(f"Parameter {param_name} of stage {stage_id} must be numeric")
                else:
                    # Validate range
                    if "range" in param_schema:
                        min_val, max_val = param_schema["range"]
                        if not (min_val <= param_value <= max_val):
                            self.errors.append(
                                f"Parameter {param_name} of stage {stage_id} out of range "
                                f"[{min_val}, {max_val}]: {param_value}"
                            )
            
            elif param_type == "enum":
                valid_values = param_schema.get("values", [])
                if param_value not in valid_values:
                    self.errors.append(
                        f"Parameter {param_name} of stage {stage_id} must be one of {valid_values}, "
                        f"got: {param_value}"
                    )
            
            elif param_type == "boolean":
                if not isinstance(param_value, bool):
                    self.errors.append(f"Parameter {param_name} of stage {stage_id} must be boolean")
    
    def _validate_connections(self, connections: List[Dict[str, Any]], stage_ids: Set[str]):
        """Validate DSP graph connections."""
        if not isinstance(connections, list):
            self.errors.append("Connections must be an array")
            return
        
        # Check connection count
        max_connections = self.schema.get("validation", {}).get("max_connections", 128)
        if len(connections) > max_connections:
            self.errors.append(f"Cannot exceed {max_connections} connections")
        
        # Validate each connection
        for i, conn in enumerate(connections):
            if "from" not in conn:
                self.errors.append(f"Connection {i} missing 'from' field")
                continue
            
            if "to" not in conn:
                self.errors.append(f"Connection {i} missing 'to' field")
                continue
            
            # Check that stages exist
            if conn["from"] not in stage_ids:
                self.errors.append(f"Connection {i} references non-existent source stage: {conn['from']}")
            
            if conn["to"] not in stage_ids:
                self.errors.append(f"Connection {i} references non-existent destination stage: {conn['to']}")
            
            # Validate weight (if present)
            if "weight" in conn:
                weight = conn["weight"]
                if not isinstance(weight, (int, float)):
                    self.errors.append(f"Connection {i} weight must be numeric")
                elif not (0.0 <= weight <= 1.0):
                    self.errors.append(f"Connection {i} weight must be in range [0, 1]: {weight}")
    
    def _validate_context(self, context: Dict[str, Any]):
        """Validate musical context."""
        # Validate tempo
        if "tempo" in context:
            tempo = context["tempo"]
            if not isinstance(tempo, (int, float)):
                self.errors.append("Context tempo must be numeric")
            elif not (20.0 <= tempo <= 300.0):
                self.errors.append(f"Context tempo out of range [20, 300]: {tempo}")
        
        # Validate key
        if "key" in context:
            key = context["key"]
            if not isinstance(key, int):
                self.errors.append("Context key must be an integer")
            elif not (0 <= key <= 11):
                self.errors.append(f"Context key out of range [0, 11]: {key}")
        
        # Validate scale
        if "scale" in context:
            valid_scales = ["major", "minor", "dorian", "phrygian", "lydian", "mixolydian", "locrian"]
            if context["scale"] not in valid_scales:
                self.errors.append(f"Invalid scale: {context['scale']}, must be one of {valid_scales}")
    
    def _validate_constraints(self, constraints: Dict[str, Any]):
        """Validate rendering constraints."""
        constraint_schema = self.schema.get("constraints", {})
        
        for constraint_name, constraint_value in constraints.items():
            if constraint_name not in constraint_schema:
                self.warnings.append(f"Unknown constraint: {constraint_name}")
                continue
            
            schema = constraint_schema[constraint_name]
            
            # Validate type
            if not isinstance(constraint_value, (int, float)):
                self.errors.append(f"Constraint {constraint_name} must be numeric")
                continue
            
            # Validate range
            if "range" in schema:
                min_val, max_val = schema["range"]
                if not (min_val <= constraint_value <= max_val):
                    self.errors.append(
                        f"Constraint {constraint_name} out of range "
                        f"[{min_val}, {max_val}]: {constraint_value}"
                    )
    
    def get_validation_report(self) -> str:
        """Get a formatted validation report."""
        report = []
        
        if self.errors:
            report.append("ERRORS:")
            for error in self.errors:
                report.append(f"  - {error}")
        
        if self.warnings:
            report.append("WARNINGS:")
            for warning in self.warnings:
                report.append(f"  - {warning}")
        
        if not self.errors and not self.warnings:
            report.append("Validation passed successfully!")
        
        return "\n".join(report)


def validate_preset_file(preset_path: str, schema_path: str = "config/preset_schema.yaml") -> bool:
    """
    Validate a preset JSON file.
    
    Args:
        preset_path: Path to preset JSON file
        schema_path: Path to schema YAML file
        
    Returns:
        True if valid, False otherwise
    """
    try:
        with open(preset_path, 'r') as f:
            preset = json.load(f)
        
        validator = PresetSchemaValidator(schema_path)
        is_valid = validator.validate(preset)
        
        print(f"\nValidation report for {preset_path}:")
        print(validator.get_validation_report())
        
        return is_valid
    
    except Exception as e:
        logger.error(f"Failed to validate preset file: {e}")
        return False


if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python schema_validator.py <preset_file.json>")
        sys.exit(1)
    
    preset_file = sys.argv[1]
    is_valid = validate_preset_file(preset_file)
    
    sys.exit(0 if is_valid else 1)
