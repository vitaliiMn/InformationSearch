#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
#include <chrono>

struct TermRecord {
    std::string term;
    std::vector<int> doc_ids;
};

struct DocRecord {
    int doc_id;
    std::string title;
    std::string url;
};

static char to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

static std::vector<std::string> tokenize_query(const std::string& q) {
    std::vector<std::string> tokens;
    std::string cur;
    for (size_t i = 0; i < q.size(); ++i) {
        char c = q[i];
        if (c == '(' || c == ')' || c == '|' || c == '&' || c == '!') {
            if (!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
            if (c == '|' && i + 1 < q.size() && q[i + 1] == '|') {
                tokens.push_back("||");
                ++i;
            } else if (c == '&' && i + 1 < q.size() && q[i + 1] == '&') {
                tokens.push_back("&&");
                ++i;
            } else {
                tokens.push_back(std::string(1, c));
            }
        } else if (c == ' ') {
            if (!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
        } else {
            cur += to_lower(c);
        }
    }
    if (!cur.empty()) tokens.push_back(cur);
    return tokens;
}

static std::vector<int> get_doc_ids(const std::string& term, const std::vector<TermRecord>& inv) {
    for (const auto& tr : inv)
        if (tr.term == term)
            return tr.doc_ids;
    return {};
}

static std::vector<int> and_op(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> r;
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            r.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return r;
}

static std::vector<int> or_op(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> r;
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            r.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            r.push_back(a[i]);
            ++i;
        } else {
            r.push_back(b[j]);
            ++j;
        }
    }
    while (i < a.size()) r.push_back(a[i]), ++i;
    while (j < b.size()) r.push_back(b[j]), ++j;
    return r;
}

static std::vector<int> not_op(const std::vector<int>& a, int total) {
    std::set<int> excl(a.begin(), a.end());
    std::vector<int> r;
    for (int i = 0; i < total; ++i)
        if (excl.find(i) == excl.end())
            r.push_back(i);
    return r;
}

static std::vector<int> eval_expr(const std::vector<std::string>& t, size_t& p, const std::vector<TermRecord>& inv, int total);
static std::vector<int> eval_term(const std::vector<std::string>& t, size_t& p, const std::vector<TermRecord>& inv, int total);
static std::vector<int> eval_factor(const std::vector<std::string>& t, size_t& p, const std::vector<TermRecord>& inv, int total);

static std::vector<int> eval_expr(const std::vector<std::string>& t, size_t& p, const std::vector<TermRecord>& inv, int total) {
    auto left = eval_term(t, p, inv, total);
    while (p < t.size() && t[p] == "||") {
        ++p;
        auto right = eval_term(t, p, inv, total);
        left = or_op(left, right);
    }
    return left;
}

static std::vector<int> eval_term(const std::vector<std::string>& t, size_t& p, const std::vector<TermRecord>& inv, int total) {
    auto left = eval_factor(t, p, inv, total);
    while (p < t.size() && (t[p] == "&&" || t[p] == " ")) {
        ++p;
        auto right = eval_factor(t, p, inv, total);
        left = and_op(left, right);
    }
    return left;
}

static std::vector<int> eval_factor(const std::vector<std::string>& t, size_t& p, const std::vector<TermRecord>& inv, int total) {
    if (p >= t.size()) return {};
    if (t[p] == "!") {
        ++p;
        auto operand = eval_factor(t, p, inv, total);
        return not_op(operand, total);
    }
    if (t[p] == "(") {
        ++p;
        auto result = eval_expr(t, p, inv, total);
        if (p < t.size() && t[p] == ")") ++p;
        return result;
    }
    std::string term = t[p];
    ++p;
    return get_doc_ids(term, inv);
}

static std::vector<TermRecord> load_inv_index(const std::string& f) {
    std::ifstream in(f, std::ios::binary);
    if (!in) {
        std::cerr << "Ошибка: не удаётся открыть " << f << "\n";
        return {};
    }
    int n_terms;
    in.read(reinterpret_cast<char*>(&n_terms), sizeof(int));
    std::vector<TermRecord> idx;
    for (int i = 0; i < n_terms; ++i) {
        int len;
        in.read(reinterpret_cast<char*>(&len), sizeof(int));
        std::string term(len, '\0');
        in.read(&term[0], len);
        int n_docs;
        in.read(reinterpret_cast<char*>(&n_docs), sizeof(int));
        std::vector<int> docs(n_docs);
        for (int j = 0; j < n_docs; ++j)
            in.read(reinterpret_cast<char*>(&docs[j]), sizeof(int));
        idx.push_back({term, docs});
    }
    return idx;
}

static std::vector<DocRecord> load_fwd_index(const std::string& f) {
    std::ifstream in(f, std::ios::binary);
    if (!in) {
        std::cerr << "Ошибка: не удаётся открыть " << f << "\n";
        return {};
    }
    int n_docs;
    in.read(reinterpret_cast<char*>(&n_docs), sizeof(int));
    std::vector<DocRecord> idx;
    for (int i = 0; i < n_docs; ++i) {
        int id;
        in.read(reinterpret_cast<char*>(&id), sizeof(int));
        int len_title;
        in.read(reinterpret_cast<char*>(&len_title), sizeof(int));
        std::string title(len_title, '\0');
        in.read(&title[0], len_title);
        int len_url;
        in.read(reinterpret_cast<char*>(&len_url), sizeof(int));
        std::string url(len_url, '\0');
        in.read(&url[0], len_url);
        idx.push_back({id, title, url});
    }
    return idx;
}

static std::vector<int> search(const std::string& q, const std::vector<TermRecord>& inv, const std::vector<DocRecord>& fwd) {
    auto tokens = tokenize_query(q);
    size_t pos = 0;
    int total = static_cast<int>(fwd.size());
    return eval_expr(tokens, pos, inv, total);
}

static void print_results(const std::vector<int>& ids, const std::vector<DocRecord>& fwd) {
    for (int id : ids) {
        if (id >= 0 && id < static_cast<int>(fwd.size())) {
            const auto& dr = fwd[id];
            std::cout << dr.title << " | " << dr.url << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " \"запрос\"\n";
        return 1;
    }

    auto inv = load_inv_index("inverted_index.bin");
    auto fwd = load_fwd_index("forward_index.bin");
    if (inv.empty() || fwd.empty()) {
        std::cerr << "Не удалось загрузить индексы\n";
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto res = search(argv[1], inv, fwd);
    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
    std::cout << "Время: " << ms << " мс\n";
    std::cout << "Найдено: " << res.size() << "\n";
    print_results(res, fwd);
    return 0;
}
