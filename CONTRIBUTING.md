# Contributing

## Setup development env

### Install development prerequicites:

```shell
apt install build-essential cmake indent libcurl4-gnutls-dev lcov
```

##### Mimick

```shell
git clone https://github.com/Snaipe/Mimick.git
mkdir Mimick/build
cd Mimick/build
cmake ..
sudo make install
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

#### Coverage result

```shell
make clean cover-html
```

Or

```shell
make clean cover-html-show
```

to automatically open the browser and show the coverage result index html 
file.

### Coding style

Run make indent to uniform the code with `GNU indent` tool before commit.

Run:

```shell
make indent
```

to reformat the entire project.

## Code of Conduct

We value the participation of each individual and want all to have an enjoyable and fulfilling experience. Nobody knows everything, and nobody is expected to be perfect. Asking questions is how we learn.

## What matters to us
- If you contribute to this project it must be because you want to not because you have to.
- You must accept that as a memeber of this opensource comunity you are responsible for the code you commit.  
- Every member of this comunity must be respectfull of others, their positions, their skills, their commitments, and their efforts.
- Everyone must gracefully accept constructive criticism.
- Be Considerate: Everyone is around to either help or be helped. We’re all around to learn. All the work we do affects someone else and we need to consider this when we make any changes.

## What we don't care about
We don't care about your level of experience, gender, gender identity and expression, sexual orientation, disability, personal appearance, body size, race, ethnicity, age, or religion.
