import * as React from 'react';

/** Specifies whether elements are ordered horizontally or vertically. */
enum BoxOrientation {

  /** Elements are layed out side-by-side. */
  HORIZONTAL,

  /** Elements are layed out top-to-bottom. */
  VERTICAL
}

interface Properties {

  /** The width of the BoxLayout, leaving this value empty results in a width
   *  equal to the width of its content.
   */
  width?: number | string;

  /** The height of the BoxLayout, leaving this value empty results in a
   *  height equal to the height of its content.
   */
  height?: number | string;
  
  /** The unique id of the BoxLayout HTML element. */
  id?: string;

  /** Specifies the CSS class of the BoxLayout HTML element. */
  className?: string;

  /** The orientation of the children elements within the box. */
  orientation?: BoxOrientation;

  /** The event handler called when the mouse enters the region. */
  onMouseEnter?: (event?: React.MouseEvent<any>) => void;

  /** The event handler called when the mouse leaves the region,
      or one of its child components. */
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

/** Implements a layout component where children elements subdivide a box either
 *  vertically or horizontally. */
export class BoxLayout extends React.Component<Properties> {
  public static defaultProps: Properties = {
    width: '',
    height: '',
    orientation: BoxOrientation.HORIZONTAL
  };

  public render(): JSX.Element {
    const outerStyle = Object.assign({}, (this.props.style || {}) as any);
    if(this.props.width !== '') {
      outerStyle.width = this.props.width;
    } else {
      outerStyle.display = 'inline-block';
    }
    if(this.props.height !== '') {
      outerStyle.height = this.props.height;
    }
    const direction = (() => {
      if(this.props.orientation === BoxOrientation.HORIZONTAL) {
        return 'row';
      } else {
        return 'column';
      }
    })();
    const innerStyle = {
      display: 'flex',
      flexDirection: direction,
      flexBasis: 'auto'
    } as any;
    if(this.props.width !== '') {
      innerStyle.width = this.props.width;
      if(this.props.orientation === BoxOrientation.HORIZONTAL) {
        if(typeof(this.props.width) === 'string' &&
            this.props.width.endsWith('%')) {
          innerStyle.flexGrow = 1;
          innerStyle.flexShrink = 1;
        } else {
          innerStyle.flexGrow = 0;
          innerStyle.flexShrink = 0;
        }
      }
    } else {
      innerStyle.width = '100%';
      if(this.props.orientation === BoxOrientation.HORIZONTAL) {
        innerStyle.flexGrow = 1;
        innerStyle.flexShrink = 1;
      }
    }
    if(this.props.height !== '') {
      innerStyle.height = this.props.height;
      if(this.props.orientation === BoxOrientation.VERTICAL) {
        if(typeof(this.props.width) === 'string' &&
            this.props.width.endsWith('%')) {
          innerStyle.flexGrow = 1;
          innerStyle.flexShrink = 1;
        } else {
          innerStyle.flexGrow = 0;
          innerStyle.flexShrink = 0;
        }
      }
    } else {
      innerStyle.height = '100%';
      if(this.props.orientation === BoxOrientation.VERTICAL) {
        innerStyle.flexGrow = 1;
        innerStyle.flexShrink = 1;
      }
    }
    const properChildren = React.Children.map(this.props.children,
      (child: any) => {
        if(child === null || child.type === undefined) {
          return child;
        } else if(child.type.name === 'Padding') {
          return React.cloneElement(child,
            {
              orientation: this.props.orientation
            });
        }
        return child;
      });
    return (
      <div id={this.props.id} style={outerStyle}
          onMouseEnter={this.props.onMouseEnter}
          className={this.props.className}
          onMouseOut={this.props.onMouseOut} onClick={this.props.onClick}
          onMouseLeave={this.props.onMouseLeave}
          ref={this.props.container}>
        <div style={innerStyle}>
          {properChildren}
        </div>
      </div>);
  }
}

export module BoxLayout {
  export const Orientation = BoxOrientation;
}
