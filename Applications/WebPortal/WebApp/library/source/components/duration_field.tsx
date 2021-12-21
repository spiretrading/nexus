import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '..';
import { IntegerField } from './integer_field';

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

  /** The largest value the hours field can hold. */
  maxHourValue?: number;

  /** The smallest value the hours field can hold. */
  minHourValue?: number;

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

/** A component that displays a duration. */
export class DurationField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Beam.Duration(0),
    minHourValue: 0,
    maxHourValue: 99,
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isFocused: false,
      componentWidth: 0
    };
    this.containerRef = React.createRef<HTMLDivElement>();
  }

  public render(): JSX.Element {
    const splitTime = this.props.value.split();
    const containerStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DurationField.STYLE.containerSmall;
      } else {
        return DurationField.STYLE.containerLarge;
      }
    })();
    const focusClassName = (() => {
      if(this.state.isFocused) {
        return DurationField.STYLE.focused;
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
        <div style={DurationField.STYLE.inner}>
          <IntegerField
            min={this.props.minHourValue} max={this.props.maxHourValue}
            value={splitTime.hours}
            className={css(DurationField.EXTRA_STYLE.effects)}
            style={DurationField.STYLE.integerBox}
            onChange={this.onChange.bind(this, TimeUnit.HOURS)}
            readonly={this.props.readonly}
            padding={2}/>
          <div style={DurationField.STYLE.colon}>:</div>
          <IntegerField
            min={0} max={59}
            value={splitTime.minutes}
            className={css(DurationField.EXTRA_STYLE.effects)}
            style={DurationField.STYLE.integerBox}
            onChange={this.onChange.bind(this, TimeUnit.MINUTES)}
            readonly={this.props.readonly}
            padding={2}/>
          <div style={DurationField.STYLE.colon}>:</div>
          <IntegerField
            min={0} max={59}
            value={splitTime.seconds}
            className={css(DurationField.EXTRA_STYLE.effects)}
            style={DurationField.STYLE.integerBox}
            onChange={this.onChange.bind(this, TimeUnit.SECONDS)}
            readonly={this.props.readonly}
            padding={2}/>
        </div>
        <div style={DurationField.STYLE.placeholder}>
          {hintText}
        </div>
      </div>);
  }
  

  public componentDidMount() {
    window.addEventListener('resize', this.handleResize);
    this.handleResize();
  }

  public componentWillUnmount() {
    window.addEventListener('resize', this.handleResize);
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
    const oldDuration = this.props.value.split();
    const newValue = (() => {
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
    this.props.onChange(newValue);
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
