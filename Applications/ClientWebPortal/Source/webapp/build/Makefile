UNAME:=$(shell uname)
RM:=rm -rf
CP:=cp
ifeq ($(UNAME), windows32)
	NODEJS:=node
	MKDIR:=mkdir
else
	NODEJS:=nodejs
	MKDIR:=mkdir -p
endif
BABEL:=$(NODEJS) ../node_modules/babel-cli/bin/babel.js
BABEL_FLAGS:=
DIRS:=. ./components ./services
INTERMEDIATE_DIR:=output
DEPLOYMENT_DIR:=../../../Application/webapp
JSX_FILES:=$(foreach dir, $(DIRS), $(wildcard ../$(dir)/*.jsx))
_OUTPUT_FILES:=$(JSX_FILES:.jsx=.js)
OUTPUT_FILES:=$(addprefix $(INTERMEDIATE_DIR)/, $(_OUTPUT_FILES:../%=%))
JS_FILES:=$(foreach dir, $(DIRS), $(wildcard ../$(dir)/*.js))
OUTPUT_JS_FILES:=$(addprefix $(INTERMEDIATE_DIR)/, $(JS_FILES:../%=%))

ifeq ($(UNAME), windows32)
	canon_path=$(subst /,\,$1)
else
	canon_path=$1
endif

target_name=$(1:output/%=%)

all: $(OUTPUT_FILES) $(OUTPUT_JS_FILES)
	$(CP) -r $(INTERMEDIATE_DIR)/* $(DEPLOYMENT_DIR)

$(OUTPUT_FILES): $(INTERMEDIATE_DIR)/%.js: ../%.jsx
	- $(MKDIR) $(call canon_path,$(@D))
	$(BABEL) $< $(BABEL_FLAGS) --out-file $@

$(OUTPUT_JS_FILES): $(INTERMEDIATE_DIR)/%.js: ../%.js
	echo $< $@
	- $(MKDIR) $(call canon_path,$(@D))
	- $(CP) $< $@

clean:
	- $(RM) $(INTERMEDIATE_DIR)
	- $(foreach file,$(OUTPUT_FILES),$(shell $(RM) $(call canon_path,$(DEPLOYMENT_DIR)/$(call target_name,$(file)))))
