/** Loads the metadata and available text content of generated reports. */
export abstract class ReportDetailModel {

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Retrieves the identified generated report. */
  public abstract loadReport(id: string): Promise<ReportDetailModel.Report>;
}

export namespace ReportDetailModel {

  /** A report parameter and its display value. */
  export interface Parameter {

    /** The parameter label. */
    label: string;

    /** The formatted parameter value. */
    value: string;
  }

  /** A generated report available for viewing or downloading. */
  export interface Report {

    /** The report's unique identifier. */
    id: string;

    /** The report heading. */
    title: string;

    /** The report parameters in display order. */
    parameters: readonly Parameter[];

    /** The download URL. Its lifetime is managed by the provider. */
    filePath: string;

    /** Optional text content for a viewer. Omitted for download-only files. */
    content?: string;
  }
}
