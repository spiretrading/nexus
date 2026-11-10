import * as React from 'react';
import { CsvReportViewer } from './csv_report_viewer';
import { JsonReportViewer } from './json_report_viewer';
import { MarkdownReportViewer } from './markdown_report_viewer';
import { PdfReportViewer } from './pdf_report_viewer';
import { ReportDetailModel } from './report_detail_model';
import * as ViewerRuntime from './report_viewer_runtime_loader';
import { TextReportViewer } from './text_report_viewer';

interface Properties {

  /** The report whose file is to be displayed. */
  report: ReportDetailModel.Report;
}

/** Selects a viewer for supported report formats. */
export function ReportViewer(props: Properties): JSX.Element {
  const report = props.report;
  if(report.status !== ReportDetailModel.Status.READY || !report.filePath) {
    return null;
  }
  const mediaType = report.mediaType.split(';', 1)[0].trim().toLowerCase();
  if(mediaType === 'application/pdf') {
    return <PdfReportViewer filePath={report.filePath} title={report.title}/>;
  }
  if(mediaType === 'text/csv') {
    return <CsvReportViewer filePath={report.filePath} title={report.title}/>;
  }
  if(mediaType === 'text/markdown') {
    return <MarkdownReportViewer filePath={report.filePath}
      title={report.title}/>;
  }
  if(mediaType === 'application/json' ||
      /^application\/[a-z0-9!#$&^_.+-]+\+json$/.test(mediaType)) {
    return <JsonReportViewer filePath={report.filePath} title={report.title}/>;
  }
  if(mediaType === 'text/plain') {
    return <TextReportViewer filePath={report.filePath} title={report.title}/>;
  }
  return null;
}

export namespace ReportViewer {

  /** Loads and caches a viewer's script, stylesheets, and runtime. */
  export import RuntimeLoader = ViewerRuntime.ReportViewerRuntimeLoader;

  /** A mounted report viewer. */
  export interface Instance {

    /** Resolves when initialized, or rejects if loading fails. */
    ready: Promise<void>;

    /** Cancels pending work and releases the viewer's resources. */
    dispose(): void;
  }

  /** Creates viewers for a supported report format. */
  export interface Runtime {

    /**
     * Mounts a viewer and begins loading its report.
     * @param container The element in which to display the report.
     * @param file The URL of the report file.
     * @param onError Receives failures occurring after initialization.
     * @return The mounted viewer and its initialization promise.
     */
    create(container: HTMLElement, file: string, onError: () => void): Instance;
  }
}
