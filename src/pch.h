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

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// spdlog
#include <spdlog/spdlog.h>

// json
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// clang-format on

#endif // TEDIT_PCH_
