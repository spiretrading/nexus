import * as React from 'react';
import { CsvReportViewer } from './csv_report_viewer';
import { PdfReportViewer } from './pdf_report_viewer';
import { ReportDetailModel } from './report_detail_model';

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
  return null;
}

export namespace ReportViewer {

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
