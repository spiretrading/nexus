import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';

/** The properties used to display the BurgerButton. */
interface Properties {

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
  constructor(props: Properties) {
    super(props);
    this.onMouseEnter = this.onMouseEnter.bind(this);
    this.onMouseLeave = this.onMouseLeave.bind(this);
    this.componentWillReceiveProps(props);
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
      <button className={this.unHighlightedClassName}
          onClick={this.props.onClick}
          onMouseEnter={this.onMouseEnter}
          onMouseLeave={this.onMouseLeave}
          ref={(ref) => this.button = ref}>
        <svg className={css(BurgerButton.STYLE.icon)}
            ref={(ref) => this.icon = ref}>
          <g>
            <rect y='0' width='100%' height='15%'/>
            <rect y='42.5%' width='100%' height='15%'/>
            <rect y='85%' width='100%' height='15%'/>
          </g>
        </svg>
      </button>);
  }

  private onMouseEnter() {
    this.button.className = this.highlightedClassName;
  }

  private onMouseLeave() {
    this.button.className = this.unHighlightedClassName;
  }

  private static defaultProps = {
    onClick: () => {}
  }
  private static STYLE = StyleSheet.create({
    buttonBase: {
      border: 'none',
      outline: '0',
      padding: 0,
      backgroundColor: 'rgba(0, 0, 0, 0)',
      '-webkit-tap-highlight-color': 'rgba(0,0,0,0)',
      ':hover': {
        cursor: 'pointer'
      },
      ':active': {
        cursor: 'pointer'
      }
    },
    icon: {
      width: '100%',
      height: '100%',
      fill: 'inherit',
      shapeRendering: 'geometricPrecision'
    }
  });
  private unHighlightedClassName: string;
  private highlightedClassName: string;
  private button: HTMLButtonElement;
  private icon: SVGElement;
}
