import { Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { HLine } from './hline';

/** The properties used to display the BurgerButton. */
interface Properties {

  /** The width of the button. */
  width: number | string;

  /** The height of the button. */
  height: number | string;

  /** The color of the bars. */
  color: string;

  /** The color of the bars when highlighted. */
  highlightColor?: string;

  /** The onClick event handler. */
  onClick?: (event?: React.MouseEvent<any>) => void;
}

interface State {
  isHovered: boolean;
}

/** Displays a burger button. */
export class BurgerButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isHovered: false,
    };
    this.onHover = this.onHover.bind(this);
    this.onLeave = this.onLeave.bind(this);
  }

  public render(): JSX.Element {
    const barHeight = parseInt(this.props.height as any) / 7;
    const color = (() => {
      if(this.state.isHovered && this.props.highlightColor) {
        return this.props.highlightColor;
      }
      return this.props.color;
    })();
    const style = {
      minWidth: this.props.width,
      width: this.props.width,
      minHeight: this.props.height,
      height: this.props.height,
      display: 'inline-block',
      cursor: 'pointer'
    };
    return (
      <div style={style}>
        <VBoxLayout width={this.props.width}
            height={this.props.height} onMouseEnter={this.onHover}
            onMouseOut={this.onLeave} onClick={this.props.onClick}>
          <HLine height={barHeight} color={color}/>
          <Padding/>
          <HLine height={barHeight} color={color}/>
          <Padding/>
          <HLine height={barHeight} color={color}/>
        </VBoxLayout>
      </div>);
  }

  private onHover() {
    this.setState({
      isHovered: true
    });
  }

  private onLeave() {
    this.setState({
      isHovered: false
    });
  }
}
