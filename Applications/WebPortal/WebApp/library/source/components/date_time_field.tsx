import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { DurationInputField } from './duration_input_field';
import { DateField } from './date_field';

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.Duration;

  newValue?: Beam.DateTime;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange: (value: Beam.DateTime) => void;
}

interface State {
  isInFocus: boolean
}

/** A component that displays and lets a user edit a duration. */
export class DateTimeField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Beam.Duration(0),
    newValue: new Beam.DateTime(new Beam.Date(0, 0, 0), new Beam.Duration(0)),
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isInFocus: false
    }
  } 

  public render(): JSX.Element {
    const focusClassName = (() => {
      if(this.state.isInFocus) {
        return DateTimeField.EXTRA_STYLE.focusEffects;
      } else {
        return null;
      }
    })();
    console.log('should purple?: ', this.state.isInFocus);
    return (
      <div style={DateTimeField.STYLE.outerWrapper}
        onFocus={() => this.setState({isInFocus: true})}
        onBlur={() => this.setState({isInFocus: false})}>
        <DateField
          className={css(focusClassName)}
          displaySize={this.props.displaySize}
          value={this.props.newValue.date()}/>
        <div style={DateTimeField.STYLE.filler}/>  
        <div style={DateTimeField.STYLE.durationWrapper}>
          <DurationInputField 
            displaySize={this.props.displaySize}
            value={this.props.newValue.timeOfDay()}
            className={css(focusClassName)}/>
          <select style={DateTimeField.STYLE.select}
            className={css(DateTimeField.EXTRA_STYLE.noHighlighting, focusClassName)}>
            <option>{'AM'}</option>
            <option>{'PM'}</option>
          </select>
        </div>
      </div>);
  }

  private static readonly STYLE = {
    outerWrapper: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      height: '78px'
    },
    durationWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
    },
    select: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      marginLeft: '10px',
      paddingLeft: '7px',
      width: '64px',
      height: '34px',
      color: '#000000',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage:
        'url(resources/account_page/profile_page/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none'
    },
    filler: {
      height: '10px',
      width: '100%'
    }
  };
  public static readonly EXTRA_STYLE = StyleSheet.create({
    focusEffects: {
      ouline: 0,
      outlineColor: 'transparent',
      outlineStyle: 'none',
      border: '1px solid #684BC7',
      borderRadius: '1px'
    },
    noHighlighting: {
      ':focus': {
        ouline: 0,
        outlineColor: 'transparent',
        outlineStyle: 'none',
        border: '1px solid #684BC7',
        borderRadius: '1px'
      },
      '::moz-focus-inner': {
        border: 0
      },
      ':-moz-focusring': {
        color: 'transparent',
        textShadow: '0 0 0 #000'
      },
      '-webkit-user-select': 'text',
      '-moz-user-select': 'text',
      '-ms-user-select': 'text',
      'user-select': 'text'
    }
  });
}
