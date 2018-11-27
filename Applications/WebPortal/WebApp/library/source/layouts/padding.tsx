import * as React from 'react';
import { BoxLayout } from './box_layout';

interface Properties {

  /** The size of the padding to add, by default the size is 100%. */
  size?: number | string;

  /** Specifies the CSS class of the Padding HTML element. */
  className?: string;
  /** Used internally to determine the padding's orientation. */
  orientation?: any;
}

/** Implements a layout component to space out sibling elements. */
export class Padding extends React.Component<Properties> {
  public static defaultProps: Properties = {
    size: 'auto'
  }

  public render(): JSX.Element {
    let style = (() => {
      if(this.props.size === 'auto') {
        return {
          width: '100%',
          height: '100%',
          flex: '1 1 auto'
        };
      } else if(this.props.orientation === BoxLayout.Orientation.HORIZONTAL) {
        return {
          width: this.props.size,
          height: '100%',
          flex: '0 0 auto'
        };
      } else if(this.props.orientation === BoxLayout.Orientation.VERTICAL) {
        return {
          width: '100%',
          height: this.props.size,
          flex: '0 0 auto'
        };
      } else {
        return {
          width: 'auto',
          height: 'auto',
          flex: `0 0 ${this.props.size}`
        };
      }
    })();
    return <div className={this.props.className} style={style}></div>;
  }
}
