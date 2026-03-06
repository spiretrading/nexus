import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: string;

  /** The callback to update the value. */
  update?: (newValue: any) => void;
}

/** A number input field for positive numbers. */
export class NumberInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <input type="number" value={this.props.value}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(event.target.valueAsNumber);
  }
}
