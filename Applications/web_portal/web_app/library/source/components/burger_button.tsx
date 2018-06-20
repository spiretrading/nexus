import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {HBoxLayout} from '../';

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
    const buttonStyles = StyleSheet.create({
      highlighted: {
        ...BurgerButton.STYLE.buttonBase,
        width: this.props.width,
        height: this.props.height,
        fill: this.props.highlightColor
      },
      unHighlighted: {
        ...BurgerButton.STYLE.buttonBase,
        width: this.props.width,
        height: this.props.height,
        fill: this.props.color
      }
    });
    this.unHighlightedClassName = css([buttonStyles.unHighlighted,
      BurgerButton.STYLE.buttonBase]);
    this.highlightedClassName = css([buttonStyles.highlighted,
      BurgerButton.STYLE.buttonBase]);
    this.onClick = this.onClick.bind(this);
  }

  public componentWillReceiveProps(newProps: Properties) {
    const buttonStyles = StyleSheet.create({
      highlighted: {
        ...BurgerButton.STYLE.buttonBase,
        width: newProps.width,
        height: newProps.height,
        fill: newProps.highlightColor
      },
      unHighlighted: {
        ...BurgerButton.STYLE.buttonBase,
        width: newProps.width,
        height: newProps.height,
        fill: newProps.color
      }
    });
    this.unHighlightedClassName = css([buttonStyles.unHighlighted,
      BurgerButton.STYLE.buttonBase]);
    this.highlightedClassName = css([buttonStyles.highlighted,
      BurgerButton.STYLE.buttonBase]);
  }

  public render(): JSX.Element {
    return (
        <button onClick={this.onClick}
            className={this.unHighlightedClassName}
            ref={(ref) => this.button = ref}>
          <svg className={css(BurgerButton.STYLE.icon)}
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
    if(this.button.className === this.unHighlightedClassName) {
      this.button.className = this.highlightedClassName;
    } else {
      this.button.className = this.unHighlightedClassName;
    }
    if(this.props.onClick) {
      this.props.onClick();
    }
  }
  private static STYLE = StyleSheet.create({
    buttonBase: {
      position: 'relative' as 'relative',
      border: 'none',
      backgroundColor: 'rgba(0, 0, 0, 0)',
      outline: '0',
      '-webkit-tap-highlight-color': 'rgba(0,0,0,0)',
      ':hover': {
        cursor: 'pointer'
      },
      ':active': {
        cursor: 'pointer'
      }
    },
    icon: {
      position: 'absolute' as 'absolute',
      top: 'calc(50% - 7px)',
      left: 'calc(50% - 10px)',
      viewBox: '0 0 20 14',
      width: '20px',
      height: '14px',
      fill: 'inherit',
      shapeRendering: 'geometricPrecision',
      ':hover': {
        cursor: 'pointer'
      },
      ':active': {
        cursor: 'pointer'
      }
    }
  });
  private unHighlightedClassName: string;
  private highlightedClassName: string;
  private button: HTMLButtonElement;
  private icon: SVGElement;
}
