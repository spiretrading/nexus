import * as React from 'react';
import { DateExpression, DateExpressionInput } from 'web_portal';

interface Properties {

  /** The date expression shown in the property editor. */
  value: DateExpression;

  /** Updates the catalog's date expression property. */
  update?: (value: DateExpression) => void;
}

/** Edits a date expression property. */
export class DateExpressionValueInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <DateExpressionInput aria-label='Date expression property'
      value={this.props.value} onInput={this.props.update}
      onChange={this.props.update}/>;
  }
}
