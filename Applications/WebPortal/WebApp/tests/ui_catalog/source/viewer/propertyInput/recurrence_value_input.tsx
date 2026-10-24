import * as React from 'react';
import { Recurrence, RecurrenceInput } from 'web_portal';

interface Properties {

  /** The recurrence shown in the property editor. */
  value: Recurrence;

  /** Updates the catalog's recurrence property. */
  update?: (value: Recurrence) => void;
}

/** Edits a recurrence property. */
export function RecurrenceValueInput(props: Properties): JSX.Element {
  return <RecurrenceInput value={props.value} onChange={props.update}/>;
}
