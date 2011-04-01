#!/usr/bin/python

import re

# match all occurrences of characters that have a syntactic meaning in regular expressions:
# () [] {} braces, ., +, ^, $ and \ (backslash) characters without trailing * or ?
_re0 = re.compile(r"[()[\]{}.+^$]|\\(?![*?])")
# match all * which are not preceded by a \ character
_re1 = re.compile(r"(?<!\\)[*]")
# match all ? which are not preceded by a \ character
_re2 = re.compile(r"(?<!\\)[?]")

def matches(wildcards, text):
    return bool(compile(wildcards).match(text))
    
def compile(wildcards):
    restring = _re0.sub(r"\\\g<0>", wildcards)
    #print restring
    restring = _re1.sub(r".\g<0>", restring)
    #print restring
    restring = _re2.sub(r".", restring)
    #print restring
    return re.compile("^"+restring+"$")
