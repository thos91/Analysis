.PHONY: all clean

all: doc

clean:
	cd doc; rm -rf build source *.pyc __pycache__ 

prepare:
	if test -d doc/source; then rm -rf doc/source; fi
	mkdir doc/source
	cp -f doc/conf.py doc/source
	cp -f doc/base_add_doc_path.py doc/source/add_doc_path.py
	cp -f doc/index.rst doc/source
	for i in `find ./ -name '*.rst' | grep -v "docs"`; do cp -f $$i doc/source; echo "sys.path.append(\""`dirname $$i`"\")" >> doc/source/add_doc_path.py; done
	for i in `find ./ -name '*.doc.svg' | grep -v "docs"`; do cp -f $$i doc/source; done
	for i in `find ./ -name '*.doc.png' | grep -v "docs"`; do cp -f $$i doc/source; done

doc: prepare
	cd doc; sphinx-build -b html source build
