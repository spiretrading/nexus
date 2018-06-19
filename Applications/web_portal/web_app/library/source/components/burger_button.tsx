import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {HBoxLayout} from '../layouts';
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

export interface State {}

/** Displays a burger button. */
export class BurgerButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.buttonStyle = StyleSheet.create({
      dimensions: {
        width: this.props.width,
        height: this.props.height
      },
      unHighlighted: {
        backgroundColor: this.props.color
      },
      highlighted: {
        backgroundColor: this.props.highlightColor
      }
    });
    this.iconStyle = StyleSheet.create({
      unHighlighted: {
        fill: this.props.highlightColor
      },
      highlighted: {
        fill: this.props.color
      }
    });
  }

  public componentWillReceiveProps(newProps: Properties) {
    this.buttonStyle = StyleSheet.create({
      dimensions: {
        width: newProps.width,
        height: newProps.height
      },
      unHighlighted: {
        backgroundColor: newProps.color
      },
      highlighted: {
        backgroundColor: newProps.highlightColor
      }
    });
    this.iconStyle = StyleSheet.create({
      unHighlighted: {
        fill: newProps.highlightColor
      },
      highlighted: {
        fill: newProps.color
      }
    });
  }
  public render(): JSX.Element {
    
    return (
      <HBoxLayout>
        <button onClick={this.props.onClick}
                className={css([BurgerButton.STYLE.buttonBase,
                                this.buttonStyle.dimensions,
                                this.buttonStyle.unHighlighted])}>
          <svg className={css([BurgerButton.STYLE.iconBase,
                               this.iconStyle.unHighlighted])}>
            <g>
              <rect y='0' width='20px' height='2'/>
              <rect y='6' width='20px' height='2'/>
              <rect y='12' width='20px' height='2'/>
          </g>
        </svg>
      </button>
    </HBoxLayout>);
  }
  private static STYLE = StyleSheet.create({
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
  });
  private buttonStyle: any;
  private iconStyle: any;
}
