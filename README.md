Step-by-step Derivative Calculator
==================================

This is the source code for "Step-by-step Derivative Calculator",
also known as "EasyDerivSteps". It was written by me ("bl0ckeduser")
in May, June 2013. It is a fork of my project "symdiff" which is 
a symbolic differentiator that lacks a GUI, lacks latex output, 
and lacks a system for showing steps in a relatively intelligent manner.

This version has a GUI and shows the steps.

The source is somewhat hacky / messy in spots. Some of the step-by-step
stuff was quickly done. I have however added a reasonable quantity of comments.


Homepage with compiled windows binary download is here:

https://sites.google.com/site/stepderivcalc


Compiling
=========

To compile, emscripten (providing the command "emcc") is required.
As of writing, I am using version:

1.4.7 (commit a1eb425371aa310e074b06d80d56adf71d6f5383)

To run, xulrunner is required.

linux-run.sh and build.sh are the relevant scripts.

The compiled stuff all goes into "compiled-xul-app", which also stores
a copy of mathjax and the botched XUL GUI boilerplate code.

build-win32.sh builds a windows archive pre-bundled with the
windows version of xulrunner. it will probably need editing because
it uses a hardcoded path to a volume on my machine.
