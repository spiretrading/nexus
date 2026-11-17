import type { ReportViewer } from './report_viewer';
import { ReportViewerComponent } from './report_viewer_component';
import { ReportViewerRuntimeLoader } from './report_viewer_runtime_loader';

/** Displays a Markdown report using a separately loaded renderer. */
export class MarkdownReportViewer extends ReportViewerComponent {
  constructor(props: ReportViewerComponent.Properties) {
    super(props, MarkdownReportViewer.loader, 'spire-markdown-report');
  }

  private static loader = new ReportViewerRuntimeLoader(
    'resources/markdown_report_viewer/viewer.js',
    ['resources/markdown_report_viewer/viewer.css'],
    () => (window as Window & {
      spireMarkdownViewerReady?: Promise<ReportViewer.Runtime>;
    }).spireMarkdownViewerReady);
}
