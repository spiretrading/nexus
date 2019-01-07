import * as React from 'react';
import { BoxLayout } from './box_layout';

interface Properties {

  /** The width of the VBoxLayout, leaving this value empty results in a width
   *  equal to the width of its content.
   */
  width?: number | string;

  /** The height of the VBoxLayout, leaving this value empty results in a
   *  height equal to the height of its content.
   */
  height?: number | string;

  /** The unique id of the VBoxLayout HTML element. */
  id?: string;

  /** Specifies the CSS class of the VBoxLayout HTML element. */
  className?: string;

  /** The event handler called when the mouse enters the region. */
  onMouseEnter?: (event?: React.MouseEvent<any>) => void;

  /** The event handler called when the mouse leaves the region,
   *  or one of its child components. 
   */
  onMouseOut?: (event?: React.MouseEvent<any>) => void;

  /** The event handler called when the mouse leaves the region. */
  onMouseLeave?: (event?: React.MouseEvent<any>) => void;

  /** The event handler called when the layout is clicked. */
  onClick?: (event?: React.MouseEvent<any>) => void;

  /** Callback receiving the containing DIV element. */
  container?: (container: HTMLDivElement) => void;

  /** The CSS style to apply. */
  style?: any;
}

/** Provides a layout component where its children are positioned
 * top-to-bottom.
 */
export class VBoxLayout extends React.Component<Properties> {
  public static defaultProps: Properties = {
    width: '',
    height: ''
  };

  public render(): JSX.Element {
    return (
      <BoxLayout width={this.props.width} height={this.props.height}
          id={this.props.id} className={this.props.className}
          onMouseEnter={this.props.onMouseEnter}
          onMouseLeave={this.props.onMouseLeave}
          onMouseOut={this.props.onMouseOut} onClick={this.props.onClick}
          orientation={BoxLayout.Orientation.VERTICAL} style={this.props.style}
          container={this.props.container}>
        {this.props.children}
      </BoxLayout>);
  }
}
