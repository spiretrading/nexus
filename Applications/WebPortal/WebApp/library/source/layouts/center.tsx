import * as React from 'react';
import { HBoxLayout } from './hbox_layout'
import { Padding } from './padding'
import { VBoxLayout } from './vbox_layout'

interface Properties {

  /** The width of the box to center. */
  width?: number | string;

  /** The height of the box to center. */
  height?: number | string;

  /** The unique id of the Center HTML element. */
  id?: string;

  /** Specifies the CSS class of the Center HTML element. */
  className?: string;

  /** The event handler called when the mouse enters the region. */
  onMouseEnter?: (event?: React.MouseEvent<any>) => void;

  /** The event handler called when the mouse leaves the region,
      or one of its child components. */
  onMouseOut?: (event?: React.MouseEvent<any>) => void;

   /** The event handler called when the mouse leaves the region. */
  onMouseLeave?: (event?: React.MouseEvent<any>) => void;

  /** The event handler called when the layout is clicked. */
  onClick?: (event?: React.MouseEvent<any>) => void;
}

/** Implements a layout component that centers its content. */
export class Center extends React.Component<Properties> {
  public static defaultProps: Properties = {
    width: '100%',
    height: '100%'
  }

  public render(): JSX.Element {
    return (
      <HBoxLayout width={this.props.width} height={this.props.height}
          id={this.props.id} className={this.props.className}
          onMouseEnter={this.props.onMouseEnter}
          onMouseOut={this.props.onMouseOut} onClick={this.props.onClick}>
        <Padding/>
        <VBoxLayout>
          <Padding/>
          {this.props.children}
          <Padding/>
        </VBoxLayout>
        <Padding/>
      </HBoxLayout>);
  }
}
