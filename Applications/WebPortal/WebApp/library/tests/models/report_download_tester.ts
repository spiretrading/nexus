import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import { HttpGeneratedReportsModel } from '../../source/pages/report_page';

describe('HTTP report downloads', () => {
  const originalFetch = globalThis.fetch;
  const originalDocument = globalThis.document;
  const originalWindow = globalThis.window;
  const create = URL.createObjectURL;
  const revoke = URL.revokeObjectURL;
  let requests: string[];
  let clicks: {href: string; download: string}[];
  let blobs: Blob[];
  let releases: (() => void)[];
  let revoked: string[];
  beforeEach(() => {
    requests = [];
    clicks = [];
    blobs = [];
    releases = [];
    revoked = [];
    globalThis.fetch = async (input, init) => {
      assert.equal(init.credentials, 'same-origin');
      requests.push(String(input));
      return new Response(new Uint8Array([65, 0, 66]), {headers: {
        'Content-Type': 'application/octet-stream',
        'Content-Disposition': "attachment; filename*=UTF-8''caf%C3%A9-job.bin"
      }});
    };
    URL.createObjectURL = blob => {
      blobs.push(blob as Blob);
      return `blob:report-${blobs.length}`;
    };
    URL.revokeObjectURL = url => { revoked.push(url); };
    globalThis.document = {body: {appendChild: () => {}},
      createElement: () => {
        const anchor = {href: '', download: '', hidden: false,
          click: () => clicks.push({href: anchor.href,
            download: anchor.download}), remove: () => {}};
        return anchor;
      }} as unknown as Document;
    globalThis.window = {setTimeout: (callback: () => void, delay: number) => {
      assert.equal(delay, 60000);
      releases.push(callback);
      return releases.length;
    }} as unknown as Window & typeof globalThis;
  });
  afterEach(() => {
    globalThis.fetch = originalFetch;
    globalThis.document = originalDocument;
    globalThis.window = originalWindow;
    URL.createObjectURL = create;
    URL.revokeObjectURL = revoke;
  });

  it('downloads_distinct_files_with_binary_bytes_and_filenames', async () => {
    const model = new HttpGeneratedReportsModel(null);
    await model.load();
    await model.download(['a /?', 'b', 'a /?']);
    assert.deepEqual(requests, [
      '/api/reporting_service/download_report?id=a%20%2F%3F',
      '/api/reporting_service/download_report?id=b']);
    assert.equal(clicks.length, 2);
    assert.equal(clicks[0].download, 'caf\u00e9-job.bin');
    assert.deepEqual(Array.from(new Uint8Array(await blobs[0].arrayBuffer())),
      [65, 0, 66]);
    assert.equal(revoked.length, 0);
    releases.forEach(release => release());
    assert.deepEqual(revoked, ['blob:report-1', 'blob:report-2']);
  });

  it('rejects_failed_downloads_without_creating_files', async () => {
    globalThis.fetch = async () => new Response(null, {status: 403});
    const model = new HttpGeneratedReportsModel(null);
    await model.load();
    await assert.rejects(model.download(['job']),
      (error: any) => error.code === 403);
    assert.equal(clicks.length, 0);
    assert.equal(blobs.length, 0);
  });
});
