import type { ReportViewer } from './report_viewer';
import { ReportViewerComponent } from './report_viewer_component';
import { ReportViewerRuntimeLoader } from './report_viewer_runtime_loader';

/** Displays a JSON report with indented, lossless number formatting. */
export class JsonReportViewer extends ReportViewerComponent {
  constructor(props: ReportViewerComponent.Properties) {
    super(props, JsonReportViewer.loader, 'spire-json-report');
  }

  private static loader = new ReportViewerRuntimeLoader(
    'resources/json_report_viewer/viewer.js', [],
    () => (window as Window & {
      spireJsonViewerReady?: Promise<ReportViewer.Runtime>;
    }).spireJsonViewerReady);
}
