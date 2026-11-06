import { create, preformatted } from '../report_viewer/text';

window.spireTextViewerReady = Promise.resolve({
  create(container, file) {
    return create(container, file, preformatted);
  }
});
