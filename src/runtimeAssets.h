#pragma once
#include <filesystem>
#include <string>
void setAssetDirectory(const std::filesystem::path& directory);
std::string assetPath(const std::string& relative);
void validateAssets();
