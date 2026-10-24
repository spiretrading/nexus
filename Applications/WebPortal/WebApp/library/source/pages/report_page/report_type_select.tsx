import * as React from 'react';
import { Select } from '../../components';
import { ReportDefinition } from './report_definition';

interface Properties extends Omit<React.ComponentProps<typeof Select>,
    'children' | 'value' | 'defaultValue' | 'multiple'> {

  /** The available report types, in display order. */
  reportTypes: readonly (string | ReportDefinition)[];

  /** The selected report definition's ID, or the selected string option. */
  value?: string;
}

/** Selects a report type from the types supplied by the caller. */
export function ReportTypeSelect(props: Properties): JSX.Element {
  const {reportTypes, ...attributes} = props;
  return <Select {...attributes}>
    {reportTypes.map((reportType, index) => {
      if(typeof reportType === 'string') {
        return <option key={index} value={reportType}>{reportType}</option>;
      }
      return <option key={reportType.id} value={reportType.id}>
        {reportType.name}
      </option>;
    })}
  </Select>;
}
