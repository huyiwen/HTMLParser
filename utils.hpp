#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <locale>
#include <codecvt>
#include <iostream>
#include <vector>
#include <set>
#include <regex>

// pop from top of stack safely
template<typename T>
inline void safe_pop(T &stack) {
    if (stack.size()) {
        // std::wcout << stack.top() << std::endl;
        if constexpr (std::is_same_v<T, std::vector<typename T::value_type>>) {
            stack.pop_back();
        } else {
            stack.pop();
        }
    } else {
        std::wcerr << "Warning: Trying to pop from an empty stack!" << std::endl;
    }
}

inline std::wstring strip(const std::wstring &src) {
    std::wregex ws (L"^\\s+|\\s+$");
    return std::regex_replace(src, ws, L"");
}

inline std::string strip(const std::string &src) {
    std::regex ws ("^\\s+|\\s+$");
    return std::regex_replace(src, ws, "");
}

// convert std::string to std::wstring
inline std::wstring to_wstring(const std::string& input) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    return converter.from_bytes(input);
}

// convert std::string to std::string
inline std::string to_byte_string(const std::wstring& input) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    return converter.to_bytes(input);
}

template<typename T, typename F>
inline void __split(const T &text, const F &filter, std::vector<T> &container, bool once = false, bool save_del = false) {
    size_t pos = 0, npos = 0;
    for (const auto &ele: text) {
        npos++;
        if (filter(ele)) {
            if (npos > 1) {
                container.push_back(text.substr(pos, npos-1));
                if (once) {
                    pos = npos;
                    npos = text.size() - npos;
                    break;
                }
            }
            pos += npos;
            npos = 0;
            if (save_del) {
                container.push_back(T(1, ele));
            }
        }
    }
    if (npos > 0) {
        container.push_back(text.substr(pos, npos));
    }
}

// split std::string
template<typename T, typename C = typename T::value_type>
inline void split(const T &text, const C &delimiter, std::vector<T> &container, bool once = false) {
    __split(text, [&delimiter](const C &ele) { return ele == delimiter; }, container, once);
}

template<typename T, typename C = typename T::value_type>
inline void split(const T &text, const std::set<C> &delimiters, std::vector<T> &container, bool once = false, bool save_del = false) {
    __split(text, [&delimiters](const C &ele) { return delimiters.count(ele); }, container, once, save_del);
}

template<typename T, typename C = typename T::value_type>
inline std::vector<T> split(const T &text, const C &delimiter, bool once = false) {
    std::vector<T> container;
    __split(text, [&delimiter](const C &ele) { return ele == delimiter; }, container, once);
    return container;
}

template<typename T, typename C = typename T::value_type>
inline std::vector<T> split(const T &text, const std::set<C> &delimiters, bool once = false, bool save_del = false) {
    std::vector<T> container;
    __split(text, [&delimiters](const C &ele) { return delimiters.count(ele); }, container, once, save_del);
    return container;
}

template<typename T>
inline void extend(T &lhs, const T &rhs) {
    lhs.reserve(std::size(lhs) + std::distance(std::begin(rhs), std::end(rhs)));
    lhs.insert(std::end(lhs), std::begin(rhs), std::end(rhs));
}

template<typename T, typename U>
inline void extend(std::set<T> &lhs, const U &rhs) {
    lhs.insert(std::begin(rhs), std::end(rhs));
}

// comparison ignoring cases
template<typename T>
inline int wild_compare(const T& __a, const T& __b) {
    T __ta(__a), __tb(__b);
    transform(__a.begin(), __a.end(), __ta.begin(), ::toupper);
    transform(__b.begin(), __b.end(), __tb.begin(), ::toupper);
    return __ta.compare(__tb);
}

inline void sub(const std::wstring &pattern, const std::wstring &repl, std::wstring & str) {
    std::wstring::size_type pos = 0, rp, lenp = pattern.size(), lenr = repl.size();
    while ((rp = str.find_first_of(pattern, pos)) != std::wstring::npos) {
        std::wcout << rp << L" " << lenp << L" " << repl << std::endl;
        str.replace(rp, lenp, repl);
        pos = rp + lenr + 1;
    }
}

// initialize Chinese environment (supported on Mac OS)
inline void init_chinese_env() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
#if defined(_MSC_VER)
    cout << "Warning: Initialization of Chinese environment hasn't been tested on Windows." << std::endl;
#endif
}

#endif // __UTILS_HPP__

