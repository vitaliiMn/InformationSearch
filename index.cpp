#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <dirent.h>

#ifdef _WIN32
    #include <windows.h>
#endif

static char to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

static std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string token;
    for (char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            token += to_lower(c);
        } else {
            if (token.size() >= 2) tokens.push_back(token);
            token.clear();
        }
    }
    if (token.size() >= 2) tokens.push_back(token);
    return tokens;
}

struct TermRecord {
    std::string term;
    std::vector<int> doc_ids;
};

struct DocRecord {
    int doc_id;
    std::string title;
    std::string url;
};

static bool term_less(const TermRecord& a, const TermRecord& b) {
    return a.term < b.term;
}

static std::vector<std::string> list_txt_files(const std::string& dir) {
    std::vector<std::string> files;
    DIR* dp = opendir(dir.c_str());
    if (!dp) {
        std::cerr << "Ошибка: не удалось открыть директорию " << dir << std::endl;
        return files;
    }
    struct dirent* ep;
    while ((ep = readdir(dp))) {
        std::string name = ep->d_name;
        if (name == "." || name == "..") continue;
        if (name.size() > 4 && name.substr(name.size() - 4) == ".txt")
            files.push_back(name);
    }
    closedir(dp);
    std::sort(files.begin(), files.end());
    return files;
}

int main() {
    const std::string corpus_dir = "corpus";
    const std::string inv_file = "inverted_index.bin";
    const std::string fwd_file = "forward_index.bin";

    std::vector<TermRecord> inv_index;
    std::vector<DocRecord> fwd_index;

    auto filenames = list_txt_files(corpus_dir);
    if (filenames.empty()) {
        std::cerr << "Нет файлов в " << corpus_dir << std::endl;
        return 1;
    }

    size_t n = filenames.size();
    std::cout << "Найдено " << n << " файлов\n";

    for (size_t i = 0; i < n; ++i) {
        std::string path = corpus_dir + "/" + filenames[i];
        std::ifstream f(path, std::ios::binary);
        if (!f) {
            std::cerr << "Не удалось открыть: " << path << std::endl;
            continue;
        }
        std::ostringstream buf;
        buf << f.rdbuf();
        std::string content = buf.str();
        f.close();

        auto tokens = tokenize(content);

        DocRecord doc;
        doc.doc_id = static_cast<int>(i);
        size_t dot = filenames[i].find('.');
        doc.title = (dot != std::string::npos) ? filenames[i].substr(0, dot) : filenames[i];
        size_t underscore = doc.title.find('_');
        std::string site = (underscore != std::string::npos) ? doc.title.substr(0, underscore) : "unknown";
        doc.url = "https://" + site + ".stackexchange.com/questions/" + doc.title.substr(underscore + 1);
        fwd_index.push_back(doc);

        for (const auto& tok : tokens) {
            bool found = false;
            for (auto& tr : inv_index) {
                if (tr.term == tok) {
                    tr.doc_ids.push_back(static_cast<int>(i));
                    found = true;
                    break;
                }
            }
            if (!found) {
                TermRecord tr;
                tr.term = tok;
                tr.doc_ids.push_back(static_cast<int>(i));
                inv_index.push_back(tr);
            }
        }

        if ((i + 1) % 1000 == 0)
            std::cout << "Обработано: " << (i + 1) << "\n";
    }

    std::sort(inv_index.begin(), inv_index.end(), term_less);

    std::ofstream inv_out(inv_file, std::ios::binary);
    if (!inv_out) {
        std::cerr << "Не удалось создать " << inv_file << std::endl;
        return 1;
    }
    int num_terms = static_cast<int>(inv_index.size());
    inv_out.write(reinterpret_cast<const char*>(&num_terms), sizeof(int));
    for (const auto& tr : inv_index) {
        int len = static_cast<int>(tr.term.size());
        inv_out.write(reinterpret_cast<const char*>(&len), sizeof(int));
        inv_out.write(tr.term.c_str(), len);
        int cnt = static_cast<int>(tr.doc_ids.size());
        inv_out.write(reinterpret_cast<const char*>(&cnt), sizeof(int));
        for (int id : tr.doc_ids)
            inv_out.write(reinterpret_cast<const char*>(&id), sizeof(int));
    }
    inv_out.close();

    std::ofstream fwd_out(fwd_file, std::ios::binary);
    if (!fwd_out) {
        std::cerr << "Не удалось создать " << fwd_file << std::endl;
        return 1;
    }
    int num_docs = static_cast<int>(fwd_index.size());
    fwd_out.write(reinterpret_cast<const char*>(&num_docs), sizeof(int));
    for (const auto& dr : fwd_index) {
        fwd_out.write(reinterpret_cast<const char*>(&dr.doc_id), sizeof(int));
        int len_title = static_cast<int>(dr.title.size());
        fwd_out.write(reinterpret_cast<const char*>(&len_title), sizeof(int));
        fwd_out.write(dr.title.c_str(), len_title);
        int len_url = static_cast<int>(dr.url.size());
        fwd_out.write(reinterpret_cast<const char*>(&len_url), sizeof(int));
        fwd_out.write(dr.url.c_str(), len_url);
    }
    fwd_out.close();

    std::cout << "\nИндексы построены.\n";
    std::cout << "Документов: " << num_docs << "\n";
    std::cout << "Терминов: " << num_terms << "\n";
    return 0;
}
