#include "Class.h"
#include "Tuple.h"
#include "CommandLine.h"
#include "Matcher.h"
#include <iostream>

using namespace std;

namespace {
    using namespace Scripting;
    string usage() {
        return "Usage:\n"
               "cd <path>\n"
               "  to change the current object\n"
               "ls\n"
               "  to list the contents of the current object\n"
               "ls <path>\n"
               "  to list the contents of the object at <path>\n"
               "funcs\n"
               "  to list the functions of the current object\n"
               "funcs <path>\n"
               "  to list the functions of the object at <path>\n"
               "vars\n"
               "  to list the current variables\n"
               "<path>\n"
               "  to print an object\n"
               "<var> = <literal>\n"
               "  to assign a variable a new value\n"
               "<var> = <path>\n"
               "  to assign a variable an object\n"
               "<var> = <path> <funcname> [<arg> [<arg> [...]]]\n"
               "  to call a function and assign a variable the return value\n"
               "<path> <funcname> [<arg> [<arg> [...]]]\n"
               "  to call a function\n";
    }

    string ls(Ptr<ValueBase> current_object) {
        string res;

        Func * dir_func = current_object->getType()->getFunc(
            "dir",
            make_spec<Ptr<Tuple> > ());
        if (!dir_func) {
            res += "<no children>\n";
            return res;
        }

        vector<Ptr<ValueBase> > params;
        Ptr<Tuple> tuple = get<Ptr<Tuple> >(dir_func->call(current_object, params));

        for (int i=0; i<tuple->size(); i++) {
            res += get<string>(tuple->get(i));
            res += "\n";
        }

        return res;
    }

    string la(Ptr<ValueBase> current_object) {
        string res;

        Func * dir_func = current_object->getType()->getFunc(
            "dir",
            make_spec<Ptr<Tuple> > ());
        Func * child_func = current_object->getType()->getFunc(
            "child",
            make_spec<Any, string>());
        if (!dir_func || !child_func) {
            res += "<no children>\n";
            return res;
        }

        vector<Ptr<ValueBase> > params;
        Ptr<Tuple> tuple = get<Ptr<Tuple> >(dir_func->call(current_object, params));
        params.resize(1);

        for (int i=0; i<tuple->size(); i++) {
            params[0] = tuple->get(i);
            
            res += get<string>(params[0]);
            res += " : ";

            Ptr<ValueBase> entry = child_func->call(current_object,
                                                    params);
            res += entry->getType()->getName();

            res += "\n";
        }

        return res;
    }

    string funcs(Ptr<ValueBase> current_object) {
        string res;
        const vector<Func*> & funcs = current_object->getType()->funcs;
        if (funcs.empty()) res += "<no functions>";
        for(int i=0; i<funcs.size(); ++i) {
            res += funcs[i]->desc.func_name;

            const vector<const Class*> & param_types = funcs[i]->spec.param_types;
            res += "(";
            for(int j=0; j<param_types.size(); j++) {
                res+=param_types[j]->getName();
                res+=(j==param_types.size()-1)?"":", ";
            }

            res += ") : ";
            res += funcs[i]->spec.return_type->getName();
            res += "\n";
        }
        return res;
    }

    string info(Ptr<ValueBase> obj) {
        string res = "Object of type "+obj->getType()->getName()+"\n";
        Func * func = obj->getType()->getFunc("string", make_spec<string>());
        if (func) {
            res += "Value: ";
            vector<Ptr<ValueBase> > params;
            res += get<string>(func->call(obj, params));
        }
        return res;
    }

    bool expr(Ptr<Environment> & env,
              Tokenizer & tok,
              Ptr<ValueBase> & res);

    bool expr_literal(Ptr<Environment> & env,
                      Tokenizer & tok,
                      Ptr<ValueBase> & res)
    {
        TokenizerGuard guard(tok);

        if (!tok.nextToken()) return false;
        if (tok.isInteger()) {
            res = make_val(tok.getInteger());
        } else if (tok.isFloat()) {
            res = make_val(tok.getFloat());
        } else if (tok.isString()) {
            res = make_val(tok.getString());
        } else return false;

        guard.forget();
        return true;
    }

    bool expr_expr_func_exprs(Ptr<Environment> & env,
                              Tokenizer & tok,
                              Ptr<ValueBase> & res)
    {
        TokenizerGuard guard(tok);

        Ptr<ValueBase> base;
        if (!expr(env,tok,base)) return false;

        if (!tok.nextToken()) return false;
        if (!tok.isIdentifier()) return false;

        string funcname = tok.getIdentifier();

        guard.forget();

        vector<Ptr<ValueBase> > params;
        FuncSpec spec;
        spec.return_type = Traits<Any>::type;

        Ptr<ValueBase> param;
        while (expr(env,tok,param)) {
            params.push_back(param);
            spec.param_types.push_back(param->getType());
        }

        Func * func = base->getType()->getFunc(funcname,spec);
        if (!func) {
            string desc = "Could not find function for type " + base->getType()->getName() + ":\n";
            desc += "  "+funcname;
            for(int i=0; i<spec.param_types.size(); ++i)
                desc+=" "+spec.param_types[i]->getName();
            desc += " : " + spec.return_type->getName() + "\n";
            throw new Exception(desc.c_str());
        }

        res = func->call(base, params);
        return true;
    }

    bool expr_path(Ptr<Environment> & env,
                   Tokenizer & tok,
                   Ptr<ValueBase> & res)
    {
        TokenizerGuard guard(tok);
        string path;
        if (!Matcher(tok)._path(path)) return false;
        guard.forget();
        res = env->resolve(path).first.back();
        return true;
    }

    bool expr_b_expr_b(Ptr<Environment> & env,
                       Tokenizer & tok,
                       Ptr<ValueBase> & res)
    {
        TokenizerGuard guard(tok);

        if (!tok.nextToken() || !tok.is("(")) return false;
        if (!expr(env,tok,res)) return false;
        if (!tok.nextToken() || !tok.is(")")) return false;

        guard.forget();
        return true;
    }

    bool expr(Ptr<Environment> & env,
              Tokenizer & tok,
              Ptr<ValueBase> & res)
    {
        TokenizerGuard guard(tok);
        if(   expr_expr_func_exprs(env,tok,res)
           || expr_path(env,tok,res)
           || expr_literal(env,tok,res)
           || expr_b_expr_b(env, tok, res))
        {
            guard.forget();
            return true;
        }
        return false;
    }

    bool handle_eval(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);
        if (!Matcher(tok)._("eval")) return false;
        Ptr<ValueBase> val;
        if (!expr(env,tok,val)) return false;

        guard.forget();
        res = info(val);
        return true;
    }

    bool handle_cd_path(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        string path;
        if (!Matcher(tok)._("cd")._path(path)._end()) return false;

        try {
            env->setCurrent(env->resolve(path));
        } catch (Exception * exception) {
            res = exception->what();
            res += "\nAn error occurred.\n";
        }

        return true;
    }

    bool handle_ls(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        if (!Matcher(tok)._("ls")._end()) return false;

        res = ls(env->current_stack.back());

        return true;
    }

    bool handle_ls_path(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        string path;
        if (!Matcher(tok)._("ls")._path(path)._end()) return false;

        try {
            StackPair new_stack = env->resolve(path);
            res = ls(new_stack.first.back());
        } catch (Exception * exception) {
            res = exception->what();
            res += "\nAn error occurred.\n";
        }

        return true;
    }

    bool handle_la(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        if (!Matcher(tok)._("la")._end()) return false;

        res = la(env->current_stack.back());

        return true;
    }

    bool handle_la_path(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        string path;
        if (!Matcher(tok)._("la")._path(path)._end()) return false;

        try {
            StackPair new_stack = env->resolve(path);
            res = la(new_stack.first.back());
        } catch (Exception * exception) {
            res = exception->what();
            res += "\nAn error occurred.\n";
        }

        return true;
    }

    bool handle_funcs(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        if (!Matcher(tok)._("funcs")._end()) return false;

        res = funcs(env->current_stack.back());

        return true;
    }

    bool handle_funcs_path(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        string path;
        if (!Matcher(tok)._("funcs")._path(path)._end()) return false;

        try {
            StackPair new_stack = env->resolve(path);
            res = funcs(new_stack.first.back());
        } catch (Exception * exception) {
            res = exception->what();
            res += "\nAn error occurred.\n";
        }

        return true;
    }

    bool handle_vars(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        if (!Matcher(tok)._("vars")._end()) return false;
        res = "";
        if (env->symbols.empty()) res += "<no vars>\n";
        for (SymbolMap::iterator i=env->symbols.begin(); i!=env->symbols.end(); ++i) {
            res += i->first;
            res += "\t";
            res += i->second->getType()->getName();
            res += "\n";
        }
        return true;
    }

    bool handle_path(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        string path;
        if (!Matcher(tok)._path(path)._end()) return false;

        try {
            StackPair new_stack = env->resolve(path);
            res = info(new_stack.first.back());
        } catch (exception * ex) {
            res = ex->what();
            res += "\nAn error occurred.\n";
        }
        return true;
    }

    bool handle_var_equals_literal(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        string var;
        if (!Matcher(tok)._identifier(var)._("=")) return false;

        Ptr<ValueBase> val;

        if (tok.isInteger()) {
            val = make_val(tok.getInteger());
        } else if (tok.isFloat()) {
            val = make_val(tok.getFloat());
        } else if (tok.isString()) {
            val = make_val(tok.getString());
        } else return false;

        if (tok.nextToken()) return false;

        env->symbols.insert(make_pair(var, val));
        return true;
    }

    bool handle_var_equals_path(Ptr<Environment> env, Tokenizer & tok, string & res) {
        TokenizerGuard guard(tok);

        string var, path;
        if (!Matcher(tok)._identifier(var)._("=")._path(path)._end()) return false;

        try {
            StackPair new_stack = env->resolve(path);
            env->symbols[var] = new_stack.first.back();
        } catch (exception * ex) {
            res = ex->what();
            res += "\nAn error occurred.\n";
        }

        return true;
    }

    bool handle_var_equals_path_funcname
        (Ptr<Environment> env, Tokenizer & tok, string & res)
    {
        TokenizerGuard guard(tok);

        string var, path, funcname;
        if (!Matcher(tok)._identifier(var)._("=")._path(path)._identifier(funcname)) return false;


        try {

            Ptr<ValueBase> current_object =
                env->resolve(path).first.back();
            vector<Ptr<ValueBase> > params;
            FuncSpec spec;
            spec.return_type = Traits<Any>::type;
            while (tok.nextToken()) {
                if (tok.isInteger()) {
                    params.push_back(make_val(tok.getInteger()));
                } else if (tok.isFloat()) {
                    params.push_back(make_val(tok.getFloat()));
                } else if (tok.isString()) {
                    params.push_back(make_val(tok.getString()));
                } else if (tok.isPath()) {
                    params.push_back(env->resolve(tok.getPath()).first.back());
                }
                spec.param_types.push_back(params.back()->getType());
            }

            Func * func = current_object->getType()->getFunc(funcname, spec);
            if (!func) {
                res = "No matching function.";
                return true;
            }
            Ptr<ValueBase> result = func->call(current_object, params);
            env->symbols[var] = result;
            res = info(result);
        } catch (exception * ex) {
            res = ex->what();
            res += "\nAn error occurred.\n";
        }

        return true;
    }

    bool handle_path_funcname
        (Ptr<Environment> env, Tokenizer & tok, string & res)
    {
        TokenizerGuard guard(tok);

        string path, funcname;
        if (!Matcher(tok)._path(path)._identifier(funcname)) return false;

        try {

            Ptr<ValueBase> current_object =
                env->resolve(path).first.back();
            vector<Ptr<ValueBase> > params;
            FuncSpec spec;
            spec.return_type = Traits<Any>::type;
            while (tok.nextToken()) {
                if (tok.isInteger()) {
                    params.push_back(make_val(tok.getInteger()));
                } else if (tok.isFloat()) {
                    params.push_back(make_val(tok.getFloat()));
                } else if (tok.isString()) {
                    params.push_back(make_val(tok.getString()));
                } else if (tok.isPath()) {
                    params.push_back(env->resolve(tok.getPath()).first.back());
                }
                spec.param_types.push_back(params.back()->getType());
            }

            Func * func = current_object->getType()->getFunc(funcname, spec);
            if (!func) {
                res = "No matching function.";
                return true;
            }
            Ptr<ValueBase> result = func->call(current_object, params);
            res = info(result);
        } catch (exception * ex) {
            res = ex->what();
            res += "\nAn error occurred.\n";
        }
        return true;
    }

}




namespace Scripting {

    string CommandLine::eval(const string & line) {
        Tokenizer tokenizer(line);

        string result;
        bool handled =
               handle_cd_path(env, tokenizer, result)
            || handle_ls(env, tokenizer, result)
            || handle_ls_path(env, tokenizer, result)
            || handle_la(env, tokenizer, result)
            || handle_la_path(env, tokenizer, result)
            || handle_funcs(env, tokenizer, result)
            || handle_funcs_path(env, tokenizer, result)
            || handle_vars(env, tokenizer, result)
            || handle_path(env, tokenizer, result)
            || handle_var_equals_literal(env, tokenizer, result)
            || handle_var_equals_path(env, tokenizer, result)
            || handle_var_equals_path_funcname(env, tokenizer, result)
            || handle_path_funcname(env, tokenizer, result)
            || handle_eval(env,tokenizer,result)
            || !tokenizer.nextToken()
            ;
        if (handled)
            return result;
        else
            return usage();
    }

} // namespace Scripting
