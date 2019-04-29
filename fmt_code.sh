#!/bin/bash
#https://wiki.qt.io/Qt_Coding_Style
astyle --style=kr  \
--indent=spaces=4  \
--align-pointer=name  \
--align-reference=name  \
--convert-tabs  \
--attach-namespaces \
--max-code-length=100  \
--max-instatement-indent=120  \
--pad-header \
--pad-oper \
--recursive  *.cpp *.h
