.PHONY: test

clean:
	sudo python setup.py clean
	rm simhash/table.cpp

install:
	sudo python setup.py install

uninstall:
	sudo pip uninstall simhash -y

test:
	python test/test.py

