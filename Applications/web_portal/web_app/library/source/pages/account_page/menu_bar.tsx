import * as React from 'react';

interface Properties {

  /** Indicates the account item was clicked. */
  onAccountClick?: () => void;

  /** Indicates the risk controls item was clicked. */
  onRiskControlsClick?: () => void;

  /** Indicates the entitlements item was clicked. */
  onEntitlementsClick?: () => void;

  /** Indicates the compliance item was clicked. */
  onComplianceClick?: () => void;

  /** Indicates the profit and loss item was clicked. */
  onProfitAndLossClick?: () => void;
}

interface State {
}

/** Displays the horizontal menu used to navigate an account's properties. */
export class MenuBar extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return null;
  }
}

interface ItemProperties {
  iconSrc: string;
  name: string;
  onClick?: () => void;
}

interface ItemState {
}

class Item extends React.Component<ItemProperties, ItemState> {
  public render(): JSX.Element {
    return null;
  }
}
