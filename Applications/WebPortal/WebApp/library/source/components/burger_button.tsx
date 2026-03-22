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

  /** Determines if the button is disabled. */
  disabled?: boolean;

  /** The onClick event handler. */
  onClick?: (event?: React.MouseEvent<HTMLDivElement>) => void;
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
  }

  public render(): JSX.Element {
    const barHeight = parseInt(this.props.height as any) / 7;
    const color = (() => {
      if(this.state.isHovered && this.props.highlightColor) {
        return this.props.highlightColor;
      }
      return this.props.color;
    })();
    const style: React.CSSProperties = {
      minWidth: this.props.width,
      width: this.props.width,
      minHeight: this.props.height,
      height: this.props.height,
      display: 'block',
      cursor: this.props.disabled ? 'not-allowed' : 'pointer',
      opacity: this.props.disabled ? 0.4 : undefined,
      pointerEvents: this.props.disabled ? 'none' : undefined
    };
    return (
      <div style={style} role='button' tabIndex={this.props.disabled ? -1 : 0}
          aria-label='Menu' aria-disabled={this.props.disabled}
          onKeyDown={this.onKeyDown}>
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

  private onHover = () => {
    this.setState({
      isHovered: true
    });
  }

  private onLeave = () => {
    this.setState({
      isHovered: false
    });
  }

  private onKeyDown = (event: React.KeyboardEvent) => {
    if(this.props.disabled) {
      return;
    }
    if(event.key === 'Enter' || event.key === ' ') {
      event.preventDefault();
      this.props.onClick?.();
    }
  }
}
