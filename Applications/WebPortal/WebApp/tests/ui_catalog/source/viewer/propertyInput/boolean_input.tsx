import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: boolean;

  /** The callback to update the value. */
  update?: (newValue: any) => void;
}

/** A boolean input field. */
export class BooleanInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <input type='checkbox' checked={this.props.value}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(event.target.checked);
  }
}
