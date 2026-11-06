import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { PdfReportViewer, ReportDetailModel, ReportViewer } from
  '../../source/pages/report_page/report_detail_page';

describe('ReportViewer', () => {
  const report: ReportDetailModel.Report = {
    id: 'report',
    title: 'Account Statement',
    parameters: [],
    mediaType: 'application/pdf',
    status: ReportDetailModel.Status.READY,
    filePath: '/api/report?id=1&name=statement.pdf'
  };

  it('selects PDFs by media type rather than filename', () => {
    for(const mediaType of ['application/pdf', 'APPLICATION/PDF; version=1.7']) {
      const viewer = ReportViewer({report: {...report, mediaType}});
      assert.equal(viewer.type, PdfReportViewer);
      assert.equal(viewer.props.filePath, report.filePath);
      assert.equal(viewer.props.title, report.title);
    }
    for(const mediaType of ['text/csv', 'text/markdown',
        'application/octet-stream', '']) {
      assert.equal(ReportViewer({report: {...report, mediaType}}), null);
    }
    for(const status of [ReportDetailModel.Status.IN_PROGRESS,
        ReportDetailModel.Status.FAILED]) {
      assert.equal(ReportViewer({report: {...report, status}}), null);
    }
    assert.equal(ReportViewer({report: {...report, filePath: ''}}), null);
  });

  it('loads on demand and ignores failures from replaced reports', async () => {
    const originalDocument =
      Object.getOwnPropertyDescriptor(globalThis, 'document');
    const originalWindow = Object.getOwnPropertyDescriptor(globalThis, 'window');
    const resources: any[] = [];
    const viewers: any[] = [];
    const runtime = {
      create(container: unknown, file: string, onError: () => void) {
        let resolve: () => void;
        let reject: (error: Error) => void;
        const ready = new Promise<void>((accept, fail) => {
          resolve = accept;
          reject = fail;
        });
        const viewer = {file, container, ready, resolve, reject, onError,
          disposed: false, dispose() {this.disposed = true;}};
        viewers.push(viewer);
        return viewer;
      }
    };
    Object.defineProperty(globalThis, 'document', {configurable: true,
      value: {
        baseURI: 'https://portal.example/',
        createElement: () => ({remove() {}}),
        head: {appendChild(element: any) {
          resources.push(element);
          queueMicrotask(() => element.onload());
        }}
      }});
    Object.defineProperty(globalThis, 'window', {configurable: true,
      value: {spirePdfViewerReady: Promise.resolve(runtime)}});
    const component = new PdfReportViewer({filePath: 'api/report?id=1&name=a',
      title: report.title}) as any;
    component.container.current = {};
    component.setState = (state: {failed: boolean}, callback?: () => void) => {
      component.state = {...component.state, ...state};
      callback?.();
    };
    try {
      const element = component.render().props.children;
      assert.equal(element.type, 'div');
      assert.equal(element.props['aria-label'], report.title);
      assert.equal(resources.length, 0);
      component.componentDidMount();
      await new Promise(resolve => setImmediate(resolve));
      assert.equal(resources.length, 3);
      assert.notEqual(resources[2].type, 'module');
      assert.equal(viewers[0].file,
        'https://portal.example/api/report?id=1&name=a');
      const previous = component.props;
      component.props = {...previous, filePath: 'blob:https://portal.example/2'};
      component.componentDidUpdate(previous);
      await new Promise(resolve => setImmediate(resolve));
      assert.ok(viewers[0].disposed);
      assert.equal(viewers[1].file, 'blob:https://portal.example/2');
      viewers[0].reject(new Error('Stale failure.'));
      viewers[0].onError();
      await new Promise(resolve => setImmediate(resolve));
      assert.ok(!component.state.failed);
      viewers[1].reject(new Error('PDF could not be loaded.'));
      await new Promise(resolve => setImmediate(resolve));
      assert.ok(component.state.failed);
      assert.ok(viewers[1].disposed);
      component.onRetry();
      await new Promise(resolve => setImmediate(resolve));
      assert.ok(!component.state.failed);
      assert.equal(resources.length, 3);
      viewers[2].resolve();
      await new Promise(resolve => setImmediate(resolve));
      component.componentWillUnmount();
      assert.ok(viewers[2].disposed);
      viewers[2].onError();
      assert.ok(!component.state.failed);
    } finally {
      component.componentWillUnmount();
      for(const [name, descriptor] of [
          ['document', originalDocument], ['window', originalWindow]] as const) {
        if(descriptor) {
          Object.defineProperty(globalThis, name, descriptor);
        } else {
          delete (globalThis as any)[name];
        }
      }
    }
  });
});
