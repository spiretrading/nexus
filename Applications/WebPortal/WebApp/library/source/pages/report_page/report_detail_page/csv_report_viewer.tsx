import type { ReportViewer } from './report_viewer';
import { ReportViewerComponent } from './report_viewer_component';
import { ReportViewerRuntimeLoader } from './report_viewer_runtime_loader';

/** Displays a CSV report using a separately loaded data grid. */
export class CsvReportViewer extends ReportViewerComponent {
  constructor(props: ReportViewerComponent.Properties) {
    super(props, CsvReportViewer.loader, 'spire-csv-report');
  }

  private static loader = new ReportViewerRuntimeLoader(
    'resources/csv_report_viewer/viewer.js', [],
    () => (window as Window & {
      spireCsvViewerReady?: Promise<ReportViewer.Runtime>;
    }).spireCsvViewerReady);
}
