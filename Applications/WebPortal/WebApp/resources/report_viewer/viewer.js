const pdf = require('pdfjs-dist/build/pdf.mjs');
const script = document.currentScript.src;
window.spirePdfViewerReady = (async () => {
  const {EventBus, LinkTarget, PDFLinkService, PDFViewer} =
    require('pdfjs-dist/web/pdf_viewer.mjs');
  const assets = new URL('../pdfjs/', script);
  pdf.GlobalWorkerOptions.workerSrc =
    new URL('build/pdf.worker.mjs', assets).href;
  if(location.protocol === 'file:' && !globalThis.pdfjsWorker) {
    await new Promise((resolve, reject) => {
      const worker = document.createElement('script');
      worker.src = new URL('worker.js', script).href;
      worker.onload = () => {
        worker.remove();
        resolve();
      };
      worker.onerror = () => {
        worker.remove();
        reject(new Error('Unable to load PDF worker.'));
      };
      document.head.appendChild(worker);
    });
  }

  function create(container, source, onError) {
    const file = new URL(source, document.baseURI);
    const localBlob = location.protocol === 'file:' &&
      file.protocol === 'blob:' && file.origin === 'null';
    if(!localBlob && (file.origin !== location.origin ||
        !['http:', 'https:', 'blob:'].includes(file.protocol))) {
      throw new Error('The report must be on this origin.');
    }
    const pages = document.createElement('div');
    pages.className = 'pdfViewer';
    container.replaceChildren(pages);
    const eventBus = new EventBus();
    const linkService = new PDFLinkService({eventBus,
      externalLinkTarget: LinkTarget.BLANK,
      externalLinkRel: 'noopener noreferrer', ignoreDestinationZoom: true});
    const abort = new AbortController();
    const viewer = new PDFViewer({container, viewer: pages, eventBus,
      linkService,
      annotationMode: pdf.AnnotationMode.ENABLE,
      annotationEditorMode: pdf.AnnotationEditorType.DISABLE,
      imageResourcesPath: new URL('web/images/', assets).href,
      removePageBorders: true, abortSignal: abort.signal});
    linkService.setViewer(viewer);
    let disposed = false;
    let initialized = false;
    eventBus.on('pagesinit', () => {
      initialized = true;
      viewer.currentScaleValue = 'page-width';
    });
    eventBus.on('pagerendered', event => {
      if(!disposed && event.error) {
        onError();
      }
    });
    const observer = new ResizeObserver(() => {
      container.style.setProperty('--viewer-container-height',
        `${container.clientHeight}px`);
      if(initialized && !disposed && container.clientWidth > 0) {
        viewer.currentScaleValue = 'page-width';
        viewer.update();
      }
    });
    observer.observe(container);
    const loading = pdf.getDocument({url: file.href,
      cMapUrl: new URL('cmaps/', assets).href, cMapPacked: true,
      standardFontDataUrl: new URL('standard_fonts/', assets).href,
      wasmUrl: new URL('wasm/', assets).href,
      iccUrl: new URL('iccs/', assets).href});
    const ready = (async () => {
      const document = await loading.promise;
      await document.getPage(1);
      if(disposed) {
        return;
      }
      linkService.setDocument(document);
      viewer.setDocument(document);
      await viewer.firstPagePromise;
    })();
    return {
      ready,
      dispose() {
        if(disposed) {
          return;
        }
        disposed = true;
        observer.disconnect();
        abort.abort();
        viewer.setDocument(null);
        linkService.setDocument(null);
        loading.destroy().catch(() => {});
        pages.remove();
      }
    };
  }
  return {create};
})();
window.spirePdfViewerReady.catch(() => {});
