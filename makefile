CPP_DEPS = \
	simhash/simhash.pyx \
	simhash/simhash.pxd \
	simhash/simhash-cpp/include/permutation.h \
	simhash/simhash-cpp/src/permutation.cpp \
	simhash/simhash-cpp/include/simhash.h \
	simhash/simhash-cpp/src/simhash.cpp

.PHONY: test
test: simhash/simhash.so
	nosetests --verbose --nocapture

simhash/simhash.so: $(CPP_DEPS)
	python setup.py build_ext --inplace

clean:
	rm -rf simhash.egg-info build dist simhash/simhash.cpp
	find . -name '*.pyc' | xargs --no-run-if-empty rm -f
	find simhash -name '*.so' | xargs --no-run-if-empty rm -f

install:
	python setup.py install
