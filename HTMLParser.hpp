#include <iostream>
#include <string>
#include <istream>
#include <sstream>
#include <fstream>
#include <cstring>

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>

#include <cstdlib>
#include <cassert>
#include <functional>
#include <regex>
#include <memory>
#include "utils.hpp"

using std::string;     using std::wstring;
using std::cout;       using std::wcout;
using std::cin;        using std::wcin;
using std::wifstream;
using std::endl;

using std::vector;
using std::multimap;

using std::shared_ptr;
using std::weak_ptr;
using std::function;


#ifdef DEBUG
#define debug std::wcout
#else
std::wostringstream __null__;
#define debug __null__
#endif

#if defined(__clang__) || defined(__GNUC__)

    #define CPP_STANDARD __cplusplus

#elif defined(_MSC_VER)

    #define CPP_STANDARD _MSVC_LANG

#endif

#if CPP_STANDARD >= 201703L


#ifndef __HTML_PARSER_HPP__
#define __HTML_PARSER_HPP__


class HTMLElement: public std::enable_shared_from_this<HTMLElement> {

private:  // fields

    wstring _tag;
    bool _self_closed;
    multimap<wstring, wstring> _attributes;
    wstring _basic_text;
    int _depth;
    weak_ptr<HTMLElement> _father;
    vector<shared_ptr<HTMLElement> > _children;


public:  // utils

    friend class HTMLParser;  /* They want full access to elements information. */
    friend class ParserShell;  /* They want full access to elements information. */

    typedef shared_ptr<HTMLElement> PT;  // pointer type
    typedef function<bool (const HTMLElement&)> FT;  // filter type
    typedef function<bool (const PT)> AT;  // if return false: the signal of early_stopping
    typedef vector<PT> RT;  // return type


public:  // initialization

    // root node
    HTMLElement()
        : _father(), _depth(0), _tag(L"root")
    {}

    // normal node
    HTMLElement(shared_ptr<HTMLElement> father, bool splited, const wstring &raw_tag, const bool &self_closed)
        : _father(father), _self_closed(self_closed)
    {
        if (father) {
            this->_depth = father->_depth + 1;
        }
        if (splited) {
            this->_tag = raw_tag;
        } else {
            vector<wstring> temp;
            split(raw_tag, L' ', temp, true);
            this->_tag = temp[0];
            this->AddAttributes(temp[1]);
        }
    }

    // text node
    HTMLElement(const wstring &text, shared_ptr<HTMLElement> father)
        : _father(father), _tag(L"__text__"), _basic_text(text)
    {
        if (father) {
            this->_depth = father->_depth + 1;
        }
    }

    void AddText(const wstring &text) {
        wstring stripped_text = strip(text);
        if (stripped_text.size()) {
            debug << "ADD_TEXT: " << stripped_text << endl;
            this->_children.push_back(std::make_shared<HTMLElement>(stripped_text, this->shared_from_this()));
        }
    }

    void AddAttribute(const wstring &key, const wstring &value=L"") {
        this->_attributes.insert({key, value});
    }

    void AddAttributes(const wstring &attributes) {
        vector<wstring> splited;
        vector<wstring> pair;
        split(attributes, L' ', splited);
        for (auto p: splited) {
            pair.clear();
            split(p, L'=', pair, true);
            if (pair.size() == 2) {
                this->AddAttribute(pair[0], pair[1]);
            } else if (pair.size() == 1) {
                this->AddAttribute(pair[0]);
            } else {
                std::wcerr << "AddAttributesError: Failed to parse attribute " << p << "." << endl;
            }
        }
    }


public: // get information: basics

    wstring GetAttribute(const wstring &key) const {
        auto it = this->_attributes.find(key);
        return it != this->_attributes.end() ? it->second : L"";
    }

    wstring GetAttribute(const string &key) const {
        auto it = this->_attributes.find(to_wstring(key));
        return it != this->_attributes.end() ? it->second : L"";
    }

    template<typename T>
    wstring operator[](const T &key) const {
        return this->GetAttribute(key);
    }

    void GetElementsByFilter(const AT &adder, const FT &filter, bool recursive) const {
        for (auto ep: this->_children) {
            if (filter(*ep)) {
                if (!adder(ep)) {
                    return ;
                }
            }
            if (recursive) {
                ep->GetElementsByFilter(adder, filter, recursive);
            }
        }
    }

    void Traverse(const function<void (const HTMLElement&)> &lambda, bool recursive, bool first) const {
        for (const auto ep: this->_children) {
            lambda(*ep);
            if (recursive) {
                ep->Traverse(lambda, recursive, first);
            }
        }
    }


public:  // get information: advanced

    void GetElementsById(std::set<HTMLElement::PT> &results, const wstring &id, bool recursive, bool first) const {
        this->GetElementsByFilter(
            [&results, &first](const HTMLElement::PT pt) -> bool {
                results.insert(pt);
                return !first;
            },
            [&id](const HTMLElement &__ele) -> bool {
                return (__ele[L"id"] == id);
            },
            recursive);
    }

    void GetAllElements(std::set<HTMLElement::PT> &results, bool recursive = true) const {
        this->GetElementsByFilter(
            [&results](const HTMLElement::PT pt) -> bool {
                results.insert(pt);
                return true;
            },
            [](const HTMLElement &__ele) -> bool {
                return true;
            },
            recursive);
    }

    void GetElementsByFather(std::set<HTMLElement::PT> &results, const wstring &father, bool recursive, bool first) const {
        this->GetElementsByFilter(
            [&results, &first](const HTMLElement::PT pt) -> bool {
                results.insert(pt);
                return !first;
            },
            [&father](const HTMLElement &__ele) -> bool {
                return (__ele._father.lock()->GetTag() == father);
            },
            recursive);
    }

    void GetElementsByClass(std::set<HTMLElement::PT> &results, const wstring &cls, bool recursive, bool first) const {
        this->GetElementsByFilter(
            [&results, &first](const HTMLElement::PT pt) -> bool {
                results.insert(pt);
                return !first;
            },
            [&cls](const HTMLElement &__ele) -> bool {
                return (__ele[L"class"] == cls);
            },
            recursive);
    }

    void GetElementsByAttribute(std::set<HTMLElement::PT> &results, const wstring &key, bool recursive, bool first) const {
        if (key == L"empty") {
            this->GetElementsByFilter(
                [&results, &first](const HTMLElement::PT pt) -> bool {
                    results.insert(pt);
                    return !first;
                },
                [](const HTMLElement &__ele) -> bool {
                    return (__ele._children.size() == 0);
                },
                recursive);
        }
        
        vector<wstring> splitted = split(key, L'=');
        if (splitted.size() == 2) {
            this->GetElementsByFilter(
                [&results, &first](const HTMLElement::PT pt) -> bool {
                    results.insert(pt);
                    return !first;
                },
                [&splitted](const HTMLElement &__ele) -> bool {
                    return (__ele[splitted[0]] == splitted[1]);
                },
                recursive);

        } else {
            this->GetElementsByFilter(
                [&results, &first](const HTMLElement::PT pt) -> bool {
                    results.insert(pt);
                    return !first;
                },
                [&key](const HTMLElement &__ele) -> bool {
                    return (__ele._attributes.count(key) || (key == L"optional" && __ele._attributes.count(L"required") == 0));
                },
                recursive);
        }
    }

    void GetElementsByTag(std::set<HTMLElement::PT> &results, const wstring &tag, bool recursive, bool first) const {
        this->GetElementsByFilter(
            [&results, &first](const HTMLElement::PT pt) -> bool {
                results.insert(pt);
                return !first;
            },
            [&tag](const HTMLElement &__ele) -> bool {
                return __ele.GetTag() == tag;
            },
            recursive);
    }

    wstring GetTag() const {
        return this->_tag;
    }

    wstring GetInnerText(const wchar_t delimiter = L'\0') const {
        wstring inner_text;
        for (auto &child: this->_children) {
            if (child->_basic_text.size()) {
                inner_text += child->_basic_text + L" ";
            }
        }
        if (inner_text.size()) {
            inner_text.pop_back();
            if (delimiter != L'\0') {
                inner_text += delimiter;
            }
        }
        return inner_text;
    }

    wstring GetAllText() const {
        wstring all_text;
        this->Traverse([&all_text](const HTMLElement& __ele) {
            if (__ele._basic_text.size()) {
                all_text += __ele._basic_text + L"\n";
            }
        }, true, false);
        all_text.pop_back();
        return all_text;
    }

    wstring GetOuterHTML(bool attributes = true) const {
        wstring all_attributes = L" ";
        if (attributes && this->_attributes.size()) {
            for (const auto &attr: this->_attributes) {
                if (attr.second.size()) {
                    all_attributes += attr.first + L"=" + attr.second + L" ";
                } else {
                    all_attributes += attr.first + L" ";
                }
            }
        }
        all_attributes.pop_back();
        if (this->_basic_text.size()) {
            debug << "__text__" << endl;
            return this->_basic_text;
        }
        if (!this->_self_closed) {
            return L"<" + this->_tag + all_attributes + L">" + this->GetInnerText() + L"</" + this->_tag + L">";
        } else {
            return L"<" + this->_tag + all_attributes + L"/>";
        }
    }

    wstring GetInnerHTML(bool recursive) const {
        wstring all_html;
        for (const auto &child: this->_children) {
            if (child->_basic_text.size()) {
                all_html += child->_basic_text;
            } else {
                all_html += child->GetAllHTML(true);
            }
        }
        debug << "GetInnerHTML: " << all_html << endl;
        return all_html;
    }

    wstring GetAllHTML(bool attributes = true) const {
        wstring all_attributes = L" ";
        if (attributes && this->_attributes.size()) {
            for (const auto &attr: this->_attributes) {
                if (attr.second.size()) {
                    all_attributes += attr.first + L"=" + attr.second + L" ";
                } else {
                    all_attributes += attr.first + L" ";
                }
            }
        }
        all_attributes.pop_back();
        debug << this->GetTag() << endl;
        if (this->_basic_text.size()) {
            return this->_basic_text;
        }
        if (!this->_self_closed) {
            return L"<" + this->_tag + all_attributes + L">" + this->GetInnerHTML(true) + L"</" + this->_tag + L">";
        } else {
            return L"<" + this->_tag + all_attributes + L"/>";
        }
    }

};


template<typename C>
inline void traverse(const C &rt, const function<void (HTMLElement &)> lambda) {
    for (const HTMLElement::PT &ep: rt) {
        lambda(*ep);
    }
}


template<typename T>
using stack = std::vector<T>;
#define push push_back
#define top back
#define pop pop_back


class HTMLParser {

private:  // fields

    stack<wchar_t> _char_stack;
    stack<HTMLElement::PT> _ele_stack;
    HTMLElement::PT _father;
    HTMLElement::PT _root;
    bool _parsed = false;

    // the set of self-closed elements
    static inline std::unordered_set<wstring> _SELF_CLOSED = {L"link", L"meta", L"img", L"input", L"textarea",\
        L"source", L"br", L"hr", L"base", L"area", L"svg"};

    // the set of elements to be deleted
    static inline std::unordered_set<wstring> _FILTER = {L"script", L"meta", L"link", L"img", L"input", L"svg",\
        L"style"};

    // the set of delimiter of class
    static inline wstring DELIMITER = L" =";


public:  // initialization

    HTMLParser()
        : _root(std::make_shared<HTMLElement>())
    {
        this->_ele_stack.push(this->_root);
    }

    ~HTMLParser() = default;  // explicitly enable default destructor


private:  // helper functions

    // whether pop_an element should be deleted
    static bool _is_ignore(const wstring& __s) {
        return !__s.length() || _FILTER.count(__s) || !wild_compare(__s.substr(0, 4), wstring(L"meta")) || __s.front() == L'!';
    }

    // helper function: get the type of current tag
    // Example:
    //    Input: <div class="x"/>
    //    Output: div
    static wstring _get_tag_type(const wstring& _raw) {
        if (!_raw.length()) {
            return _raw;
        }
        size_t _pos = 0;
        size_t _begin_slash = _raw.front() == L'/';
        size_t _end_slash = _raw.back() == L'/';
        for (wchar_t del: DELIMITER) {
            if ((_pos = _raw.find(del)) != -1) {
                if (_end_slash && _pos == _raw.length()) {
                    _pos--;
                }
                return _raw.substr(_begin_slash, _pos-_begin_slash);
            }
        }
        return _raw.substr(_begin_slash, _raw.length()-_begin_slash-_end_slash);
    }


private:  // char parsing

    // pop text from _char_stack (using '<' as seperator)
    // this function will not pop the seperator
    wstring _pop_text() {
        wchar_t _char_buf, last = -1;
        wstring _tag_buf = L"";
        while (this->_char_stack.size() && (_char_buf = this->_char_stack.top()) != '<') {
            if (true || last != ' ' || last != _char_buf) {
                _tag_buf += _char_buf;
            }
            last = _char_buf;
            this->_char_stack.pop();
        }
        return wstring(_tag_buf.rbegin(), _tag_buf.rend());
    }


private:  // element parsing

    void _new_ele(const wstring &tag_raw, const bool &self_closed) {
        auto fa = this->_ele_stack.top();
        auto ep = std::make_shared<HTMLElement>(fa, false, tag_raw, self_closed);
        if (!self_closed) {
            this->_ele_stack.push(ep);
        }
        fa->_children.push_back(ep);
    }

    void _close_ele(const wstring &tag_type, const wstring & tag_text) {
        // assert: size of element stack
        if (_ele_stack.size() <= 1) {
            std::wcerr << L"\033[1;31mExtra element \"" << tag_type << L"\" found!\033[0;m" << endl;
            exit(1);
        }

        // assert: pop pairing element
        const wstring stack_raw = _ele_stack.top()->GetTag();
        const wstring stack_class = _get_tag_type(stack_raw);
        if (wild_compare(stack_class, tag_type)) {
            std::wcerr << L"\033[1;31mClosing element \"" << tag_type
                 << L"\" doesn't match a starting element \"" << stack_class << "\"." << endl;
            std::wcerr << "<" << stack_class << ">" << tag_text << L"</" << tag_type << L">\033[0;m" << endl;
            exit(1);
        }

        // add text
        _ele_stack.top()->AddText(tag_text);
        _ele_stack.pop();
    }

    // Pop the span of element from top of _char_stack. We done this by first poping the raw tag of element
    // and then retrieving the text if needed.
    //
    // Notes:
    //    There are three types of tags in HTML:
    //    1. self-closed element like <br/> or <br>
    //    2. starting element like <h1>
    //    3. closing element like </h1>
    //
    //    We subdivide a fourth type from the self-closed: script-like element.
    //    That is those without any natrual language text.
    int _pop_element(bool &script, bool &exclamation) {



        // get raw tag and its type
        wstring tag_raw = _pop_text();
        wstring tag_type = _get_tag_type(tag_raw);

        // parse completed
        if (tag_raw.length() == 0) {
            this->_char_stack.pop();
            return 0;
        }

        // assert tag length
        if (tag_raw.length() < 1 || !wild_compare(tag_raw, wstring(L"/"))) {
            std::wcerr << "\033[1;31mTag length " << tag_raw.length() << " error.\033[0m" << endl;
            return 1;
        }

        // Automata
        enum ElementClosingType {
            CLOSING,
            STARTING,
            SELF_CLOSING,
        } status;

        // determine tag closing status e.g. <br/> or <h1></h1>
        if (!this->_SELF_CLOSED.count(tag_type)) {
            status = tag_raw.front() != L'/' ? STARTING : CLOSING;
        } else {
            status = SELF_CLOSING;
        }
        // wcout << "=== " << tag_raw << " ++ " << tag_type << "{status=" << status << "reserve=" << !this->_is_ignore(tag_type) << "}"<< " ===" << endl;
        if (tag_type == L"script") {
            script = (status == STARTING);
        }

        switch (status) {

            case STARTING: {
                // script logs the overall states and leave '<' in _char_stack as marker
                if (!script && !exclamation && !this->_FILTER.count(tag_type)) {
                    this->_new_ele(tag_raw, false);
                    debug << "\n## new ##\n" << endl;
                }
                safe_pop(this->_char_stack);
                break;
            };

            case CLOSING: {
                // get text inside
                safe_pop(this->_char_stack);
                wstring tag_text = this->_pop_text();

                // close
                bool added = false;
                if (!script && !exclamation && !this->_is_ignore(tag_type)) {
                    this->_close_ele(tag_type, tag_text);
                    added = true;
                    debug << "\n## close ##\n" << endl;
                }
                // safe_pop(this->_char_stack);

                // get text remained
                tag_text = this->_pop_text();
                if (added && tag_text.size()) {
                    _ele_stack.top()->AddText(tag_text);
                }
                break;
            };

            case SELF_CLOSING: {
                if (!this->_FILTER.count(tag_type)) {
                    this->_new_ele(tag_raw, true);
                    debug << "\n## self-closed ##\n" << endl;
                }
                break;
            }

        }
        /*wcout << "\n+++++" << script << exclamation;
        for (auto p: this->_ele_stack) {
            wcout << p->GetTag() << " ";
        }
        wcout << endl;*/
        return 0;
    }

public:  // information: basics

    HTMLElement::PT GetRoot() const {
        return this->_root;
    }

    void output() const {
        int idx = 0;
        this->_root->Traverse(
            [&idx](const HTMLElement &ptr) -> void {  // using a lambda function
                if (ptr._basic_text.size()) {
                    return ;
                }
                std::wcout << idx++;
                wstring temp = L": ";
                if (ptr._depth >= 0) {
                    temp += wstring(ptr._depth, L' ');
                }
                std::wcout << temp << ptr.GetTag() << std::endl;
            }, true, false);
    }

    // return whether loaded something
    bool loaded() const {
        return this->_parsed;
    }

    // parse from a wistream
    int parse(std::wistream & ifs) {
        wcout << "Parsing Started!" << endl;
        this->_father = this->_root;
        unsigned int reading = 0;
        bool exclamation = 0;
        bool script = 0;
        wstring _line_buf;

        while (ifs >> _line_buf) {

            _line_buf += L' ';  // inputing a string from stream needs whitespaces
            for (auto _char_buf: _line_buf) {
                debug << _char_buf;// << exclamation << script;
                // html has <br/> but doesn't have '\n'
                if (_char_buf == L'\n' || _char_buf == L'\t') {
                    _char_buf = L' ';
                }

                // push at char level and pop at element level
                // parsing stack: <
                if (_char_buf == L'>') {
                    if (!reading) {
                        wcout << "\033[1;031mEncounter HTML syntax error when parsing the html.\033[0m" << endl;
                        return 1;
                    }
                    reading--;
                    if (!reading) {
                        this->_pop_element(script, exclamation);
                        exclamation = 0;
                    } else if (!exclamation){
                        cout << "\033[1;033mWarning: Nested element detected!\033[0m" << endl;
                    }

                } else if (_char_buf == L'<') {
                    if (!reading) {
                        this->_char_stack.push(_char_buf);
                    } else if (!exclamation){
                        cout << "\033[1;033mWarning: Nested element detected!\033[0m" << endl;
                    }
                    reading++;

                } else if (_char_buf == L'!'){
                    exclamation |= reading;

                } else {
                    if (reading <= 1 && !exclamation) {
                        this->_char_stack.push(_char_buf);
                    }
                }
            }
        }
        this->_parsed = this->_root->_children.size();
        if (this->_parsed) {
            cout << "Parsing Completed!" << endl;
        } else {
            cout << "Nothing found in root node!" << endl;
        }
        return !this->_parsed;
    }
};


class ParserShell {

private:

    HTMLParser _parser;
    std::set<HTMLElement::PT> _selected = std::set<HTMLElement::PT> { _parser.GetRoot() };
    std::set<HTMLElement::PT> _last;
    std::string _source;
    wifstream _ifs;

    static inline string HELP = \
        "HTMLParser Shell  designed by huyiwen\n"\
        "  clear        clear selected element and go back to root\n"\
        "  css expr     select css  e.g. css #firstname\n"\
        "  get type     get information type = [text,all_text,html,all_html,href]\n"\
        "  reload       reload html\n"\
        "  help         show this information\n"\
        "  exit\n"\
        ;  // ends here

    template<typename T>
    static wstring GetPrompt(const T &sele) {
        if (sele.size() >= 1){
            wstring ret;
            int cnt = 0;
            for (const auto &p: sele) {
                if (p->_basic_text.size()) {
                    continue;
                }
                ret += p->GetTag() + L", ";
                if (++cnt > 2) {
                    break;
                }
            }
            ret.pop_back();
            ret.pop_back();
            if (cnt == 3) {
                ret += L", ...";
            }
            ret += L" (" + std::to_wstring(sele.size()) + L")";
            return ret;
        } else {
            return L"empty";
        }
    }

    void parse_from_source() {
        // user input
        if (_source.length() == 0) {
            cout << "\033[1;033mURL/Path to webpage: \033[0m";
            cout.flush();
            std::getline(std::cin, _source);
        }

        // default value
        if (_source.length() == 0) {
            _source = "https://www.baidu.com";
        }

        // curl from internet > read from local
        if (_source.substr(0, 4) == "http") {
            string _cmd = "curl " + _source + " > output.html 2> /dev/null";
            std::system(_cmd.c_str());
            // std::system("cat output.html");
            _ifs.open("output.html", std::ios::in);
            cout << "Crawling: " << _source << endl;

        } else {
            _ifs.open(_source, std::ios::in);
            cout << "Opening: " << _source << endl;
        }

        int exit_code = _parser.parse(_ifs);
        if (exit_code || !_parser.loaded()) {
            this->_parser = HTMLParser();
            this->_source.clear();
            wcout << "\033[1;31mParsing failed with exit code " << exit_code << "!\033[0m" << endl;
        } else {
            wcout << L"Parser results:" << endl;
            this->_parser.output();
        }
    }

    void _filter_current(const wstring &path, bool append, bool attr) {
        std::set<HTMLElement::PT> ret;
        std::set<HTMLElement::PT> &sel = append ? this->_last : this->_selected;
        if (append) {
            ret = this->_selected;
        }

        if (attr) {
            if (path == L"empty") {
                for (const auto &ep: sel) {
                    if (ep->_children.size() == 0) {
                        ret.insert(ep);
                    }
                }
            } else {
                for (const auto &ep: sel) {
                    if (ep->_attributes.count(path)) {
                        ret.insert(ep);
                    }
                }
            }

        } else if (path == L"*") {
            ret = this->_selected;

        } else if (path[0] == L'.') {
            for (const auto &ep: sel) {
                if (ep->GetAttribute(L"class") == path.substr(1)) {
                    ret.insert(ep);
                }
            }

        } else if (path[0] == L'#') {
            for (const auto &ep: sel) {
                if (ep->GetAttribute(L"id") == path.substr(1)) {
                    ret.insert(ep);
                }
            }

        } else {
            for (const auto &ep: sel) {
                if (ep->GetTag() == path) {
                    ret.insert(ep);
                }
            }
        }

        this->_last = _selected;
        this->_selected = ret;
        return ;
    }

    void _get_next(const wstring &path, bool recursive, bool append, bool first, bool attr, bool current) {
        if (current) {
            this->_filter_current(path, append, attr);
            return ;
        }
        std::set<HTMLElement::PT> ret;
        std::set<HTMLElement::PT> &sel = append ? this->_last : this->_selected;
        if (append) {
            ret = this->_selected;
        }

        if (attr) {
            for (const auto &ep: sel) {
                ep->GetElementsByAttribute(ret, path, recursive, first);
            }

        } else if (path == L"*") {
            for (const auto &ep: sel) {
                ep->GetAllElements(ret);
            }

        } else if (path[0] == L'.') {
            for (const auto &ep: sel) {
                ep->GetElementsByClass(ret, path.substr(1), recursive, first);
            }

        } else if (path[0] == L'#') {
            for (const auto &ep: sel) {
                ep->GetElementsById(ret, path.substr(1), recursive, first);
            }

        } else {
            for (const auto &ep: sel) {
                ep->GetElementsByTag(ret, path, recursive, first);
            }
        }

        this->_last = _selected;
        this->_selected = ret;
        return ;
    }

private:

    std::wregex _between = std::wregex(L"\\b\\s+\\b");
    std::wregex _between_dot = std::wregex(L"\\b\\s+\\.");
    std::wregex _dot_between = std::wregex(L"\\b\\.\\b");
    std::wregex _sharp_between = std::wregex(L"\\b#\\b");
    std::wregex _bracket = std::wregex(L"\\]");
    std::wregex _whitespaces = std::wregex(L"\\s+");

    void _get_elements(const wstring &expr) {
        wstring trimmed_expr = std::regex_replace(expr, this->_between, L"&");  // "word word" -> "word&word"
        debug << L"trimmed_expr: " << trimmed_expr << endl;

        trimmed_expr = std::regex_replace(trimmed_expr, this->_between_dot, L"&.");  // "word .word" -> "word&.word"
        debug << L"trimmed_expr: " << trimmed_expr << endl;

        trimmed_expr = std::regex_replace(trimmed_expr, this->_whitespaces, L"");  // " " -> ""
        debug << L"trimmed_expr: " << trimmed_expr << endl;

        trimmed_expr = std::regex_replace(trimmed_expr, this->_dot_between, L"$.");  // "word.word" -> "word$.word"
        debug << L"trimmed_expr: " << trimmed_expr << endl;

        trimmed_expr = std::regex_replace(trimmed_expr, this->_sharp_between, L"$#");  // "word#word" -> "word$#word"
        debug << L"trimmed_expr: " << trimmed_expr << endl;

        trimmed_expr = std::regex_replace(trimmed_expr, this->_bracket, L"$");  // "[word]" -> "[word$"
        debug << L"trimmed_expr: " << trimmed_expr << endl;

        vector<wstring> path = split(trimmed_expr, {L'&', L'>', L',', L'+', L'~', L'$', L':', L'['}, false, true);
        unsigned int state = 0;
        bool continuous = false;
        // 0 select by ancestors
        // 1 among the direct son(s)
        // 2 append the last one(s)
        // 3 first element only
        // 4 find by attributes

        for (const auto &ele: path) {
            debug << ele << L' ' << std::endl;
            if (ele == L"$") {
                continue;
            }
            bool last = continuous;
            if (ele == L"&" || ele == L"~") {  // self-defined, used to signal recursively find by tag
                state = 0b0;
                continuous = false;
            } else if (ele == L">") {
                state = 0b1;
                continuous = false;
            } else if (ele == L",") {  // element-wise or: just append the last one
                state |= 0b10;
                continuous = false;
            } else if (ele == L"+") {
                state = 0b100;
                continuous = false;
            } else if (ele == L":" || ele == L"[") {
                state = 0b1000;
                // continuous = true;
            } else if (ele == L":root") {
                this->_last = this->_selected;
                this->_selected = { _parser.GetRoot() };
            } else {
                continuous = true;
                debug << last << continuous << endl;
                this->_get_next(ele, ~state & 0b1, state & 0b10, state & 0b100, state & 0b1000, !(last ^ continuous));
            }
        }
    }

    void _get_info(const string &type) {
        if (type == "text") {
            traverse(this->_selected,
                    [](const HTMLElement& __ele) {
                        wcout << __ele.GetInnerText(L'\n');
                    });
            wcout << endl;

        } else if (type == "all_text") {
            int idx = 0;
            traverse(this->_selected,
                    [&idx](const HTMLElement& __ele) {
                        wcout << idx++ << L": " + __ele.GetAllText() << endl;
                    });

        } else if (type == "html") {
            traverse(this->_selected,
                    [](const HTMLElement& __ele) {
                        wcout << __ele.GetOuterHTML() << endl;
                    });

        } else if (type == "all_html") {
            traverse(this->_selected,
                    [](const HTMLElement& __ele) {
                        wcout << __ele.GetAllHTML() << endl;
                    });

        } else if (type == "href") {
            traverse(this->_selected,
                    [](const HTMLElement& __ele) {
                        wstring tmp = __ele[L"href"];
                        if (tmp.size()) {
                            wcout << tmp << endl;
                        }
                    });
        } else {
            cout << "Not supported type " << type << endl;
        }
    }

    void _reload(const string &source = "") {
        if (source.size()) {
            this->_source = source;
        } else {
            this->_source.clear();
        }
        this->_ifs = wifstream();
        this->_last.clear();
        this->_selected = {this->_parser.GetRoot()};
        this->_parser = HTMLParser();
        this->parse_from_source();
    }


public:

    void SetSource(const string &src) {
        this->_source = src;
    }


public:

    void Start() {
        while (1) {
            if (!_parser.loaded()) {
                this->parse_from_source();
                cout << '\n' << this->HELP << endl;
                continue;
            }

            wcout << L"\033[1;033m[" << GetPrompt(_selected) << L"] \033[0m";
            wcout.flush();
            string ln;
            std::getline(std::cin, ln);

            std::vector<string> untied;
            split(ln, ' ', untied, true);

            // wcout << this->_parser.GetRoot()->_children[0]->_children[1]->GetAttribute(L"link") << endl;

            if (untied.size() < 1) {
                continue;
            }

            if (untied[0] == "help") {
                cout << this->HELP << endl;

            } else if (untied[0] == "clear") {
                this->_last = this->_selected;
                this->_selected = { _parser.GetRoot() };

            } else if (untied[0] == "css") {
                this->_get_elements(to_wstring(untied[1]));

            } else if (untied[0] == "get") {
                this->_get_info(untied[1]);

            } else if (untied[0] == "reload") {
                this->_reload(untied.size() > 1 ? untied[1] : "");

            } else if (untied[0] == "exit") {
                return ;

            } else {
                cout << "Receive unkown command.\n" << HELP << endl;
            }
        }
    }
};

#endif // __HTML_PARSER_HPP__
#endif // CPP_STANDARD

