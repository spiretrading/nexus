BABEL:=nodejs ../node_modules/babel-cli/bin/babel.js
BABEL_FLAGS:=
INTERMEDIATE_DIR:=output
DEPLOYMENT_DIR:=../../../Application/webapp
JSX_FILES:=$(wildcard ../*.jsx)
JS_FILES:=$(addprefix $(INTERMEDIATE_DIR)/, $(notdir $(JSX_FILES:.jsx=.js)))

all: $(INTERMEDIATE_DIR) $(JS_FILES)
	cp $(INTERMEDIATE_DIR)/*.js $(DEPLOYMENT_DIR)

$(INTERMEDIATE_DIR):
	mkdir $(INTERMEDIATE_DIR)

$(INTERMEDIATE_DIR)/%.js: ../%.jsx
	$(BABEL) $< $(BABEL_FLAGS) --out-file $@

clean:
	rm -rf $(INTERMEDIATE_DIR)
	rm $(DEPLOYMENT_DIR)/*.js
