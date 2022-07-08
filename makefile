COMPILE_CMD = x86_64-w64-mingw32-g++
LINK_CMD = x86_64-w64-mingw32-g++
OBJ_DIR = bin/obj
OUT_DIR = bin/out
DEBUG_CC_FLAGS = -ggdb -c -Wall -D cdwDebugMode
RELEASE_CC_FLAGS = -O3 -c -Wall
DEBUG_LNK_FLAGS_POST = -ggdb -static-libgcc -static-libstdc++ -static
RELEASE_LNK_FLAGS_POST = -static-libgcc -static-libstdc++ -static

debug: \
	dirs \
	$(OUT_DIR)/debug/akiro.txt \
	$(OUT_DIR)/debug/akiro.exe \
	$(OUT_DIR)/debug/akcompact.exe \
	$(OUT_DIR)/debug/akmonitor.exe \

all: \
	debug \
	$(OUT_DIR)/release/akiro.exe \
	$(OUT_DIR)/release/akcompact.exe \
	$(OUT_DIR)/release/akmonitor.exe \

clean:
	rm -rf bin

dirs:
	@mkdir -p $(OBJ_DIR)/debug/cmn
	@mkdir -p $(OBJ_DIR)/debug/akiro
	@mkdir -p $(OBJ_DIR)/debug/akcompact
	@mkdir -p $(OBJ_DIR)/debug/akmonitor
	@mkdir -p $(OBJ_DIR)/release/cmn
	@mkdir -p $(OBJ_DIR)/release/akiro
	@mkdir -p $(OBJ_DIR)/release/akcompact
	@mkdir -p $(OBJ_DIR)/release/akmonitor
	@mkdir -p $(OUT_DIR)/debug
	@mkdir -p $(OUT_DIR)/release

.PHONY: debug all clean dirs

# ----------------------------------------------------------------------
# cmn

CMN_SRC = \
	src/cmn/crypto.cpp \
	src/cmn/file.cpp \
	src/cmn/path.cpp \
	src/cmn/shmem-block.cpp \
	src/cmn/shmem.cpp \
	src/cmn/staging.cpp \
	src/cmn/temp.cpp \
	src/cmn/wlog.cpp \
	src/cmn/worker.cpp \

CMN_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(CMN_SRC)))

$(OUT_DIR)/debug/cmn.lib: $(CMN_DEBUG_OBJ)
	$(info $< --> $@)
	@ar crs $@ $^

$(CMN_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

CMN_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(CMN_SRC)))

$(OUT_DIR)/release/cmn.lib: $(CMN_RELEASE_OBJ)
	$(info $< --> $@)
	@ar crs $@ $^

$(CMN_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# akiro

AKIRO_SRC = \
	src/akiro/cmdStartStopStatus.cpp \
	src/akiro/configParser.cpp \
	src/akiro/main.cpp \
	src/akiro/start.cpp \

$(OUT_DIR)/debug/akiro.txt : akiro.txt
	$(info $< --> $@)
	@cp $< $@

AKIRO_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(AKIRO_SRC)))

$(OUT_DIR)/debug/akiro.exe: $(AKIRO_DEBUG_OBJ) $(OUT_DIR)/debug/cmn.lib
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(AKIRO_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -lcmn

$(AKIRO_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

AKIRO_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(AKIRO_SRC)))

$(OUT_DIR)/release/akiro.exe: $(AKIRO_RELEASE_OBJ) $(OUT_DIR)/release/cmn.lib
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(AKIRO_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -lcmn

$(AKIRO_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# akcompact

AKCOMPACT_SRC = \
	src/akcompact/cmdCompact.cpp \
	src/akcompact/fileDb.cpp \
	src/akcompact/main.cpp \
	src/akcompact/rootDb.cpp \
	src/akcompact/treeDb.cpp \

AKCOMPACT_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(AKCOMPACT_SRC)))

$(OUT_DIR)/debug/akcompact.exe: $(AKCOMPACT_DEBUG_OBJ) $(OUT_DIR)/debug/cmn.lib
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(AKCOMPACT_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -lcmn

$(AKCOMPACT_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

AKCOMPACT_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(AKCOMPACT_SRC)))

$(OUT_DIR)/release/akcompact.exe: $(AKCOMPACT_RELEASE_OBJ) $(OUT_DIR)/release/cmn.lib
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(AKCOMPACT_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -lcmn

$(AKCOMPACT_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# akmonitor

AKMONITOR_SRC = \
	src/akmonitor/main.cpp \

AKMONITOR_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(AKMONITOR_SRC)))

$(OUT_DIR)/debug/akmonitor.exe: $(AKMONITOR_DEBUG_OBJ) $(OUT_DIR)/debug/cmn.lib
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(AKMONITOR_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -lcmn

$(AKMONITOR_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

AKMONITOR_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(AKMONITOR_SRC)))

$(OUT_DIR)/release/akmonitor.exe: $(AKMONITOR_RELEASE_OBJ) $(OUT_DIR)/release/cmn.lib
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(AKMONITOR_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -lcmn

$(AKMONITOR_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@
