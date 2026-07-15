#pragma once
#include <string>

namespace Engine {

/// <summary>
/// マルチバイト文字列(UTF-8)をワイド文字列(UTF-16)に変換する
/// </summary>
/// <param name="str">変換するマルチバイト文字列</param>
/// <returns>変換後のワイド文字列</returns>
[[nodiscard]] std::wstring ConvertString(const std::string& str);

/// <summary>
/// ワイド文字列(UTF-16)をマルチバイト文字列(UTF-8)に変換する
/// </summary>
/// <param name="str">変換するワイド文字列</param>
/// <returns>変換後のマルチバイト文字列</returns>
[[nodiscard]] std::string ConvertString(const std::wstring& str);

} // namespace Engine
