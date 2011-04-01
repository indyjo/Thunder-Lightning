#!/usr/bin/python

import sys
import xmllib
import xmlclasses

class TexgenParser(xmllib.XMLParser):
    def __init__(self, xmlclasses):
        xmllib.XMLParser.__init__(self)
        self.xmlclasses = xmlclasses
        self.obj_stack = []
    def unknown_starttag(self, tag, attributes):
        print len(self.obj_stack) * "  " + "<" + tag + ">"
        if not self.xmlclasses.has_key(tag):
            raise xmlclasses.XMLNameException("No handler class defined for xml tag <" + tag + ">")
        if len(self.obj_stack) == 0:
            o = self.xmlclasses[tag](None, attributes)
            self.root_obj = o
        else:
            o = self.xmlclasses[tag] ( self.obj_stack[len(self.obj_stack)-1], attributes )
        self.obj_stack.append(o)
    def unknown_endtag(self, tag):
        self.obj_stack.pop()
        print len(self.obj_stack) * "  " + "</" + tag + ">"


parser = TexgenParser(xmlclasses.xmlclasses)

for line in sys.stdin.readlines():
    parser.feed(line)
   
parser.root_obj.run()
