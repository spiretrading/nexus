import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';

/** The properties used to display the BurgerButton. */
export interface Properties {

  /** The width of the button. */
  width: number | string;

  /** The height of the button. */
  height: number | string;

  /** The color of the bars. */
  color: string;

  /** The color of the bars when highlighted. */
  highlightColor: string;

  /** The onClick event handler. */
  onClick?: () => void;
}

/** Displays a burger button. */
export class BurgerButton extends React.Component<Properties> {
  constructor(properties: Properties) {
    super(properties);
    this.buttonStyle = StyleSheet.create({
      unHighlighted: {
        ...BurgerButton.STYLE.buttonBase,
        backgroundColor: this.props.color,
        width: this.props.width,
        height: this.props.height
      },
      highlighted: {
        ...BurgerButton.STYLE.buttonBase,
        backgroundColor: this.props.highlightColor,
        width: this.props.width,
        height: this.props.height
      }
    });
    this.iconStyle = StyleSheet.create({
      unHighlighted: {
        ...BurgerButton.STYLE.iconBase,
        fill: this.props.highlightColor
      },
      highlighted: {
        ...BurgerButton.STYLE.iconBase,
        fill: this.props.color
      }
    });
    this.onClick = this.onClick.bind(this);
  }

  public componentWillReceiveProps(newProps: Properties) {
    this.buttonStyle = StyleSheet.create({
      unHighlighted: {
        backgroundColor: newProps.color,
        width: newProps.width,
        height: newProps.height
      },
      highlighted: {
        backgroundColor: newProps.highlightColor,
        width: newProps.width,
        height: newProps.height
      }
    });
    this.iconStyle = StyleSheet.create({
      unHighlighted: {
        ...BurgerButton.STYLE.iconBase,
        fill: newProps.highlightColor
      },
      highlighted: {
        ...BurgerButton.STYLE.iconBase,
        fill: newProps.color
      }
    });
  }
  public render(): JSX.Element {
    return (
        <button onClick={this.onClick}
                className={css(this.buttonStyle.unHighlighted)}
                ref={(ref) => this.button = ref}>
          <svg className={css(this.iconStyle.unHighlighted)}
               ref={(ref) => this.icon = ref}>
            <g>
              <rect y='0' width='20px' height='2'/>
              <rect y='6' width='20px' height='2'/>
              <rect y='12' width='20px' height='2'/>
          </g>
        </svg>
      </button>);
  }
  private onClick() {
    if(this.button.className === css(this.buttonStyle.unHighlighted)) {
      this.button.className = css(this.buttonStyle.highlighted);
    } else {
      this.button.className = css(this.buttonStyle.unHighlighted);
    }
    if(this.icon.className.baseVal === css(this.iconStyle.unHighlighted)) {
      this.icon.className.baseVal = css(this.iconStyle.highlighted);
    } else {
      this.icon.className.baseVal = css(this.iconStyle.unHighlighted);
    }
    if(this.props.onClick) {
      this.props.onClick();
    }
  }
  private static STYLE = {
    buttonBase: {
      position: 'relative' as 'relative',
      border: 'none',
      outline: '0',
      '-webkit-tap-highlight-color': 'rgba(0,0,0,0)',
      ':hover': {
        cursor: 'pointer'
      },
      ':active': {
        cursor: 'pointer'
      }
    },
    iconBase: {
      position: 'absolute' as 'absolute',
      top: 'calc(50% - 7px)',
      left: 'calc(50% - 10px)',
      width: '20px',
      height: '14px',
      ':hover': {
        cursor: 'pointer'
      },
      ':active': {
        cursor: 'pointer'
      }
    }
  };
  private buttonStyle: any;
  private iconStyle: any;
  private button: HTMLButtonElement;
  private icon: SVGElement;
}
