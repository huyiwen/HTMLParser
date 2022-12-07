#include <iostream>
#include <string>
#include <istream>
#include <fstream>
#include <cstring>

#include <vector>
#include <unordered_set>
#include <set>
#include <map>
#include <stack>
#include <regex>

#include <cstdlib>
#include <cassert>
#include <locale>
#include <functional>
#include <codecvt>
#include <memory>
#include "utils.hpp"
#include "HTMLParser.hpp"

using std::string;     using std::wstring;
using std::cin;        using std::wcin;
using std::cout;        using std::wcout;
using std::wifstream;
using std::endl;

using std::vector;
using std::stack;
using std::multimap;

using std::shared_ptr;
using std::weak_ptr;
using std::function;


#define T(x) cout<<"\n\nTesting: "#x"\n"<<endl;(x)();cout<<"Test Passed!"<<endl


inline void test_split() {
    vector<wstring> res;
    vector<wstring> res2;

    wstring a = L"div class=\"apple\"  style=\"banana\"";

    split(a, L' ', res, true);
    for (auto p: res) {
        wcout << p << endl;
    }
    wcout << L"res" << endl;
    
    res.clear();
    split(a, L' ', res);
    for (auto p: res) {
        wcout << p << endl;
        split(p, {L'='}, res2, false, true);
    }

    wcout << L"res2" << endl;
    for (auto p: res2) {
        wcout << p << endl;
    }
}

inline void test_to_wstring() {
    string a = "中文测试";
    wcout << to_wstring(a) << endl;
}

inline void test_sub() {
    wstring s = L"apple < banana > chocolate";
    sub(L" <", L"<", s);
    wcout << s;
}

inline void test_strip() {
    wstring t = L"  sadf  ";
    wcout << t << "\n" << strip(t) << endl;
}

inline void test_regex() {
    auto bet = std::wregex(L"\\b\\s+\\b");
    auto bed = std::wregex(L"\\b\\s+\\.");
    auto w = std::wregex(L"\\s+");
    wcout << std::wstring(1, L'p') << endl;
    wcout << std::regex_replace(std::regex_replace(L".a.b > __text__ .c > apple banana", bet, L"&"), w, L"~") << endl;
}

int main() {
    init_chinese_env();
    T(test_to_wstring);
    T(test_split);
    T(test_sub);
    T(test_strip);
    T(test_regex);

    ParserShell ps;
    ps.Start();
}
