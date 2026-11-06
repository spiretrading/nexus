import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { CsvReportViewer, JsonReportViewer, MarkdownReportViewer,
  PdfReportViewer, ReportDetailModel, ReportViewer, TextReportViewer } from
  '../../source/pages/report_page/report_detail_page';
import { ReportViewerComponent } from
  '../../source/pages/report_page/report_detail_page/report_viewer_component';

describe('ReportViewer', () => {
  const report: ReportDetailModel.Report = {
    id: 'report',
    title: 'Account Statement',
    parameters: [],
    mediaType: 'application/pdf',
    status: ReportDetailModel.Status.READY,
    filePath: '/api/report?id=1&name=statement.pdf'
  };

  it('selects viewers by media type rather than filename', () => {
    for(const mediaType of [
        'application/pdf', 'APPLICATION/PDF; version=1.7']) {
      const viewer = ReportViewer({report: {...report, mediaType}});
      assert.equal(viewer.type, PdfReportViewer);
      assert.equal(viewer.props.filePath, report.filePath);
      assert.equal(viewer.props.title, report.title);
    }
    for(const mediaType of ['text/csv', 'TEXT/CSV; charset=utf-8']) {
      const viewer = ReportViewer({report: {...report, mediaType}});
      assert.equal(viewer.type, CsvReportViewer);
      assert.equal(viewer.props.filePath, report.filePath);
      assert.equal(viewer.props.title, report.title);
    }
    for(const [Viewer, types] of [
        [MarkdownReportViewer,
          ['text/markdown', 'TEXT/MARKDOWN; charset=utf-8']],
        [JsonReportViewer, ['application/json',
          'APPLICATION/JSON; charset=utf-8', 'application/vnd.spire+json',
          'application/problem+json']],
        [TextReportViewer,
          ['text/plain', 'TEXT/PLAIN; charset=utf-8']]] as const) {
      for(const mediaType of types) {
        const viewer = ReportViewer({report: {...report, mediaType}});
        assert.equal(viewer.type, Viewer);
        assert.equal(viewer.props.filePath, report.filePath);
        assert.equal(viewer.props.title, report.title);
      }
    }
    for(const mediaType of ['text/html', 'application/octet-stream', '',
        'application/not-json']) {
      assert.equal(ReportViewer({report: {...report, mediaType}}), null);
    }
    for(const status of [ReportDetailModel.Status.IN_PROGRESS,
        ReportDetailModel.Status.FAILED]) {
      assert.equal(ReportViewer({report: {...report, status}}), null);
    }
    assert.equal(ReportViewer({report: {...report, filePath: ''}}), null);
  });

  it('shares pending and successful runtime loads', async () => {
    const originalDocument =
      Object.getOwnPropertyDescriptor(globalThis, 'document');
    const resources: any[] = [];
    const runtime: ReportViewer.Runtime = {create: () => null};
    let resolveRuntime: (runtime: ReportViewer.Runtime) => void;
    const ready = new Promise<ReportViewer.Runtime>(resolve => {
      resolveRuntime = resolve;
    });
    let requests = 0;
    const loader = new ReportViewer.RuntimeLoader('viewer.js',
      ['first.css', 'second.css'], () => {
        ++requests;
        return ready;
      });
    Object.defineProperty(globalThis, 'document', {configurable: true,
      value: {
        baseURI: 'https://portal.example/',
        createElement: (tag: string) => ({tag, remove() {}}),
        head: {appendChild(element: any) {resources.push(element);}}
      }});
    try {
      assert.equal(resources.length, 0);
      const pending = loader.load();
      assert.equal(loader.load(), pending);
      assert.equal(resources.length, 3);
      assert.equal(resources[0].href, 'first.css');
      assert.equal(resources[1].href, 'second.css');
      assert.equal(resources[2].tag, 'script');
      assert.notEqual(resources[2].type, 'module');
      resources[2].onload();
      resources[0].onload();
      await new Promise(resolve => setImmediate(resolve));
      assert.equal(requests, 0);
      resources[1].onload();
      await new Promise(resolve => setImmediate(resolve));
      assert.equal(requests, 1);
      let settled = false;
      pending.then(() => {settled = true;});
      await new Promise(resolve => setImmediate(resolve));
      assert.ok(!settled);
      resolveRuntime(runtime);
      assert.equal(await pending, runtime);
      assert.equal(loader.load(), pending);
      assert.equal(resources.length, 3);
    } finally {
      if(originalDocument) {
        Object.defineProperty(globalThis, 'document', originalDocument);
      } else {
        delete (globalThis as any).document;
      }
    }
  });

  for(const failure of ['script', 'style', 'missing', 'throw', 'reject']) {
    it(`retries runtime loading after ${failure} failure`, async () => {
      const originalDocument =
        Object.getOwnPropertyDescriptor(globalThis, 'document');
      const resources: any[] = [];
      const runtime: ReportViewer.Runtime = {create: () => null};
      let failing = true;
      const loader = new ReportViewer.RuntimeLoader('viewer.js?v=1#viewer',
        ['viewer.css'], () => {
          if(failing) {
            if(failure === 'missing') {
              return undefined;
            }
            if(failure === 'throw') {
              throw new Error('Initialization failed.');
            }
            if(failure === 'reject') {
              return Promise.reject(new Error('Initialization failed.'));
            }
          }
          return Promise.resolve(runtime);
        });
      Object.defineProperty(globalThis, 'document', {configurable: true,
        value: {
          baseURI: 'file:///demo/index.html',
          createElement: (tag: string) => ({tag, removed: false,
            remove() {this.removed = true;}}),
          head: {appendChild(element: any) {
            resources.push(element);
            queueMicrotask(() => {
              if(failing && (failure === 'script' && element.tag === 'script' ||
                  failure === 'style' && element.tag === 'link')) {
                element.onerror();
              } else {
                element.onload();
              }
            });
          }}
        }});
      try {
        const pending = loader.load();
        assert.equal(loader.load(), pending);
        await assert.rejects(pending);
        assert.equal(resources.length, 2);
        assert.ok(resources.every(element => element.removed));
        const first = new URL(resources[1].src);
        assert.equal(first.searchParams.get('attempt'), '0');
        assert.equal(first.searchParams.get('v'), '1');
        assert.equal(first.hash, '#viewer');
        failing = false;
        const retry = loader.load();
        assert.notEqual(retry, pending);
        assert.equal(await retry, runtime);
        assert.equal(resources.length, 4);
        assert.equal(
          new URL(resources[3].src).searchParams.get('attempt'), '1');
        assert.ok(!resources[2].removed);
        assert.ok(!resources[3].removed);
        assert.equal(loader.load(), retry);
      } finally {
        if(originalDocument) {
          Object.defineProperty(globalThis, 'document', originalDocument);
        } else {
          delete (globalThis as any).document;
        }
      }
    });
  }

  for(const action of ['replace', 'clear', 'unmount']) {
    it(`ignores pending runtime loads after ${action}`, async () => {
      class TestViewer extends ReportViewerComponent {}
      const originalDocument =
        Object.getOwnPropertyDescriptor(globalThis, 'document');
      let resolveRuntime: (runtime: ReportViewer.Runtime) => void;
      const ready = new Promise<ReportViewer.Runtime>(resolve => {
        resolveRuntime = resolve;
      });
      const loader = new ReportViewer.RuntimeLoader('viewer.js', [],
        () => ready);
      const files: string[] = [];
      let disposals = 0;
      const runtime: ReportViewer.Runtime = {
        create(container, file) {
          files.push(file);
          return {ready: Promise.resolve(), dispose() {++disposals;}};
        }
      };
      Object.defineProperty(globalThis, 'document', {configurable: true,
        value: {
          baseURI: 'https://portal.example/',
          createElement: () => ({remove() {}}),
          head: {appendChild(element: any) {
            queueMicrotask(() => element.onload());
          }}
        }});
      const component = new TestViewer({filePath: 'first', title: 'Report'},
        loader, 'test-viewer') as any;
      component.container.current = {};
      component.setState = (state: {failed: boolean},
          callback?: () => void) => {
        component.state = {...component.state, ...state};
        callback?.();
      };
      try {
        component.componentDidMount();
        await new Promise(resolve => setImmediate(resolve));
        assert.deepEqual(files, []);
        if(action === 'unmount') {
          component.componentWillUnmount();
        } else {
          const previous = component.props;
          let filePath = '';
          if(action === 'replace') {
            filePath = 'second';
          }
          component.props = {...previous, filePath};
          component.componentDidUpdate(previous);
        }
        resolveRuntime(runtime);
        await new Promise(resolve => setImmediate(resolve));
        assert.ok(!component.state.failed);
        if(action === 'replace') {
          assert.deepEqual(files, ['https://portal.example/second']);
          assert.equal(component.render().props.children.props.className,
            'test-viewer');
          component.componentWillUnmount();
          assert.equal(disposals, 1);
        } else {
          assert.deepEqual(files, []);
        }
      } finally {
        component.componentWillUnmount();
        if(originalDocument) {
          Object.defineProperty(globalThis, 'document', originalDocument);
        } else {
          delete (globalThis as any).document;
        }
      }
    });
  }

  for(const [Viewer, resourceCount, runtimeName] of [
      [PdfReportViewer, 3, 'spirePdfViewerReady'],
      [CsvReportViewer, 1, 'spireCsvViewerReady'],
      [JsonReportViewer, 1, 'spireJsonViewerReady'],
      [MarkdownReportViewer, 2, 'spireMarkdownViewerReady'],
      [TextReportViewer, 1, 'spireTextViewerReady']] as const) {
    it(`${Viewer.name} loads lazily and ignores stale failures`, async () => {
      const originalDocument =
        Object.getOwnPropertyDescriptor(globalThis, 'document');
      const originalWindow =
        Object.getOwnPropertyDescriptor(globalThis, 'window');
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
        value: {[runtimeName]: Promise.resolve(runtime)}});
      const component = new Viewer({filePath: 'api/report?id=1&name=a',
        title: report.title}) as any;
      component.container.current = {};
      component.setState = (state: {failed: boolean},
          callback?: () => void) => {
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
        assert.equal(resources.length, resourceCount);
        assert.notEqual(resources[resourceCount - 1].type, 'module');
        assert.equal(viewers[0].file,
          'https://portal.example/api/report?id=1&name=a');
        const previous = component.props;
        component.props = {...previous,
          filePath: 'blob:https://portal.example/2'};
        component.componentDidUpdate(previous);
        await new Promise(resolve => setImmediate(resolve));
        assert.ok(viewers[0].disposed);
        assert.equal(viewers[1].file, 'blob:https://portal.example/2');
        viewers[0].reject(new Error('Stale failure.'));
        viewers[0].onError();
        await new Promise(resolve => setImmediate(resolve));
        assert.ok(!component.state.failed);
        viewers[1].reject(new Error('Report could not be loaded.'));
        await new Promise(resolve => setImmediate(resolve));
        assert.ok(component.state.failed);
        assert.ok(viewers[1].disposed);
        component.onRetry();
        await new Promise(resolve => setImmediate(resolve));
        assert.ok(!component.state.failed);
        assert.equal(resources.length, resourceCount);
        viewers[2].resolve();
        await new Promise(resolve => setImmediate(resolve));
        component.componentWillUnmount();
        assert.ok(viewers[2].disposed);
        viewers[2].onError();
        assert.ok(!component.state.failed);
      } finally {
        component.componentWillUnmount();
        const globals = [['document', originalDocument],
          ['window', originalWindow]] as const;
        for(const [name, descriptor] of globals) {
          if(descriptor) {
            Object.defineProperty(globalThis, name, descriptor);
          } else {
            delete (globalThis as any)[name];
          }
        }
      }
    });
  }
});
