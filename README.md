# httpload

[![Build](https://github.com/dobisel/httpload/actions/workflows/build.yml/badge.svg)](https://github.com/dobisel/httpload/actions/workflows/build.yml)
[![Coverage Status](https://coveralls.io/repos/github/dobisel/httpload/badge.svg)](https://coveralls.io/github/dobisel/httpload)

Event based HTTP stress testing. 


## Contribution

Take a look at [Code of conduct](CODE_OF_CONDUCT.md).


### Install development prerequicites:

```shell
apt install build-essential make indent libcurl4-gnutls-dev
sudo snap install ruby --classic
```

```shell
git clone https://github.com/pylover/http-parser.git
cd http-parser
sudo make install
```

### Coding style

Run:

```shell
make indent
```

to reformat the entire project.

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

### Indent

Run make indent to uniform the code with `GNU indent` tool before commit.

```shell
make indent
```

