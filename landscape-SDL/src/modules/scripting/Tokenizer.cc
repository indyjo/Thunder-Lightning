#include <string>
#include <cctype>

#include "Tokenizer.h"

using namespace std;

namespace Scripting {
    bool Tokenizer::nextToken() {
        int n = input.length();

        cursor_begin = cursor_end;

        // Skip whitespace at beginning
        while (cursor_begin != n && isspace(input[cursor_begin]))
            ++cursor_begin;
        if (cursor_begin == n) return false;

        cursor_end = cursor_begin;

        // Test for quotes
        bool quoted;
        if ('"' == input[cursor_end]) {
            quoted = true;
            ++cursor_end;
        } else {
            quoted = false;
        }

        if (quoted) {
            char c;
            while (cursor_end != n && (c = input[cursor_end]) != '"') {
                ++cursor_end;
                if (c == '\\') {
                    if (cursor_end == n) return true;
                    ++cursor_end;
                }
            }
            if (cursor_end != n) ++cursor_end;
        } else { // not quoted
            while (cursor_end != n && ! isspace(input[cursor_end]))
                ++cursor_end;
        }
        return true;
    }

    bool Tokenizer::isString() {
        if (input[cursor_begin] != '\"')
            return false;
        if (input[cursor_end-1] != '\"')
            return false;
        for(int i=cursor_begin+1; i<cursor_end-1; i++) {
            if (input[i]!='\\')
                continue;
            i++;
            if (i==cursor_end-1)
                return false;
            switch(input[i]) {
            case '\\':
            case 'a':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            case 'v':
            case '"': break;
            case 'x':
                if (i+2 >= cursor_end-1)
                    return false;
                if (!isxdigit(input[i+1]))
                    return false;
                if (!isxdigit(input[i+2]))
                    return false;
                i += 2;
                break;
            default: return false;
            }
        }
        return true;
    }

    bool Tokenizer::isInteger() {
        int digits_begin = cursor_begin;
        if (input[digits_begin] == '-' || input[digits_begin] == '+')
            ++digits_begin;
        for(int i=digits_begin; i!=cursor_end; ++i) {
            if (!isdigit(input[i]))
                return false;
        }
        return true;
    }

    bool Tokenizer::isFloat() {
        int digits_begin = cursor_begin;
        bool had_dot = false;
        bool had_digit = false;
        bool had_exp = false;
        if (input[digits_begin] == '-' || input[digits_begin] == '+')
            ++digits_begin;
        for(int i=digits_begin; i!=cursor_end; ++i) {
            if (isdigit(input[i])) {
                had_digit = true;
            } else if (input[i]=='.') {
                had_dot = true;
            } else if (input[i]=='e') {
                if (!had_digit) return false;
                had_exp = true;
                digits_begin = i+1;
                if (i+1 == cursor_end) return false;
                break;
            } else {
                return false;
            }
        }
        if (had_exp) {
            had_digit = false;
            if (input[digits_begin] == '-' || input[digits_begin] == '+')
                ++digits_begin;
            for(int i=digits_begin; i!=cursor_end; ++i) {
                if (isdigit(input[i])) {
                    had_digit = true;
                } else return false;
            }
        }
        return had_digit;
    }

    bool Tokenizer::isIdentifier() {
        if (cursor_begin == cursor_end)
            return false;
        if (!isalpha(input[cursor_begin]) && input[cursor_begin] != '_')
            return false;
        for(int i=cursor_begin+1; i!=cursor_end; i++) {
            if (!isalnum(input[i]) && input[i] != '_')
                return false;
        }
        return true;
    }

    bool Tokenizer::isPath() {
        int i=cursor_begin;
        if (input[i]=='/') i++;
        for( ; i != cursor_end; ++i) {
            char c = input[i];
            if (c == '/') continue;
            else if (isalnum(c)) continue;
            else if (c == '_' || c == '-') continue;
            else if (c == '.') {
                for( ++i ; i!= cursor_end; ++i) {
                    if      (input[i] == '.') ; // do nothing
                    else if (input[i] == '/') break;
                    else return false;
                }
                if (i == cursor_end) return true;
            } else return false;
        }
        return true;
    }

    string Tokenizer::getString() {
        std::string result;
        for(int i=cursor_begin+1; i<cursor_end-1; i++) {
            if (input[i]!='\\') {
                result += input[i];
                continue;
            }
            i++;
            switch(input[i]) {
            case '\\':  result += '\\'; break;
            case 'a':   result += '\a'; break;
            case 'b':   result += '\b'; break;
            case 'f':   result += '\f'; break;
            case 'n':   result += '\n'; break;
            case 'r':   result += '\r'; break;
            case 't':   result += '\t'; break;
            case 'v':   result += '\v'; break;
            case '"':   result += '\"'; break;
            case 'x':
                char c = input[i+1];
                int fst_digit = isdigit(c)?c-'0':
                                isupper(c)?c-'A':
                                c-'a';
                c = input[i+2];
                int snd_digit = isdigit(c)?c-'0':
                                isupper(c)?c-'A':
                                c-'a';
                i += 2;
                result += (char)(fst_digit<<4 + snd_digit);
                break;
            }
        }
        return result;
    }

    int Tokenizer::getInteger() {
        return atoi(input.substr(cursor_begin, cursor_end-cursor_begin).c_str());
    }

    float Tokenizer::getFloat() {
        return atof(input.substr(cursor_begin, cursor_end-cursor_begin).c_str());
    }
} // namespace Scripting

// int main() {
//     string input = "Test 1234 123.23e+3 .0 .1 0. 1.2 0.-1 -3 0 \"string\" \"string too\" "
//                    "_ident __ident iDentifier09 xy_09_ab _ 9xy . _0 "
//                    "/ ./ // ./../ ../xy _X/..0/ ..01/ ././././x/y/z //z/.///c/. "
//                    "\"string with \\\"quotes\\\" inside\" \"string at end";
//     cout << input << endl;
//     Scripting::Tokenizer tokenizer(input);
//     while (tokenizer.nextToken()) {
//         cout << tokenizer.getToken();
//         if      (tokenizer.isString())     cout << " is string: "  << tokenizer.getString();
//         else if (tokenizer.isInteger())    cout << " is integer: " << tokenizer.getInteger();
//         else if (tokenizer.isFloat())      cout << " is float: "   << tokenizer.getFloat();
//         else if (tokenizer.isIdentifier()) cout << " is identifier: "
//             << tokenizer.getIdentifier();
//         else if (tokenizer.isPath())       cout << " is path: "    << tokenizer.getPath();
//         else cout << " is unknown token type.";
//         cout << endl;
//     }
//     return 0;
// }
