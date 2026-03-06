import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Nexus.Money;

  /** The callback to update the value. */
  update?: (newValue: Nexus.Money) => void;
}

/** A text input that converts between string and Nexus.Money. */
export class MoneyInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <input value={this.props.value?.toString() ?? ''}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const parsed = Nexus.Money.parse(event.target.value);
    if(parsed !== null) {
      this.props.update(parsed);
    }
  }
}
