import type { ReportViewer } from './report_viewer';
import { ReportViewerComponent } from './report_viewer_component';
import { ReportViewerRuntimeLoader } from './report_viewer_runtime_loader';

/** Displays a plain text report while preserving its whitespace. */
export class TextReportViewer extends ReportViewerComponent {
  constructor(props: ReportViewerComponent.Properties) {
    super(props, TextReportViewer.loader, 'spire-text-report');
  }

  private static loader = new ReportViewerRuntimeLoader(
    'resources/text_report_viewer/viewer.js', [],
    () => (window as Window & {
      spireTextViewerReady?: Promise<ReportViewer.Runtime>;
    }).spireTextViewerReady);
}
