/** Describes an input parameter for a report type. */
export class ReportParameterDefinition {

  /** The parameter identifier used in submitted values. */
  public readonly name: string;

  /** The label displayed beside the input. */
  public readonly label: string;

  /** The shared type name, such as DirectoryEntry or DateRange. */
  public readonly type: string;

  /** Whether a value is required to generate the report. */
  public readonly required: boolean;

  /** The default in the shared type's JSON representation.
   * Undefined means no default; false, zero, and null are preserved.
   */
  public readonly defaultValue: unknown;

  /** Parses a parameter definition from JSON. */
  public static fromJson(value: any): ReportParameterDefinition {
    return new ReportParameterDefinition(value.name, value.label, value.type,
      value.required ?? false, value.default);
  }

  /** Constructs a parameter definition.
   * @param name - The parameter identifier.
   * @param label - The input label.
   * @param type - The shared type name.
   * @param required - Whether a value is required.
   * @param defaultValue - The default JSON value, or undefined for no default.
   */
  constructor(name: string, label: string, type: string, required: boolean,
      defaultValue?: unknown) {
    this.name = name;
    this.label = label;
    this.type = type;
    this.required = required;
    this.defaultValue = defaultValue;
  }

  /** Converts this definition to JSON. */
  public toJson(): any {
    const value: any = {name: this.name, label: this.label, type: this.type,
      required: this.required};
    if(this.defaultValue !== undefined) {
      value.default = this.defaultValue;
    }
    return value;
  }
}
