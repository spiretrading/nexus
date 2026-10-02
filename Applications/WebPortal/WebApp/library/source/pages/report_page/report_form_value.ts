import * as Beam from 'beam';
import { Interval } from '../../models';
import { ReportDefinition } from './report_definition';
import { ReportFormTemplate } from './report_form_template';
import { parseReportParameterValue, reportParameterValueToJson,
  ReportParameterValue } from './report_parameter_value';

/** Copies a form configuration using its report's parameter types. */
export function copyReportFormValue(value: ReportFormTemplate.Value,
    definitions: readonly ReportDefinition[]): ReportFormTemplate.Value {
  const definition = definitions.find(report =>
    report.id === value.reportType);
  const parameters: Record<string, ReportParameterValue> = {};
  for(const parameter of definition.parameters) {
    parameters[parameter.name] = parseReportParameterValue(parameter.type,
      reportParameterValueToJson(value.parameters[parameter.name]));
  }
  return {...value, parameters, recipients: [...value.recipients],
    scheduleDateTime: (() => {
      if(value.scheduleDateTime) {
        return Beam.DateTime.fromJson(value.scheduleDateTime.toJson());
      }
      return null;
    })(), repeatInterval: value.repeatInterval &&
      new Interval(value.repeatInterval.count, value.repeatInterval.unit)};
}
