import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The parameters to display. */
  parameters: Nexus.RiskParameters;

  /** Used to lookup currency names and symbols. */
  currencyDatabase: Nexus.CurrencyDatabase;
}

interface State {}

/** Implements a React component to display a set of RiskParameters. */
export class RiskParametersView extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return null;
  }
}
