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
DIRS:=. ./components
INTERMEDIATE_DIR:=output
DEPLOYMENT_DIR:=../../../Application/webapp
JSX_FILES:=$(foreach dir, $(DIRS), $(wildcard ../$(dir)/*.jsx))
_JS_FILES:=$(JSX_FILES:.jsx=.js)
JS_FILES:=$(addprefix $(INTERMEDIATE_DIR)/, $(_JS_FILES:../%=%))

ifeq ($(UNAME), windows32)
	canon_path=$(subst /,\,$1)
else
	canon_path=$1
endif

target_name=$(1:output/%=%)

all: $(JS_FILES)
	$(foreach file,$(JS_FILES),$(shell $(MKDIR) $(call dir,$(call canon_path,$(DEPLOYMENT_DIR)/$(call target_name,$(file))))))
	$(foreach file,$(JS_FILES),$(shell $(CP) $(file) $(call canon_path,$(DEPLOYMENT_DIR)/$(call target_name,$(file)))))

$(INTERMEDIATE_DIR)/%.js: ../%.jsx
	$(MKDIR) $(call canon_path,$(@D))
	$(BABEL) $< $(BABEL_FLAGS) --out-file $@

clean:
	$(RM) $(INTERMEDIATE_DIR)
	$(foreach file,$(JS_FILES),$(shell $(RM) $(call canon_path,$(DEPLOYMENT_DIR)/$(call target_name,$(file)))))
