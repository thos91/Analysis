PROCESS = Decoder wgMakeHist wgAnaHist wgAnaHistSummary wgChangeConfig wgCheckData wgScurve wgScurveSummary wgOptimize wgPreCalib wgCalib wgCalibDiff wgAnaPedestal wgAnaPedestalSummary wgRecon wgDQCheck wgBsdSpillCheck wgHitTimeCheck wgDQHistory wgReconDisp IngDisp wgSpillEff wgSpillCheck wgGainCheck
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
