import * as Beam from 'beam';

/** Downloads a generated report using its authenticated file endpoint. */
export async function downloadReport(id: string): Promise<void> {
  const response = await fetch(
    '/api/reporting_service/download_report?id=' + encodeURIComponent(id),
    {credentials: 'same-origin'});
  if(!response.ok) {
    throw new Beam.ServiceError(
      response.statusText || `HTTP ${response.status}`, response.status);
  }
  const disposition = response.headers.get('Content-Disposition') ?? '';
  const encoded = /filename\*=UTF-8''([^;]+)/i.exec(disposition);
  const name = (() => {
    if(encoded) {
      return decodeURIComponent(encoded[1]);
    }
    return id;
  })();
  const content = await response.blob();
  const anchor = document.createElement('a');
  const url = URL.createObjectURL(content);
  try {
    anchor.href = url;
    anchor.download = name;
    anchor.hidden = true;
    document.body.appendChild(anchor);
    anchor.click();
  } finally {
    anchor.remove();
    const RELEASE_DELAY = 60 * 1000;
    window.setTimeout(() => URL.revokeObjectURL(url), RELEASE_DELAY);
  }
}
