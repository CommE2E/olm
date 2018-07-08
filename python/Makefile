PYTHON   ?= python

all: olm

olm:
	DEVELOP=$(DEVELOP) $(PYTHON) setup.py build

install: olm
	$(PYTHON) setup.py install --skip-build -O1 --root=$(DESTDIR)

test: develop py2develop
	python3 -m pytest
	python2 -m pytest
	python3 -m pytest --flake8 --benchmark-disable
	python3 -m pytest --isort --benchmark-disable
	python3 -m pytest --cov --cov-branch --benchmark-disable

clean:
	-rm -r python_olm.egg-info/ dist/ __pycache__/
	-rm *.so _libolm.o
	-rm -r packages/
	-rm -r build/

develop: _libolm.o
py2develop: _libolm.so

_libolm.so: DEVELOP ?= 1
_libolm.so: include/olm/olm.h olm_build.py
	DEVELOP=$(DEVELOP) python2 olm_build.py
	-rm _libolm.c

_libolm.o: DEVELOP ?= 1
_libolm.o: include/olm/olm.h olm_build.py
	DEVELOP=$(DEVELOP) python3 olm_build.py
	-rm _libolm.c

.PHONY: all olm install clean test develop
