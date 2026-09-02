#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <iomanip>
#include <regex>
#include <chrono>
#include <algorithm>

namespace fs = std::filesystem;

struct stats_t {
    int file_count = 0;
    int blank_count = 0;
    int comment_count = 0;
    int code_count = 0;
};

bool starts_with(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

std::string detect_language(const fs::path& path) {
    static const std::map<std::string, std::string> ext_map = {
        {".cpp", "C++"}, {".cc", "C++"}, {".cxx", "C++"},
        {".h", "C/C++ Header"}, {".hpp", "C/C++ Header"},
        {".json", "JSON"}, {".xml", "XML"},
        {".py", "Python"}, {".md", "Markdown"},
        {".sln", "Visual Studio Solution"}
    };

    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    auto it = ext_map.find(ext);
    return (it != ext_map.end()) ? it->second : "Unknown";
}

void count_file_lines(const fs::path& file, stats_t& stats, const std::string& lang) {
    std::ifstream in(file);
    if (!in.is_open()) return;

    std::string line;
    bool in_block_comment = false;

    while (std::getline(in, line)) {
        // yeah, regex is overkill for trimming whitespace, but it's short and i'm lazy
        std::string trimmed = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

        if (trimmed.empty()) {
            stats.blank_count++;
        } 
        else if (lang == "C++" || lang == "C/C++ Header") {
            // state machine for block comments
            if (in_block_comment) {
                stats.comment_count++;
                if (trimmed.find("*/") != std::string::npos)
                    in_block_comment = false;
            } 
            else if (starts_with(trimmed, "//")) {
                stats.comment_count++;
            } 
            else if (trimmed.find("/*") != std::string::npos) {
                stats.comment_count++;
                // if the block doesn't close on the same line, stay in comment mode
                if (trimmed.find("*/") == std::string::npos)
                    in_block_comment = true;
            } 
            else {
                stats.code_count++;
            }
        } 
        else if (lang == "Python") {
            if (starts_with(trimmed, "#"))
                stats.comment_count++;
            else
                stats.code_count++;
        } 
        else {
            // for everything else (json, xml, markdown), just count as code
            stats.code_count++;
        }
    }
}

int main(int argc, char* argv[]) {
    fs::path root = (argc > 1) ? fs::path(argv[1]) : fs::current_path();

    std::map<std::string, stats_t> language_stats;
    int total_files = 0;
    int ignored_files = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (auto& entry : fs::recursive_directory_iterator(root)) {
        if (!fs::is_regular_file(entry.path())) continue;
        total_files++;

        std::string lang = detect_language(entry.path());
        if (lang == "Unknown") {
            ignored_files++;
            continue;
        }

        stats_t& s = language_stats[lang];
        s.file_count++;
        count_file_lines(entry.path(), s, lang);
    }

    int unique_files = 0;
    int total_blank = 0;
    int total_comment = 0;
    int total_code = 0;

    for (auto& [lang, s] : language_stats) {
        unique_files += s.file_count;
        total_blank += s.blank_count;
        total_comment += s.comment_count;
        total_code += s.code_count;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(end_time - start_time).count();

    std::cout << std::fixed;
    std::cout << std::setw(8) << total_files << " text files.\n";
    std::cout << std::setw(8) << unique_files << " unique files.\n";
    std::cout << std::setw(8) << ignored_files << " files ignored.\n\n";

    std::cout << "cloc | created by ellii <3  "
              << "T=" << std::setprecision(2) << seconds << " s ("
              << std::setprecision(1) << (unique_files / seconds)
              << " files/s, "
              << std::setprecision(1) << (total_code / seconds)
              << " lines/s)\n";

    std::cout << "------------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(35) << "Language"
              << std::right << std::setw(15) << "files"
              << std::setw(15) << "blank"
              << std::setw(15) << "comment"
              << std::setw(15) << "code" << "\n";
    std::cout << "------------------------------------------------------------------------------------\n";

    for (auto& [lang, s] : language_stats) {
        std::cout << std::left << std::setw(35) << lang
                  << std::right << std::setw(15) << s.file_count
                  << std::setw(15) << s.blank_count
                  << std::setw(15) << s.comment_count
                  << std::setw(15) << s.code_count << "\n";
    }

    std::cout << "------------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(35) << "SUM:"
              << std::right << std::setw(15) << unique_files
              << std::setw(15) << total_blank
              << std::setw(15) << total_comment
              << std::setw(15) << total_code << "\n";
    std::cout << "------------------------------------------------------------------------------------\n";

    return 0;
}
