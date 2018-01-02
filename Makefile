GCC = @g++
LEX = @flex
YACC = @bison
MKDIR_P = mkdir -p
BUILD_DIR = build
BIN_DIR = build/bin
MAINCC = build/main.cc
MAINY = src/main.y
TOKENIZERL = src/tokenizer.l
TOKENIZERCC = build/tokenizer.cc
DEFINES = build/main.tab.h
MAINBIN = build/bin/main
CFLAG = -I "src"

main: $(BUILD_DIR) $(BIN_DIR) $(MAINCC) $(TOKENIZERCC) src/syntax.h
	$(GCC) -g $(MAINCC) $(TOKENIZERCC) -lfl -o $(MAINBIN) $(CFLAG)

$(BUILD_DIR): 
	$(MKDIR_P) $(BUILD_DIR)

$(BIN_DIR):
	$(MKDIR_P) $(BIN_DIR)

$(MAINCC): $(MAINY) 
	$(YACC) $(MAINY) -o $(MAINCC) --defines=$(DEFINES)

$(TOKENIZERCC): $(MAINCC) $(TOKENIZERL) 
	$(LEX) -o $(TOKENIZERCC) $(TOKENIZERL)



clean:
				@-rm -rf build
.PHONY: clean
