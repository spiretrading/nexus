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

  it('resolves report URLs against the host and encodes the viewer query',
      () => {
    const original = Object.getOwnPropertyDescriptor(globalThis, 'document');
    Object.defineProperty(globalThis, 'document', {configurable: true,
      value: {baseURI: 'https://portal.example/'}});
    try {
      const filePath = 'api/report?id=a&name=Monthly%20Statement.pdf#page=2';
      const frame = new PdfReportViewer({filePath, title: report.title}).
        render().props.children;
      const source = new URL(frame.props.src, document.baseURI);
      assert.equal(source.pathname, '/resources/pdfjs/web/viewer.html');
      const expected = 'https://portal.example/' + filePath;
      assert.equal(source.searchParams.get('file'), expected);
      assert.equal(source.hash, '');
      assert.equal(frame.key, expected + ':0');
      assert.equal(frame.props.title, report.title);
      const blob = 'blob:https://portal.example/report';
      const next = new PdfReportViewer({filePath: blob, title: report.title}).
        render().props.children;
      assert.equal(new URL(next.props.src, document.baseURI).
        searchParams.get('file'), blob);
      assert.notEqual(next.key, frame.key);
      assert.equal(new PdfReportViewer({filePath: '', title: report.title}).
        render(), null);
    } finally {
      if(original) {
        Object.defineProperty(globalThis, 'document', original);
      } else {
        delete globalThis.document;
      }
    }
  });
});
