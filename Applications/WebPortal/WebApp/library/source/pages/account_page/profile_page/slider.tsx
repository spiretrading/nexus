import * as React from 'react';
import { css, StyleSheet } from 'aphrodite/no-important';

interface Properties {

  /** The onClick event handler. */
  onSlide?: () => void;

  onRescale?: (num: number) => void;
}

interface State {
  value: number;
}

export class Slider extends React.Component<Properties, State> {

  constructor(properties: Properties) {
    super(properties);
    this.state = {
      value: 0
    };
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {

    return (
      <div>
        <input type='range' min='0' max='100' value={this.state.value}
          onInput={(e) => this.onChange(e)}
          className={css(Slider.SLIDER.slider)} />
      </div>);
  }

  private onChange(event: any) {
    const num = event.target.value;
    console.log('the value from slider' + num);
    if (this.state.value < num) {
      this.setState({ value: this.state.value + 5});
    } else {
      this.setState({ value: this.state.value - 5 });
    }
    console.log('the value set:' + this.state.value);
    if(this.props.onRescale) {
      this.props.onRescale(this.state.value);
    }
  }

  public static readonly STYLE = {
    containerStyle: {
      position: 'relative' as 'relative',
      width: '100%',
      height: '20px'
    },
    filler: {
      height: '8px'
    },
    circle: {
      boxSizing: 'border-box' as 'border-box',
      position: 'absolute' as 'absolute',
      cursor: 'pointer' as 'pointer',
      zIndex: 1,
      height: '20px',
      width: '20px',
      backgroundColor: '#FFFFFF',
      border: '1px solid #8C8C8C',
      borderRadius: '20px'
    }
  };

  public static readonly SLIDER = StyleSheet.create({
    slider: {
      width: '100%',
      height: '20px',
      margin: 0,
      outline: 0,
      '::-webkit-slider-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-moz-range-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-ms-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-webkit-slider-runnable-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '::-moz-range-track': {
        backgroundColor: '#E6E6E6',
        height: '4px',
        border: 0
      },
      '::-ms-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '-webkit-appearance': 'none',
      '-moz-appearance': 'none',
      'appearance': 'none',
      '::-moz-focus-outer': {
        border: 0
      }
    }
  });
}
