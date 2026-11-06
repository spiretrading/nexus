import * as React from 'react';
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
  return null;
}
