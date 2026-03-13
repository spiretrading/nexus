import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '..';
import { IntegerInput } from './integer_input';

enum TimeUnit {
  HOURS,
  MINUTES,
  SECONDS
}

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.Duration;

  /** Determines if the field box is read only. */
  readonly?: boolean;

  /**
   * Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.Duration) => void;
}

interface State {
  isFocused: boolean,
  componentWidth: number
}

/** A component that displays a time of day. */
export class TimeOfDayField extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isFocused: false,
      componentWidth: 0
    };
    this.containerRef = React.createRef<HTMLDivElement>();
  }

  public render(): JSX.Element {
    const value = this.props.value ?? new Beam.Duration(0);
    const splitTime = value.split();
    const containerStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TimeOfDayField.STYLE.containerSmall;
      } else {
        return TimeOfDayField.STYLE.containerLarge;
      }
    })();
    const focusClassName = (() => {
      if(this.state.isFocused) {
        return TimeOfDayField.STYLE.focused;
      } else {
        return null;
      }
    })();
    const hintText = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.state.componentWidth >= 165) {
          return 'Hr : Min : Sec';
        } else if(this.state.componentWidth >= 141) {
          return 'H : M : S';
        } else {
          return '';
        }
      } else {
        return 'Hr : Min : Sec';
      }
    })();
    return (
      <div style={{...containerStyle, ...focusClassName}}
          ref={this.containerRef}
          onFocus={this.onFocus}
          onBlur={this.onBlur}>
        <div style={TimeOfDayField.STYLE.inner}>
          <IntegerInput
            min={0} max={23}
            value={splitTime.hours}
            className={css(TimeOfDayField.EXTRA_STYLE.effects)}
            style={TimeOfDayField.STYLE.integerBox}
            onChange={(hours: number) => this.onChange(TimeUnit.HOURS, hours)}
            readOnly={this.props.readonly}
            leadingZeros={2}/>
          <div style={TimeOfDayField.STYLE.colon}>:</div>
          <IntegerInput
            min={0} max={59}
            value={splitTime.minutes}
            className={css(TimeOfDayField.EXTRA_STYLE.effects)}
            style={TimeOfDayField.STYLE.integerBox}
            onChange={(minutes: number) => this.onChange(TimeUnit.MINUTES, minutes)}
            readOnly={this.props.readonly}
            leadingZeros={2}/>
          <div style={TimeOfDayField.STYLE.colon}>:</div>
          <IntegerInput
            min={0} max={59}
            value={splitTime.seconds}
            className={css(TimeOfDayField.EXTRA_STYLE.effects)}
            style={TimeOfDayField.STYLE.integerBox}
            onChange={(seconds: number) => this.onChange(TimeUnit.SECONDS, seconds)}
            readOnly={this.props.readonly}
            leadingZeros={2}/>
        </div>
        <div style={TimeOfDayField.STYLE.placeholder}>
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

  private handleResize = () => {
    if(this.props.displaySize === DisplaySize.SMALL &&
        this.state.componentWidth !== this.containerRef.current.clientWidth) {
      this.setState({componentWidth: this.containerRef.current.clientWidth});
    }
  }

  private onFocus = () => {
    if(!this.props.readonly) {
      this.setState({isFocused: true});
    }
  }

  private onBlur = () => {
    if(!this.props.readonly) {
      this.setState({isFocused: false});
    }
  }

  private onChange = (timeUnit: TimeUnit, value: number) => {
    const oldDuration = (this.props.value ?? new Beam.Duration(0)).split();
    const localTimeValue = (() => {
      switch(timeUnit) {
        case TimeUnit.HOURS:
          return Beam.Duration.HOUR.multiply(value).add(
            Beam.Duration.MINUTE.multiply(oldDuration.minutes)).add(
            Beam.Duration.SECOND.multiply(oldDuration.seconds));
        case TimeUnit.MINUTES:
          return Beam.Duration.HOUR.multiply(oldDuration.hours).add(
            Beam.Duration.MINUTE.multiply(value)).add(
            Beam.Duration.SECOND.multiply(oldDuration.seconds));
        case TimeUnit.SECONDS:
          return Beam.Duration.HOUR.multiply(oldDuration.hours).add(
            Beam.Duration.MINUTE.multiply(oldDuration.minutes)).add(
            Beam.Duration.SECOND.multiply(value));
      }
    })();
    const localTimeSplit = localTimeValue.split();
    this.props.onChange?.(
      Beam.Duration.HOUR.multiply(localTimeSplit.hours).add(
        Beam.Duration.MINUTE.multiply(localTimeSplit.minutes)).add(
        Beam.Duration.SECOND.multiply(localTimeSplit.seconds)));
  }

  private static readonly STYLE = {
    containerSmall: {
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'row',
      minWidth: '110px',
      width: '100%',
      flexShrink: 1,
      flexGrow: 1,
      backgroundColor: '#ffffff',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '34px'
    } as React.CSSProperties,
    containerLarge: {
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'row',
      flexGrow: 1,
      flexShrink: 1,
      maxWidth: '246px',
      backgroundColor: '#ffffff',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '34px'
    } as React.CSSProperties,
    inner: {
      display: 'flex',
      flexDirection: 'row',
      flexGrow: 1,
      justifyContent: 'flex-start',
      alignItems: 'center',
      marginLeft: '9px' 
    } as React.CSSProperties,
    integerBox: {
      boxSizing: 'border-box',
      font: '400 14px Roboto',
      width: '18px',
      height: '17px',
      border: '0px solid #ffffff',
      padding: 0
    } as React.CSSProperties,
    colon: {
      width: '10px',
      height: '16px',
      flexGrow: 0,
      flexShrink: 0,
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      cursor: 'default'
    } as React.CSSProperties,
    placeholder: {
      font: '500 11px Roboto',
      color: '#8C8C8C',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      marginRight: '10px',
      cursor: 'default'
    } as React.CSSProperties,
    focused: {
      outlineColor: 'transparent',
      outlineStyle: 'none',
      border: '1px solid #684BC7',
      borderRadius: '1px'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        border: '0px solid #333333',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      }
    }
  });

  private containerRef: React.RefObject<HTMLDivElement>;
}
