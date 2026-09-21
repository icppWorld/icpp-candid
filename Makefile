SHELL := /bin/bash

# Disable built-in rules and variables
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

NETWORK := local

###########################################################################
# Building the pypi package.
#
# Publishing is CI-only: pushing a `vX.Y.Z` tag runs .github/workflows/
# release.yml, which builds and publishes to PyPI with trusted publishing
# (OIDC). There is no API token, no .pypirc and no upload target here - see
# icpp-pro/README-release-guide.md.
.PHONY: pypi-build
pypi-build:
	rm -rf dist
	python -m build

.PHONY: pypi-install
pypi-install:
	python -m pip install icpp-candid