import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { IntegerInputBox } from './integer_input_box';

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.Date;

  /** Determines if the selection box is read only. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.Date) => void;
}

interface State {
  isInFocus: boolean,
  componentWidth: number
}

/** A component that displays the date. */
export class DateField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Beam.Date(1, 1, 1900),
    readonly: false,
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isInFocus: false,
      componentWidth: 0
    };
    this.handleResize = this.handleResize.bind(this);
    this.onBlur = this.onBlur.bind(this);
    this.onFocus = this.onFocus.bind(this);
  } 

  public render(): JSX.Element {
    const wrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DateField.STYLE.wrapperSmall;
      } else {
        return DateField.STYLE.wrapperLarge;
      }
    })();
    const focusStyle = (() => {
      if(this.state.isInFocus) {
        return DateField.STYLE.focused;
      } else {
        return null;
      }
    })();
    const hintText = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.state.componentWidth >= 227) {
          return 'Day / Month / Year';
        } else if(this.state.componentWidth >= 161){
          return 'D / M / Y';
        } else {
          return '';
        }
      } else {
        return 'Day / Month / Year';
      }
    })();
    return (
      <div style={{...wrapperStyle, ...focusStyle}}
          ref={(input) => {this.reference = input;}}
          onFocus={this.onFocus}
          onBlur={this.onBlur}>
        <div style={DateField.STYLE.inner}>
          <IntegerInputBox
            min={1} max={31}
            value={this.props.value.day()}
            readonly={this.props.readonly}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.defaultIntegerBox}
            padding={2}/>
          <div style={DateField.STYLE.slash}>{'/'}</div>
          <IntegerInputBox
            min={1} max={12}
            value={this.props.value.month()}
            readonly={this.props.readonly}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.defaultIntegerBox}
            padding={2}/>
          <div style={DateField.STYLE.slash}>{'/'}</div>
          <IntegerInputBox
            min={2000} max={3000}
            value={this.props.value.year()}
            readonly={this.props.readonly}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.yearBox}
            padding={4}/>
        </div>
        <div style={DateField.STYLE.placeholder}>
          {hintText}
        </div>
      </div>);
  }

  public componentDidMount() {
    window.addEventListener('resize', this.handleResize);
    this.handleResize();
  }

  public componentWillUnmount() {
    window.removeEventListener('resize', this.handleResize);
  }
  private handleResize() {
    if(this.props.displaySize === DisplaySize.SMALL) {
      if(this.state.componentWidth !== this.reference.clientWidth) {
        this.setState({componentWidth: this.reference.clientWidth});
      }
    }
  }

  private onFocus() {
    if(!this.props.readonly) {
      this.setState({isInFocus: true});
    }
  }

  private onBlur() {
    if(!this.props.readonly) {
      this.setState({isInFocus: false});
    }
  }

  private static readonly STYLE = {
    wrapperSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      minWidth: '184px',
      width: '100%',
      flexShrink: 1,
      flexGrow: 1,
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '34px'
    },
    wrapperLarge: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      flexShrink: 1,
      maxWidth: '246px',
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '34px'
    },
    wrapperSmallReadonly: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      minWidth: '184px',
      width: '100%',
      flexShrink: 1,
      flexGrow: 1,
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #ffffff',
      borderRadius: '1px',
      height: '34px'
    },
    wrapperLargeReadonly: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      flexShrink: 1,
      maxWidth: '246px',
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #ffffff',
      borderRadius: '1px',
      height: '34px'
    },
    inner: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      justifyContent: 'flex-start' as 'flex-start',
      alignItems: 'center',
      marginLeft: '9px' 
    },
    defaultIntegerBox: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      minWidth: '16px',
      maxWidth: '16px',
      height: '17px',
      border: '0px solid #ffffff',
      padding: 0
    },
    yearBox: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      width: '34px',
      height: '17px',
      border: '0px solid #ffffff',
      padding: 0
    },
    slash: {
      width: '10px',
      height: '16px',
      flexGrow: 0,
      flexShrink: 0,
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      cursor: 'default' as 'default'
    },
    placeholder: {
      font: '500 11px Roboto',
      color: '#8C8C8C',
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      marginRight: '10px', 
      cursor: 'default' as 'default'
    },
    focused: {
      outline: 0,
      outlineColor: 'transparent',
      outlineStyle: 'none',
      border: '1px solid #684BC7',
      borderRadius: '1px'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        outline: 0,
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active' : {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    }
  });
  private reference: HTMLDivElement;
}
