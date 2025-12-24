#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

static char to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

static bool is_alphanum(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

static std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string token;
    for (char c : text) {
        if (is_alphanum(c)) {
            token += to_lower(c);
        } else {
            if (token.size() >= 2) tokens.push_back(token);
            token.clear();
        }
    }
    if (token.size() >= 2) tokens.push_back(token);
    return tokens;
}

static std::string read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "Ошибка: не удалось открыть " << path << "\n";
        return "";
    }
    std::ostringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <файл.txt>\n";
        return 1;
    }
    std::string content = read_file(argv[1]);
    if (content.empty()) return 1;
    auto tokens = tokenize(content);
    for (const auto& t : tokens) std::cout << t << '\n';
    return 0;
}
