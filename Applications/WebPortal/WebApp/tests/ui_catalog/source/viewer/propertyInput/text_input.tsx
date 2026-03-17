import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: string;

  /** The callback to update the value. */
  update?: (newValue: string) => void;
}

/** A text input field. */
export class TextInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <input value={this.props.value} onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(event.target.value.toString());
  }
}
