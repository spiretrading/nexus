import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { DateRange, validateDateRange, ValidationError } from '../../models';
import { ReportParameterDefinition } from './report_parameter_definition';

/** A value accepted by a report parameter. Null represents an omitted value. */
export type ReportParameterValue = Beam.DirectoryEntry |
  readonly Beam.DirectoryEntry[] | DateRange | Beam.Date | Beam.DateTime |
  Beam.Duration | Nexus.Scope | Nexus.Money | Nexus.Currency | number;

/** Parses a parameter's JSON value using its shared type name. */
export function parseReportParameterValue(type: string, value: any):
    ReportParameterValue {
  if(value == null) {
    return null;
  }
  switch(type) {
    case 'DirectoryEntry':
      return Beam.DirectoryEntry.fromJson(value);
    case 'DirectoryEntryList':
      return value.map(Beam.DirectoryEntry.fromJson);
    case 'DateRange':
      return new DateRange(parseDate(value.start), parseDate(value.end));
    case 'Date':
      return parseDate(value);
    case 'DateTime':
      return Beam.DateTime.fromJson(value);
    case 'Time':
      return Beam.Duration.fromJson(value);
    case 'Scope':
      if(value === '*') {
        return Nexus.Scope.GLOBAL.clone();
      }
      return Nexus.Scope.fromJson(value);
    case 'Decimal':
    case 'Integer':
      return value;
    case 'Money':
      if(typeof value === 'string') {
        return Nexus.Money.parse(value);
      }
      return Nexus.Money.fromJson(value);
    case 'Currency':
      if(typeof value === 'string') {
        return Nexus.buildCurrencyDatabase().fromCode(value).currency;
      }
      return Nexus.Currency.fromJson(value);
    default:
      throw new Error(`Unsupported report parameter type: ${type}`);
  }
}

/** Converts a parameter value to its shared type's JSON representation. */
export function reportParameterValueToJson(value: ReportParameterValue): any {
  if(value == null || typeof value === 'number') {
    return value;
  } else if(value instanceof DateRange) {
    return {start: value.start?.toJson() ?? null,
      end: value.end?.toJson() ?? null};
  } else if(Array.isArray(value)) {
    return value.map(entry => entry.toJson());
  }
  return (value as Exclude<ReportParameterValue,
    readonly Beam.DirectoryEntry[] | DateRange | number>).toJson();
}

/** Validates a report parameter's committed value. */
export function validateReportParameter(definition: ReportParameterDefinition,
    value: ReportParameterValue): ValidationError {
  const missing = value == null || Array.isArray(value) && value.length === 0 ||
    definition.type === 'Scope' && isEmptyScope(value as Nexus.Scope) ||
    definition.type === 'DateRange' && value != null &&
      !(value as DateRange).start && !(value as DateRange).end;
  if(missing) {
    if(definition.required) {
      return ValidationError.REQUIRED;
    }
    return ValidationError.NONE;
  }
  switch(definition.type) {
    case 'DirectoryEntry':
      if(!isDirectoryEntry(value)) {
        return ValidationError.FORMAT;
      }
      break;
    case 'DirectoryEntryList':
      if(!Array.isArray(value) ||
          value.some(entry => !isDirectoryEntry(entry))) {
        return ValidationError.FORMAT;
      }
      if(new Set(value.map(entry => entry.id)).size !== value.length) {
        return ValidationError.DUPLICATE;
      }
      break;
    case 'DateRange': {
      const range = value as DateRange;
      if(typeof range !== 'object' || !isValidDate(range.start) ||
          !isValidDate(range.end)) {
        return ValidationError.FORMAT;
      }
      if(definition.required && (!range.start || !range.end)) {
        return ValidationError.REQUIRED;
      }
      if(range.start && range.end && range.start.compare(range.end) > 0) {
        return ValidationError.OUT_OF_RANGE;
      }
      break;
    }
    case 'Date':
      if(!isValidDate(value as Beam.Date)) {
        return ValidationError.FORMAT;
      }
      break;
    case 'DateTime':
      if(!isValidDate((value as Beam.DateTime).date) ||
          !(value as Beam.DateTime).date ||
          !isTime((value as Beam.DateTime).timeOfDay)) {
        return ValidationError.FORMAT;
      }
      break;
    case 'Time':
      if(!isTime(value as Beam.Duration)) {
        return ValidationError.FORMAT;
      }
      break;
    case 'Decimal':
    case 'Integer':
      if(typeof value !== 'number' || !Number.isFinite(value) ||
          definition.type === 'Integer' && !Number.isInteger(value)) {
        return ValidationError.FORMAT;
      }
      break;
    case 'Money':
      if(typeof (value as Nexus.Money).toJson !== 'function' ||
          !Number.isFinite((value as Nexus.Money).toJson())) {
        return ValidationError.FORMAT;
      }
      break;
    case 'Currency':
      if(!Number.isInteger((value as Nexus.Currency).code) ||
          (value as Nexus.Currency).code === Nexus.Currency.NONE.code) {
        return ValidationError.FORMAT;
      }
      break;
    case 'Scope':
      if(typeof (value as Nexus.Scope).isGlobal !== 'boolean') {
        return ValidationError.FORMAT;
      }
      break;
    default:
      return ValidationError.FORMAT;
  }
  return ValidationError.NONE;
}

function parseDate(value: any): Beam.Date {
  if(value == null) {
    return null;
  }
  return Beam.Date.fromJson(value.replace(/-/g, ''));
}

function isTime(value: Beam.Duration): boolean {
  const DAY = 24 * 60 * 60 * Beam.Duration.TICKS_PER_SECOND;
  return value && Number.isFinite(value.ticks) && value.ticks >= 0 &&
    value.ticks < DAY;
}

function isValidDate(value: Beam.Date): boolean {
  return validateDateRange(new DateRange(value, value), false).valid;
}

function isDirectoryEntry(value: any): boolean {
  return value && (value.type === Beam.DirectoryEntry.Type.ACCOUNT ||
    value.type === Beam.DirectoryEntry.Type.DIRECTORY) &&
    Number.isInteger(value.id) && value.id >= 0;
}

function isEmptyScope(value: Nexus.Scope): boolean {
  return value && !value.isGlobal && value.countries?.size === 0 &&
    value.venues?.size === 0 && value.tickers?.size === 0;
}
