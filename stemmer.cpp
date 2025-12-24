#ifndef PORTER_STEMMER_H
#define PORTER_STEMMER_H

#include <string>
#include <cctype>

class PorterStemmer {
private:
    std::string s;
    int len;

    inline char &at(int i) { return s[i]; }
    inline const char &at(int i) const { return s[i]; }

    bool is_consonant(int i) const {
        char c = at(i);
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') return false;
        if (c == 'y') return (i != 0) ? is_vowel(i - 1) : true;
        return true;
    }

    bool is_vowel(int i) const {
        return !is_consonant(i);
    }

    int measure() const {
        int m = 0;
        int i = 0;
        while (i < len) {
            while (i < len && is_consonant(i)) ++i;
            if (i >= len) break;
            ++i;
            while (i < len && is_vowel(i)) ++i;
            ++m;
        }
        return m;
    }

    bool ends_with(const char* suffix) {
        int suffix_len = 0, s_idx = 0;
        while (suffix[s_idx] != '\0') {
            ++suffix_len; ++s_idx;
        }
        if (suffix_len > len) return false;
        return s.compare(len - suffix_len, suffix_len, suffix) == 0;
    }

    void replace_suffix(const char* old_suff, const char* new_suff) {
        int old_len = 0;
        while (old_suff[old_len] != '\0') ++old_len;
        s.resize(len - old_len);
        s += new_suff;
        len = static_cast<int>(s.length());
    }

    bool step1b_vowel_check() {
        for (int i = 0; i < len; ++i)
            if (is_vowel(i)) return true;
        return false;
    }

    void step1a() {
        if (ends_with("sses")) replace_suffix("sses", "ss");
        else if (ends_with("ies")) replace_suffix("ies", "i");
        else if (ends_with("ss")) { /* ничего */ }
        else if (ends_with("s")) replace_suffix("s", "");
    }

    void step1b() {
        if (ends_with("eed")) {
            if (measure() > 0) s[len-1] = 'e';
        } else if (ends_with("ed") || ends_with("ing")) {
            int old_len = len;
            if (ends_with("ed")) replace_suffix("ed", "");
            else replace_suffix("ing", "");
            if (old_len != len && step1b_vowel_check()) {
                if (ends_with("at") || ends_with("bl") || ends_with("iz"))
                    s += 'e';
                else {
                    char c = s.back();
                    if (c == 'b' || c == 'c' || c == 'd' || c == 'f' || c == 'g' ||
                        c == 'h' || c == 'j' || c == 'k' || c == 'l' || c == 'm' ||
                        c == 'n' || c == 'p' || c == 'r' || c == 's' || c == 't' ||
                        c == 'v' || c == 'w' || c == 'x' || c == 'y' || c == 'z') {
                        if (len >= 2 && s[len-1] == s[len-2] && 
                            s.back() != 'l' && s.back() != 's' && s.back() != 'z')
                            s.pop_back();
                    } else s += 'e';
                }
            }
        }
    }

    void step1c() {
        if (ends_with("y") && len > 1 && is_consonant(len - 2))
            s[len-1] = 'i';
    }

    void step2() {
        if (len < 2) return;
        if (measure() == 0) return;
        if (ends_with("ational")) replace_suffix("ational", "ate");
        else if (ends_with("tional")) replace_suffix("tional", "tion");
        else if (ends_with("enci")) replace_suffix("enci", "ence");
        else if (ends_with("anci")) replace_suffix("anci", "ance");
        else if (ends_with("izer")) replace_suffix("izer", "ize");
        else if (ends_with("abli")) replace_suffix("abli", "able");
        else if (ends_with("alli")) replace_suffix("alli", "al");
        else if (ends_with("entli")) replace_suffix("entli", "ent");
        else if (ends_with("eli")) replace_suffix("eli", "e");
        else if (ends_with("ousli")) replace_suffix("ousli", "ous");
        else if (ends_with("ization")) replace_suffix("ization", "ize");
        else if (ends_with("ation")) replace_suffix("ation", "ate");
        else if (ends_with("ator")) replace_suffix("ator", "ate");
        else if (ends_with("alism")) replace_suffix("alism", "al");
        else if (ends_with("iveness")) replace_suffix("iveness", "ive");
        else if (ends_with("fulness")) replace_suffix("fulness", "ful");
        else if (ends_with("ousness")) replace_suffix("ousness", "ous");
        else if (ends_with("aliti")) replace_suffix("aliti", "al");
        else if (ends_with("iviti")) replace_suffix("iviti", "ive");
        else if (ends_with("biliti")) replace_suffix("biliti", "ble");
    }

    void step3() {
        if (len < 2) return;
        if (measure() == 0) return;
        if (ends_with("icate")) replace_suffix("icate", "ic");
        else if (ends_with("ative")) replace_suffix("ative", "");
        else if (ends_with("alize")) replace_suffix("alize", "al");
        else if (ends_with("iciti")) replace_suffix("iciti", "ic");
        else if (ends_with("ical")) replace_suffix("ical", "ic");
        else if (ends_with("ful")) replace_suffix("ful", "");
        else if (ends_with("ness")) replace_suffix("ness", "");
    }

    void step4() {
        if (len < 2) return;
        if (measure() <= 1) return;
        if (ends_with("al")) replace_suffix("al", "");
        else if (ends_with("ance")) replace_suffix("ance", "");
        else if (ends_with("ence")) replace_suffix("ence", "");
        else if (ends_with("er")) replace_suffix("er", "");
        else if (ends_with("ic")) replace_suffix("ic", "");
        else if (ends_with("able")) replace_suffix("able", "");
        else if (ends_with("ible")) replace_suffix("ible", "");
        else if (ends_with("ant")) replace_suffix("ant", "");
        else if (ends_with("ement")) replace_suffix("ement", "");
        else if (ends_with("ment")) replace_suffix("ment", "");
        else if (ends_with("ent")) replace_suffix("ent", "");
        else if (ends_with("ion") && len >= 2 && 
                (s[len-3] == 's' || s[len-3] == 't')) replace_suffix("ion", "");
        else if (ends_with("ou")) replace_suffix("ou", "");
        else if (ends_with("ism")) replace_suffix("ism", "");
        else if (ends_with("ate")) replace_suffix("ate", "");
        else if (ends_with("iti")) replace_suffix("iti", "");
        else if (ends_with("ous")) replace_suffix("ous", "");
        else if (ends_with("ive")) replace_suffix("ive", "");
        else if (ends_with("ize")) replace_suffix("ize", "");
    }

    void step5a() {
        if (len < 2) return;
        if (measure() > 1 && ends_with("e")) {
            s.pop_back();
            --len;
        } else if (measure() == 1 && ends_with("e") && 
                   !(len >= 2 && is_consonant(len-2) && !is_consonant(len-3) && is_consonant(len-4))) {
            s.pop_back();
            --len;
        }
    }

    void step5b() {
        if (measure() > 1 && len >= 2 && s[len-1] == 'l' && s[len-2] == 'l')
            s.pop_back();
    }

public:
    std::string stem(const std::string& word) {
        if (word.length() < 3) return word;
        
        // Приведение к нижнему регистру
        s.clear();
        for (char c : word) {
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
            s += c;
        }
        
        len = static_cast<int>(s.length());
        if (len < 3) return s;
        step1a();
        step1b();
        step1c();
        step2();
        step3();
        step4();
        step5a();
        step5b();

        return s;
    }
};

#endif // PORTER_STEMMER_H
