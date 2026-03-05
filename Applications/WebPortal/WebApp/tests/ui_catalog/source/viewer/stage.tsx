import * as React from 'react';
import { ComponentSchema, SignalSchema } from '../data/schemas';

interface Properties {

  /** The component to display. */
  component: ComponentSchema;

  /** The values to pass to the component. */
  values: {[name: string]: any};

  /** Whether to show a border around the component. */
  showBorder: boolean;

  /** The container width (-1 for preferred size). */
  containerWidth: number;

  /** The container height (-1 for preferred size). */
  containerHeight: number;
}

interface State {

  /** The component to display. */
  backgroundColor: string;
}

/** Displays the component associated with a given schema. */
export class Stage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      backgroundColor: Stage.SOFT_GREY
    };
  }

  public render(): JSX.Element {
    const {children, ...componentProps} = this.props.values;
    return (
      <div style={{...Stage.STYLE.container,
          backgroundColor: this.state.backgroundColor}}>
        <div style={Stage.STYLE.header}>
          {this.props.component.name}
          <button style={Stage.STYLE.button}
              onClick={this.onClick}>
            Toggle Color
          </button>
        </div>
        <div style={{...Stage.STYLE.componentWrapper,
            ...(this.props.showBorder && Stage.STYLE.border),
            ...(this.props.containerWidth !== -1 &&
              {width: `${this.props.containerWidth}px`}),
            ...(this.props.containerHeight !== -1 &&
              {height: `${this.props.containerHeight}px`})}}>
          <this.props.component.render {...componentProps}>
            {children}
          </this.props.component.render>
        </div>
      </div>);
  }

  private onClick = () => {
    if(this.state.backgroundColor === Stage.SOFT_GREY) {
      this.setState({backgroundColor: Stage.WHITE});
    } else {
      this.setState({backgroundColor: Stage.SOFT_GREY});
    }
  }

  private static readonly STYLE = {
    container: {
      width: '100%',
      height: '100%',
      display: 'flex',
      flexDirection: 'column',
      padding: '22px 20px 20px 20px',
      boxSizing: 'border-box',
      backgroundColor: '#E8E8E8'
    } as React.CSSProperties,
    header: {
      paddingBottom: '30px',
      fontSize: '20px',
      fontWeight: 700,
      fontFamily: 'Roboto',
      color: '#000000'
    } as React.CSSProperties,
    button: {
      fontSize: '20px',
      fontWeight: 700,
      margin: '20px'
    } as React.CSSProperties,
    componentWrapper: {
      alignSelf: 'flex-start'
    } as React.CSSProperties,
    border: {
      border: '5px dashed #999'
    } as React.CSSProperties
  };
  private static readonly SOFT_GREY = '#E8E8E8';
  private static readonly WHITE = '#FFFFFF'
}
