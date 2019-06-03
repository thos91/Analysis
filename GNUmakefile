PROCESS = wgDecoder wgMakeHist wgAnaHist wgAnaHistSummary wgChangeConfig wgOptimize wgPreCalib wgAnaPedestal wgAnaPedestalSummary # wgCheckData wgScurve wgScurveSummary wgCalib wgCalibDiff wgRecon wgDQCheck wgBsdSpillCheck wgHitTimeCheck wgDQHistory wgReconDisp IngDisp wgSpillEff wgSpillCheck wgGainCheck
MDIR = process

define maker
$(1):
	@echo "make $1:";	cd $(MDIR)/$1 && make
endef

define clean
	@echo "clean:";	cd $(MDIR)/$1 && rm -f obj/* *.d *.o ; cd ../../
endef

.PHONY: all clean

all:$(PROCESS)

$(foreach tar,$(PROCESS),$(eval $(call maker,$(tar))))

clean:
	$(foreach tar,$(PROCESS),$(call clean,$(tar)))
	rm -f obj/*	
	rm -f bin/*
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
