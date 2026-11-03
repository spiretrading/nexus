import { ReportOutputDefinition } from './report_output_definition';
import { ReportParameterDefinition } from './report_parameter_definition';

/** Describes a report type available to the current user. */
export class ReportDefinition {

  /** The stable report type identifier used when requesting a report. */
  public readonly id: string;

  /** The report type's displayed name. */
  public readonly name: string;

  /** The description of the report type. */
  public readonly description: string;

  /** The report's parameters, in display order. */
  public readonly parameters: readonly ReportParameterDefinition[];

  /** The format of the generated file. */
  public readonly output: ReportOutputDefinition;

  /** Parses a report definition from the server's JSON response. */
  public static fromJson(value: any): ReportDefinition {
    return new ReportDefinition(value.id, value.name, value.description ?? '',
      value.parameters.map(ReportParameterDefinition.fromJson),
      ReportOutputDefinition.fromJson(value.output));
  }

  /** Constructs a report definition.
   * @param id - The stable report type identifier.
   * @param name - The displayed report name.
   * @param description - The report description.
   * @param parameters - The parameters, in display order.
   * @param output - The generated file's format.
   */
  constructor(id: string, name: string, description: string,
      parameters: readonly ReportParameterDefinition[],
      output: ReportOutputDefinition) {
    this.id = id;
    this.name = name;
    this.description = description;
    this.parameters = parameters.slice();
    this.output = output;
  }

  /** Converts this definition to JSON. */
  public toJson(): any {
    return {id: this.id, name: this.name, description: this.description,
      parameters: this.parameters.map(parameter => parameter.toJson()),
      output: this.output.toJson()};
  }
}
