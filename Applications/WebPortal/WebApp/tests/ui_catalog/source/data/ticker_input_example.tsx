import * as Nexus from 'nexus';
import * as React from 'react';
import { LocalTickerQueryModel, TickerInput, TickerQueryModel } from
  'web_portal';

interface Properties {
  value: Nexus.Ticker;
  readOnly: boolean;
  disabled: boolean;
  lookupDelay: number;
  failLookup: boolean;
  onChange: (value: Nexus.Ticker) => void;
}

/** Demonstrates single-ticker selection and asynchronous suggestions. */
export class TickerInputExample extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.model = new ExampleTickerModel(
      () => this.props.lookupDelay, () => this.props.failLookup);
  }

  public render(): JSX.Element {
    return <TickerInput model={this.model} value={this.props.value}
      readOnly={this.props.readOnly} disabled={this.props.disabled}
      aria-label='Ticker' onChange={this.props.onChange}/>;
  }

  private model: TickerQueryModel;
}

class ExampleTickerModel extends TickerQueryModel {
  constructor(delay: () => number, fail: () => boolean) {
    super();
    this.delay = delay;
    this.fail = fail;
    this.model = new LocalTickerQueryModel([
      'ABX.TSX', 'BHP.ASX', 'BMO.TSX', 'BMO.ASX', 'RY.TSX', 'SHOP.TSX',
      'TECK.B.TSX', 'TEST.TSXV', 'BMO.TSX'
    ].map(value => Nexus.Ticker.parse(value)));
  }

  public async submit(query: string): Promise<readonly Nexus.Ticker[]> {
    const fail = this.fail();
    await new Promise(resolve => window.setTimeout(resolve, this.delay()));
    if(fail) {
      throw new Error('The lookup service is unavailable.');
    }
    return this.model.submit(query);
  }

  private delay: () => number;
  private fail: () => boolean;
  private model: LocalTickerQueryModel;
}
