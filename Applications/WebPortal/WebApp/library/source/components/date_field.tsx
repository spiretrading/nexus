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

  /** Additional CSS styles. */
  style?: any;

  /** The class name of the input box. */
  className?: string;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.Duration) => void;
}

/** A component that displays and lets a user edit a duration. */
export class DateField extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: new Beam.Date(0, 0, 0),
    onChange: () => {}
  };

  public render(): JSX.Element {
    const wrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DateField.STYLE.wrapperSmall;
      } else {
        return DateField.STYLE.wrapperLarge;
      }
    })();
    return (
      <div style={wrapperStyle} className={this.props.className}>
        <div style={DateField.STYLE.inner}>
          <IntegerInputBox
            min={1} max={31}
            value={this.props.value.day()}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.integerBoxLarge}
            padding={2}/>
          <div style={DateField.STYLE.slash}>{'/'}</div>
          <IntegerInputBox
            min={1} max={12}
            value={this.props.value.month()}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.integerBoxLarge}
            padding={2}/>
          <div style={DateField.STYLE.slash}>{'/'}</div>
          <IntegerInputBox
            min={0} max={3000}
            value={this.props.value.year()}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.yearBox}
            padding={4}/>
          </div>
          <div style={DateField.STYLE.placeholder}>
            {'Day / Month / Year'}
          </div>
      </div>);
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
    inner: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      justifyContent: 'flex-start' as 'flex-start',
      alignItems: 'center',
      marginLeft: '9px' 
    },
    integerBoxLarge: {
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
    },
    placeholder: {
      font: '500 11px Roboto',
      color: '#8C8C8C',
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      marginRight: '10px'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        ouline: 0,
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
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    }
  });
}
