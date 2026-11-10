import * as Beam from 'beam';
import * as React from 'react';
import { YearMonthInput } from 'web_portal';

interface Properties {

  /** The selected year and month. */
  value?: Beam.Date;

  /** Reports changes to the property. */
  update?: (value: Beam.Date) => void;
}

/** Edits a year and month property. */
export function YearMonthValueInput({value, update}: Properties): JSX.Element {
  return <YearMonthInput value={value} onChange={update}/>;
}
