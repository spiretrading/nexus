import * as React from 'react';

/** The properties used to display the BurgerButton. */
interface Properties {

  /** The width and height of the button. */
  size: number | string;

  /** Determines if the check mark is readonly or not. */
  isReadOnly: boolean;

  /** The onClick event handler. */
  onClick?: (event?: React.MouseEvent<any>) => void;

}

interface State {
  isChecked: boolean;
}

/** Displays a check mark button. */
export class CheckMarkButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isChecked: false,
    };
    this.onClick = this.onClick.bind(this);
  }

  public render(): JSX.Element {
    const source = (() => {
      if (this.state.isChecked) {
        return (
          'resources/account_page/entitlements_page/icons/check-green.svg');
      } else {
        return (
          'resources/account_page/entitlements_page/icons/check-grey.svg');
      }
    })();
    return (
      <div>
        <img src={source}
          alt='checkbox'
          height={this.props.size}
          width={this.props.size} 
          onClick={this.onClick}
          style = {this.STYLE.base}/>
      </div>);
  }

  private onClick() {
    this.setState({
      isChecked: !this.state.isChecked
    });
    this.props.onClick();
  }

  private readonly STYLE = {
    base: {
      cursor: 'pointer'
    }
  }
}
