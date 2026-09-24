# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
PYTHON        = python3
SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
SPHINXPROJ    = SpireWebServices
SOURCEDIR     = source
BUILDDIR      = build

# Put it first so that "make" without argument is like "make help".
help:
	@"$(PYTHON)" build.py "$(SPHINXBUILD)" help "$(SOURCEDIR)" \
	  "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@"$(PYTHON)" build.py "$(SPHINXBUILD)" "$@" "$(SOURCEDIR)" \
	  "$(BUILDDIR)" $(SPHINXOPTS) $(O)
