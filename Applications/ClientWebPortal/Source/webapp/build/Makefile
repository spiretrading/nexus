BABEL:=node ./../node_modules/babel-cli/bin/babel.js
BABEL_FLAGS:=
INTERMEDIATE_DIR:=output
JSX_FILES:=$(wildcard ../*.jsx)
JS_FILES:=$(addprefix $(INTERMEDIATE_DIR)/, $(notdir $(JSX_FILES:.jsx=.js)))

all: $(INTERMEDIATE_DIR) $(JS_FILES)

$(INTERMEDIATE_DIR):
	mkdir $(INTERMEDIATE_DIR)

$(INTERMEDIATE_DIR)/%.js: ../%.jsx
	$(BABEL) $< $(BABEL_FLAGS) --out-file $@
