SHELL := /bin/bash

# Disable built-in rules and variables
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

NETWORK := local

###########################################################################
# Building and publishing the pypi package
.PHONY: pypi-build
pypi-build:
	rm -rf dist
	python -m build

.PHONY: testpypi-upload
testpypi-upload:
	twine upload --config-file .pypirc -r testpypi dist/*

.PHONY: testpypi-install
testpypi-install:
	pip install -i https://test.pypi.org/simple/ --extra-index-url https://pypi.org/simple/ icpp-candid

.PHONY: pypi-upload
pypi-upload:
	twine upload --config-file .pypirc dist/*

.PHONY: pypi-install
pypi-install:
	pip install icpp-candid