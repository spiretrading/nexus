import * as React from 'react';

/** The properties used to display the BurgerButton. */
interface Properties {

  /** The width and height of the button. */
  size: number | string;

  /** Determines if the check mark is readonly or not. */
  isReadOnly: boolean;

  /** The onClick event handler.
   * This is not needed if the component is readonly.
   * This should cause the isChecked property to toggle.
   * */
  onClick?(event?: React.MouseEvent<any>): void;

  /** Determines if the checkmark is currenly active(green) or inactive(grey)*/
  isChecked: boolean;
}

/** Displays a check mark button. */
export class CheckMarkButton extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
    this.onClick = this.onClick.bind(this);
  }

  public render(): JSX.Element {
    const source = (() => {
      if(this.props.isChecked) {
        return (
          'resources/account_page/entitlements_page/icons/check-green.svg');
      } else {
        return (
          'resources/account_page/entitlements_page/icons/check-grey.svg');
      }
    })();
    return (
      <img src={source}
        alt='checkbox'
        height={this.props.size}
        width={this.props.size}
        onClick={this.onClick}
        style={CheckMarkButton.STYLE.base}/>);
  }

  private onClick() {
    if(!this.props.isReadOnly) {
      this.props.onClick();
    }
  }

  private static STYLE = {
    base: {
      cursor: 'pointer'
    }
  };
}
