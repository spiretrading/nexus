import { parse, stringify } from 'lossless-json';
import { createJSONEditor, Mode, ReadonlyValue } from 'vanilla-jsoneditor';
import { create, preformatted } from '../report_viewer/text';

__webpack_public_path__ = new URL('../', document.currentScript.src).href;

function createViewer(container, file, onError) {
  let editor = null;
  const viewer = create(container, file, text => {
    let hasConstructor = false;
    const json = parse(text, (key, value) => {
      if(key === 'constructor') {
        hasConstructor = true;
      }
      return value;
    });
    if(hasConstructor) {
      return preformatted(stringify(json, null, 2));
    }
    const content = document.createElement('div');
    content.style.height = '100%';
    content.style.setProperty('--jse-main-border', 'none');
    content.addEventListener('click', event => {
      if((event.ctrlKey || event.metaKey) && event.target.closest(
          '[data-type="selectable-value"]')) {
        event.preventDefault();
        event.stopPropagation();
      }
    }, true);
    content.addEventListener('keydown', event => {
      if((event.ctrlKey || event.metaKey) && event.key === 'Enter') {
        event.preventDefault();
        event.stopPropagation();
      }
    }, true);
    editor = createJSONEditor({
      target: content,
      props: {
        content: {json},
        parser: {parse, stringify},
        mode: Mode.tree,
        readOnly: true,
        mainMenuBar: false,
        navigationBar: false,
        statusBar: false,
        onRenderValue: props => [{component: ReadonlyValue, props}],
        onError: () => onError?.()
      }
    });
    return content;
  });
  return {
    ready: viewer.ready,
    dispose() {
      viewer.dispose();
      if(editor) {
        editor.destroy().catch(() => {});
        editor = null;
      }
    }
  };
}

window.spireJsonViewerReady = Promise.resolve({create: createViewer});
