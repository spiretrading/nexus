/** Describes the file produced by a report type. */
export class ReportOutputDefinition {

  /** The output's media type, such as text/csv or application/zip. */
  public readonly mediaType: string;

  /** The output's file extension without a leading dot. */
  public readonly extension: string;

  /** Parses an output definition from JSON. */
  public static fromJson(value: any): ReportOutputDefinition {
    return new ReportOutputDefinition(value.media_type, value.extension);
  }

  /** Constructs an output definition.
   * @param mediaType - The output's media type.
   * @param extension - The file extension without a leading dot.
   */
  constructor(mediaType: string, extension: string) {
    this.mediaType = mediaType;
    this.extension = extension;
  }

  /** Converts this definition to JSON. */
  public toJson(): any {
    return {media_type: this.mediaType, extension: this.extension};
  }
}
