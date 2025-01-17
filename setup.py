"""A setuptools based setup module.

See:
https://packaging.python.org/guides/distributing-packages-using-setuptools/
https://github.com/pypa/sampleproject
"""
import sys
import pathlib
from setuptools import setup, find_packages  # type: ignore

here = pathlib.Path(__file__).parent.resolve()

# Get the version number
# pylint: disable = wrong-import-position, no-member
sys.path.append(str(here / "src/icpp_candid"))
import version  # type: ignore

setup(
    #
    # Versions should comply with PEP 440:
    # https://www.python.org/dev/peps/pep-0440/
    #
    # For a discussion on single-sourcing the version across setup.py and the
    # project code, see
    # https://packaging.python.org/en/latest/single_source_version.html
    version=version.__version__,  # Required
    #
    # When your source code is in a subdirectory under the project root, e.g.
    # `src/`, it is necessary to specify the `package_dir` argument.
    package_dir={"": "src"},  # Optional
    #
    # You can just specify package directories manually here if your project is
    # simple. Or you can use find_packages().
    #
    # Alternatively, if you just want to distribute a single Python file, use
    # the `py_modules` argument instead as follows, which will expect a file
    # called `my_module.py` to exist:
    #
    #   py_modules=["my_module"],
    #
    packages=find_packages(where="src"),  # Required
    #
    #
    # If there are data files included in your packages that need to be
    # installed, specify them here.
    # https://setuptools.pypa.io/en/latest/userguide/datafiles.html#package-data
    #
    # (-) add 'py.typed', to play nice with mypy
    #     https://mypy.readthedocs.io/en/latest/installed_packages.html
    #     #making-pep-561-compatible-packages
    #
    package_data={  # Optional
        "icpp_candid": [
            "candid/*.c",
            "candid/*.cpp",
            "candid/*.h",
            "candid/*.hpp",
            "hooks/*.c",
            "hooks/*.cpp",
            "hooks/*.h",
            "hooks/*.hpp",
            "vendors/cppcodec/*",
            "vendors/cppcodec/data/*",
            "vendors/cppcodec/detail/*",
            "vendors/hash-library/*",
            "vendors/hash-library/tests/*",
            "py.typed",
        ],
    },
)
