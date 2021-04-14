# httpload

[![Build](https://github.com/dobisel/httpload/actions/workflows/build.yml/badge.svg)](https://github.com/dobisel/httpload/actions/workflows/build.yml)
[![Coverage Status](https://coveralls.io/repos/github/dobisel/httpload/badge.svg)](https://coveralls.io/github/dobisel/httpload)

Event based HTTP stress testing. 


## Contribution

Take a look at [Code of conduct](CODE_OF_CONDUCT.md).


### Install development prerequicites:

```shell
apt install build-essential make indent libcurl4-gnutls-dev
```

##### cpp-coveralls

This tool is required to submit coverage result to `coveralls.io`.

```shell
python -m pip install git+https://github.com/pylover/cpp-coveralls.git
```

##### HTTP parser

```shell
git clone https://github.com/pylover/http-parser.git
sudo make -C http-parser install
sudo ldconfig /usr/local/lib/
```

### Running tests

```shell
make clean test
make clean cover
```

Single test file:

Use the filename without the prefix `tests/test_` and `*.c` suffix.
for example, to run `tests/test_logging.c` execute:

```shell
make clean test UNIT=logging
```

### Coding style

Run make indent to uniform the code with `GNU indent` tool before commit.

Run:

```shell
make indent
```

to reformat the entire project.
