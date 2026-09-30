import * as React from 'react';
import { Select } from '../../components';

interface Properties extends Omit<React.ComponentProps<typeof Select>,
    'children' | 'value' | 'defaultValue' | 'multiple'> {

  /** The available report types, in display order. */
  reportTypes: readonly string[];

  /** The selected report type. */
  value?: string;
}

/** Selects a report type from the types supplied by the caller. */
export function ReportTypeSelect(props: Properties): JSX.Element {
  const {reportTypes, ...attributes} = props;
  return <Select {...attributes}>
    {reportTypes.map((reportType, index) =>
      <option key={index} value={reportType}>{reportType}</option>)}
  </Select>;
}
