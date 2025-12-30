#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

char convert_to_lowercase(char symbol) {
    if (symbol >= 'A' && symbol <= 'Z') {
        return symbol + 32; // разница между 'a' и 'A'
    }
    return symbol;
}

bool is_alphanumeric(char symbol) {
    return (symbol >= 'a' && symbol <= 'z') ||
           (symbol >= 'A' && symbol <= 'Z') ||
           (symbol >= '0' && symbol <= '9');
}

std::vector<std::string> extract_words(const std::string& input_text) {
    std::vector<std::string> word_list;
    std::string current_word;

    for (char c : input_ext) {
        if (is_alphanumeric(c)) {
            current_word += convert_to_lowercase(c);
        } else {
            if (current_word.length() >= 2) {
                word_list.push_back(current_word);
            }
            current_word.clear();
        }
    }

    if (current_word.length() >= 2) {
        word_list.push_back(current_word);
    }

    return word_list;
}

std::string load_text_from_file(const std::string& file_path) {
    std::ifstream input_stream(file_path, std::ios::binary);
    if (!input_stream.is_open()) {
        std::cerr << "Ошибка: невозможно открыть файл '" << file_path << "'\n";
        return "";
    }

    std::ostringstream content_buffer;
    content_buffer << input_stream.rdbuf();
    return content_buffer.str();
}

int main(int argument_count, char* argument_values[]) {
    if (argument_count != 2) {
        std::cerr << "Использование: " << argument_values[0] << " <путь_к_файлу.txt>\n";
        return 1;
    }

    std::string file_content = load_text_from_file(argument_values[1]);
    if (file_content.empty()) {
        return 1;
    }

    std::vector<std::string> words = extract_words(file_content);
    for (const std::string& word : words) {
        std::cout << word << '\n';
    }

    return 0;
}
