# Current Issues Analysis

## Primary Problem
**Windows MinGW Build Fails with jsoncpp string_view Linker Errors**

### Error Details
```
undefined reference to `Json::Value::isMember(std::basic_string_view<char, std::char_traits<char> >) const'
undefined reference to `Json::Value::operator[](std::basic_string_view<char, std::char_traits<char> >)'
```

### Root Cause
- MinGW jsoncpp library is built WITHOUT std::string_view support
- Code was using string literals that implicitly convert to string_view
- Linker can't find the string_view overloads that don't exist in MinGW build

## Current Fix Status
✅ **FIXED**: Replaced all string literals with `std::string("key")` in `src/dsp_ir.cpp`
- 15 lines changed total
- Forces use of std::string overloads (available in MinGW)
- Avoids string_view overloads (not available in MinGW)

## Files Modified
- `src/dsp_ir.cpp` - JSON parsing functions in IRParser::parsePreset()

## Build System Issues
- Missing yaml-cpp development headers in current environment
- CMakeLists.txt expects both jsoncpp and yaml-cpp
- Need to verify all dependencies are available for Windows build

## Next Steps Needed
1. Verify the fix works in actual MinGW build
2. Check if all dependencies are properly installed
3. Test complete build process
4. Document any remaining issues