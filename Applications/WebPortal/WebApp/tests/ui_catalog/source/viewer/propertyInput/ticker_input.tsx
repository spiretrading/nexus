import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Nexus.Ticker;

  /** The callback to update the value. */
  update?: (newValue: Nexus.Ticker) => void;
}

/** A text input that converts between string and Nexus.Ticker. */
export class TickerInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <input value={this.props.value?.toString() ?? ''}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const parsed = Nexus.Ticker.parse(event.target.value.toUpperCase());
    if(!parsed.equals(Nexus.Ticker.NONE)) {
      this.props.update(parsed);
    }
  }
}
