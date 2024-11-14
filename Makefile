BASE_DIR=${PWD}
PLUGIN=$(BASE_DIR)/plugins
COMPILER=$(shell root-config --cxx)
FLAGS=$(shell root-config --cflags --libs) -g -O3 -Wall -Wextra -Wpedantic
INCLUDE=-I $(PLUGIN) -I $(PLUGIN)/blinder/rlib/include

all: fit ana skim phase range scale-opt scale-opt-re time count

clean: clean-build clean-artifact

cleanf: cleanfiles 

cleanall: clean-build clean-artifact clean-blinder

fit: src/fit.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) -lBlinders $(INCLUDE) -L $(PLUGIN)/blinder $< -o bin/$@

ana: src/analysis.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

skim: src/skim.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

phase: src/phaseval.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

range: src/rangeAna.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

scale-opt: src/scale-optimizer.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

scale-opt-re: src/scale-optimizer-re.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

test: src/simple_loop.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

time: src/makeTime.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) -lBlinders $(INCLUDE) -L $(PLUGIN)/blinder $< -o bin/$@

count: src/countEvent.cc
	@[ -d $(BASE_DIR)/bin ] || mkdir -p $(BASE_DIR)/bin
	$(COMPILER) $(FLAGS) $(INCLUDE) $< -o bin/$@

clean-build:
	rm -rf ./bin core  *.o

cleanfiles:
	rm *.txt; rm *.root; rm *.pdf

sanitize:
	find . \( -name "*.toc" -o -name "*~" -o -name "__*__" -o -name "#*#" \) -print0 | xargs -0 rm -rf

install-blinder:
	bash -c "cd plugins/blinder; ./install.sh"

clean-blinder:
	rm -rf plugins/blinder/Blinders.o plugins/blinder/libBlinders.so  plugins/blinder/packages plugins/blinder/rlib plugins/blinder/testBlinding.exe 
