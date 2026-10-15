import * as Nexus from 'nexus';
import * as React from 'react';
import { LocalTickerQueryModel, ScopeInput, ScopeQueryModel, TickerQueryModel }
  from 'web_portal';

interface Properties {
  value: Nexus.Scope;
  readOnly: boolean;
  disabled: boolean;
  lookupDelay: number;
  failLookup: boolean;
  onChange: (value: Nexus.Scope) => void;
}

/** Demonstrates the scope input and its asynchronous editor. */
export class ScopeInputExample extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.model = new ScopeQueryModel(new ExampleTickerModel(
      () => this.props.lookupDelay, () => this.props.failLookup));
  }

  public render(): JSX.Element {
    return <ScopeInput model={this.model} value={this.props.value}
      readOnly={this.props.readOnly} disabled={this.props.disabled}
      aria-label='Scope' onChange={this.props.onChange}/>;
  }

  private model: ScopeQueryModel;
}

class ExampleTickerModel extends TickerQueryModel {
  constructor(delay: () => number, fail: () => boolean) {
    super();
    this.delay = delay;
    this.fail = fail;
    this.model = new LocalTickerQueryModel([
      'ABX.TSX', 'BHP.ASX', 'BMO.TSX', 'BMO.ASX', 'RY.TSX', 'SHOP.TSX',
      'TECK.B.TSX', 'TEST.TSXV', 'BMO.TSX'
    ].map(ticker => Nexus.Ticker.parse(ticker)));
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
