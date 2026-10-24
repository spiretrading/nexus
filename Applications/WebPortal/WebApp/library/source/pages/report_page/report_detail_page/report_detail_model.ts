/** Loads the metadata and available text content of generated reports. */
export abstract class ReportDetailModel {

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Retrieves the identified generated report. */
  public abstract loadReport(id: string): Promise<ReportDetailModel.Report>;
}

export namespace ReportDetailModel {

  /** The state of the report content. */
  export enum Status {

    /** The report job is queued or running. */
    IN_PROGRESS,

    /** Report generation failed. */
    FAILED,

    /** The report is ready to view or download. */
    READY
  }

  /** A report parameter and its display value. */
  export interface Parameter {

    /** The parameter label. */
    label: string;

    /** The formatted parameter value. */
    value: string;
  }

  /** A report job and any available content. */
  export interface Report {

    /** The report's unique identifier. */
    id: string;

    /** The report heading. */
    title: string;

    /** The report parameters in display order. */
    parameters: readonly Parameter[];

    /** The state of the report content. */
    status: Status;

    /** The download URL, empty until ready. Managed by the provider. */
    filePath: string;

    /** Optional text content for a viewer. Omitted for download-only files. */
    content?: string;
  }
}
