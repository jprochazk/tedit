#ifndef TEDIT_PCH_
#define TEDIT_PCH_

// clang-format off
// STL
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>
#include <optional>
#include <array>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;
#include <cmath>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

// json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// windows
#ifdef _WIN32
#include <windows.h>
#endif
// clang-format on

#endif // TEDIT_PCH_
