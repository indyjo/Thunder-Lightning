#ifndef SCRIPTING_TOKENIZER_H
#define SCRIPTING_TOKENIZER_H

#include <stack>
#include <string>

namespace Scripting {

    class Tokenizer {
        int cursor_begin, cursor_end;
        std::stack<int> saved_cursors;

        std::string input;
    public:
        inline Tokenizer(const std::string & inp)
        :   cursor_begin(0),
            cursor_end(0),
            input(inp)
        { }

        bool nextToken();

        inline std::string getToken() const
        { return input.substr(cursor_begin, cursor_end-cursor_begin); }

        bool        isString();
        bool        isInteger();
        bool        isFloat();
        bool        isIdentifier();
        bool        isPath();

        inline bool is(const std::string & s) const { return getToken() == s; }
        inline bool beginsWith(const std::string & s) const {
            if (cursor_end-cursor_begin < s.size())
                return false;
            return s == input.substr(cursor_begin, s.size());
        }

        inline bool endsWith(const std::string & s) const {
            if (cursor_end-cursor_begin < s.size())
                return false;
            return s == input.substr(cursor_end-s.size(), s.size());
        }

        inline bool contains(const std::string & s) const {
            return getToken().find(s) != std::string::npos;
        }

        std::string getString();
        int         getInteger();
        float       getFloat();
        inline std::string getIdentifier() { return getToken(); }
        inline std::string getPath() { return getToken(); }

        inline void push() {
            saved_cursors.push(cursor_begin);
            saved_cursors.push(cursor_end);
        }
        inline void pop() {
            cursor_end = saved_cursors.top();
            saved_cursors.pop();
            cursor_begin = saved_cursors.top();
            saved_cursors.pop();
        }
        inline void forget() {
            saved_cursors.pop();
            saved_cursors.pop();
        }

    };

    struct TokenizerGuard {
        inline TokenizerGuard(Tokenizer & t)
        : tokenizer(t), restore(true)
        { t.push(); }
        
        inline ~TokenizerGuard() {
            if (restore) 
                tokenizer.pop();
            else
                tokenizer.forget();
        }
        inline void forget() { restore = false; }
    private:
        bool restore;
        Tokenizer & tokenizer;
    };
} // namespace Scripting

#endif
