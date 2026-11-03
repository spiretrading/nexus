import * as React from 'react';
import { DateExpression, DateExpressionInput } from 'web_portal';

interface Properties {

  /** The date expression shown in the property editor. */
  value: DateExpression;

  /** Updates the catalog's date expression property. */
  update?: (value: DateExpression) => void;
}

/** Edits a date expression property. */
export function DateExpressionValueInput(props: Properties): JSX.Element {
  return <DateExpressionInput aria-label='Date expression property'
    value={props.value} onInput={props.update} onChange={props.update}/>;
}
