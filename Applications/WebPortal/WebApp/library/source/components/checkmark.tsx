import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../display_size';

interface Properties {

  /** The size to display the component at. */
  displaySize?: DisplaySize;

  /** Determines if the box is checked. */
  isChecked: boolean;

  /** Determines if the component can be edited. */
  readonly?: boolean;

  /** Called when the check mark is clicked on.
   * @param value - The new value.
   */
  onClick?: (value?: boolean) => void;
}

/** A checkmark component. */
export class Checkmark extends React.Component<Properties> {
  public static readonly defaultProps = {
    onClick: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.onClick = this.onClick.bind(this);
  }
  
  public render(): JSX.Element {
    const imgSrc = (() => {
      if(this.props.isChecked) {
        return 'resources/components/check-green.svg';
      } else {
        return 'resources/components/check-grey.svg';
      }
    })();
    const size = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    const containerStyle = (() => {
      if(this.props.readonly) {
        return Checkmark.STYLE.containerReadonly;
      } else {
        return Checkmark.STYLE.container;
      }
    })();
    return (
      <div style={containerStyle}
          className={css(Checkmark.EXTRA_STYLE.noDefaults)}
          onClick={this.onClick}>
        <img height={size} width={size} src={imgSrc}/>
      </div>);
  }

  private onClick() {
    if(!this.props.readonly) {
      this.props.onClick(!this.props.isChecked);
    }
  }

  private static readonly STYLE = {
    container: {
      boxSizing: 'border-box' as 'border-box',
      height: '20px',
      width: '20px',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      alignContent: 'center' as 'center',
      justifyContent: 'center' as 'center',
      cursor: 'pointer' as 'pointer'
    },
    containerReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '20px',
      width: '20px',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      alignContent: 'center' as 'center',
      justifyContent: 'center' as 'center',
      cursor: 'default' as 'default'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    noDefaults: {
      '-webkit-appearance': 'none',
      ':active': {
        outline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':focus': {
        outline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      ':-moz-focusring': {
        color: 'transparent',
        textShadow: '0 0 0 #000000'
      }
    }
  });
}
