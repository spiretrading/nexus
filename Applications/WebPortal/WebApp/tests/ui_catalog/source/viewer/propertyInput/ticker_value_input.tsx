import * as Nexus from 'nexus';
import * as React from 'react';
import { LocalTickerQueryModel, TickerInput } from 'web_portal';

interface Properties {

  /** The value of the field. */
  value?: Nexus.Ticker;

  /** The callback to update the value. */
  update?: (newValue: Nexus.Ticker) => void;
}

/** Edits a ticker-valued catalog property. */
export class TickerValueInput extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.model = new LocalTickerQueryModel([]);
  }

  public render(): JSX.Element {
    return <TickerInput value={this.props.value} model={this.model}
      aria-label='Ticker value' onChange={this.props.update}/>;
  }

  private model: LocalTickerQueryModel;
}
