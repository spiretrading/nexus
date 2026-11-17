import type { ReportViewer } from './report_viewer';
import { ReportViewerComponent } from './report_viewer_component';
import { ReportViewerRuntimeLoader } from './report_viewer_runtime_loader';

/** Displays PDF pages using a separately loaded PDF.js renderer. */
export class PdfReportViewer extends ReportViewerComponent {
  constructor(props: ReportViewerComponent.Properties) {
    super(props, PdfReportViewer.loader, 'spire-pdf-report');
  }

  private static loader = new ReportViewerRuntimeLoader(
    'resources/report_viewer/viewer.js', ['resources/pdfjs/web/pdf_viewer.css',
      'resources/report_viewer/viewer.css'],
    () => (window as Window & {
      spirePdfViewerReady?: Promise<ReportViewer.Runtime>;
    }).spirePdfViewerReady);
}
