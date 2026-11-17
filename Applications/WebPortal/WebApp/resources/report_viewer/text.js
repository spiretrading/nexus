export function create(container, source, render) {
  const file = new URL(source, document.baseURI);
  const localBlob = location.protocol === 'file:' &&
    file.protocol === 'blob:' && file.origin === 'null';
  if(!localBlob && (file.origin !== location.origin ||
      !['http:', 'https:', 'blob:'].includes(file.protocol))) {
    throw new Error('The report must be on this origin.');
  }
  const abort = new AbortController();
  let content = null;
  let disposed = false;
  const ready = (async () => {
    const response = await fetch(file.href, {signal: abort.signal,
      credentials: 'same-origin'});
    if(!response.ok) {
      throw new Error('Unable to load report.');
    }
    const text = await response.text();
    if(disposed) {
      return;
    }
    content = render(text);
    container.appendChild(content);
  })();
  return {
    ready,
    dispose() {
      if(disposed) {
        return;
      }
      disposed = true;
      abort.abort();
      content?.remove();
    }
  };
}

export function preformatted(text) {
  const content = document.createElement('pre');
  content.style.margin = '0';
  content.style.fontFamily = 'monospace';
  content.style.fontSize = '14px';
  content.style.lineHeight = '1.5';
  content.style.tabSize = '4';
  content.textContent = text;
  return content;
}
