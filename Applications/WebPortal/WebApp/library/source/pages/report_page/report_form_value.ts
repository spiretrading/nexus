import * as Beam from 'beam';
import { Interval } from '../../models';
import { ReportDefinition } from './report_definition';
import { ReportFormTemplate } from './report_form_template';
import { parseReportParameterValue, reportParameterValueToJson,
  ReportParameterValue } from './report_parameter_value';

/** Encodes typed form values for the reporting service. */
export function reportFormValueToJson(value: ReportFormTemplate.Value): any {
  return {report_type: value.reportType,
    time_zone: Intl.DateTimeFormat().resolvedOptions().timeZone,
    parameters: Object.fromEntries(Object.entries(value.parameters).
      map(([name, parameter]) =>
        [name, reportParameterValueToJson(parameter)])),
    recipients: value.recipients.map(entry => entry.toJson()),
    scheduled: value.scheduled,
    schedule_date_time: (() => {
      if(value.scheduled) {
        return value.scheduleDateTime?.toJson() ?? null;
      }
      return null;
    })(), repeats: value.scheduled && value.repeats,
    repeat_interval: (() => {
      if(value.scheduled && value.repeats && value.repeatInterval) {
        return {count: value.repeatInterval.count,
          unit: value.repeatInterval.unit};
      }
      return null;
    })()};
}

/** Decodes a saved form configuration using its report's parameter types. */
export function parseReportFormValue(value: any,
    definitions: readonly ReportDefinition[]): ReportFormTemplate.Value {
  const definition = definitions.find(report =>
    report.id === value.report_type);
  if(!definition) {
    throw new Error(`Unknown report type: ${value.report_type}`);
  }
  const parameters: Record<string, ReportParameterValue> = {};
  for(const parameter of definition.parameters) {
    parameters[parameter.name] = parseReportParameterValue(parameter.type,
      value.parameters[parameter.name]);
  }
  return {reportType: value.report_type, parameters,
    recipients: value.recipients.map(Beam.DirectoryEntry.fromJson),
    scheduled: value.scheduled,
    scheduleDateTime: (() => {
      if(value.schedule_date_time != null) {
        return Beam.DateTime.fromJson(value.schedule_date_time);
      }
      return null;
    })(), repeats: value.repeats,
    repeatInterval: value.repeat_interval &&
      new Interval(value.repeat_interval.count, value.repeat_interval.unit)};
}

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
