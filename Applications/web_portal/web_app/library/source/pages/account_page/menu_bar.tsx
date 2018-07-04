import * as React from 'react';

interface Properties {

  /** Indicates the account item was clicked. */
  onAccountClicked?: () => void;

  /** Indicates the risk controls item was clicked. */
  onRiskControlsClicked?: () => void;

  /** Indicates the entitlements item was clicked. */
  onEntitlementsClicked?: () => void;

  /** Indicates the compliance item was clicked. */
  onComplianceClicked?: () => void;

  /** Indicates the profit and loss item was clicked. */
  onProfitAndLossClicked?: () => void;
}

interface State {
}

/** Displays the horizontal menu used to navigate an account's properties. */
export class MenuBar extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return null;
  }
}
