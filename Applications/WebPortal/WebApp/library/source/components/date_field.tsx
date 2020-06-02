import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize, IntegerField } from '..';

enum DateUnit {
  DAY,
  MONTH,
  YEAR
}

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.Date;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.Date) => void;
}

interface State {
  isFocused: boolean,
  componentWidth: number
}

/** A component that displays a date. */
export class DateField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Beam.Date(1, 1, 1900),
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isFocused: false,
      componentWidth: 0
    };
    this.containerRef = React.createRef<HTMLDivElement>();
    this.handleResize = this.handleResize.bind(this);
    this.onBlur = this.onBlur.bind(this);
    this.onFocus = this.onFocus.bind(this);
  } 

  public render(): JSX.Element {
    const containerStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DateField.STYLE.containerSmall;
      } else {
        return DateField.STYLE.containerLarge;
      }
    })();
    const focusStyle = (() => {
      if(this.state.isFocused) {
        return DateField.STYLE.focused;
      } else {
        return null;
      }
    })();
    const maxDate = (() => {
      const month = this.props.value.month;
      if(month === 4 || month === 6 || month === 9 || month === 11) {
        return 30;
      } else if(month === 2) {
        const year = this.props.value.year;
        if(year % 4 !== 0) {
          return 28;
        } else if(year % 100 !== 0) {
          return 29;
        } else if(year % 400 !== 0) {
          return 28;
        } else {
          return 29;
        }
      } else {
        return 31;
      }
    })();
    const hintText = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.state.componentWidth >= 227) {
          return 'Day / Month / Year';
        } else if(this.state.componentWidth >= 161) {
          return 'D / M / Y';
        } else {
          return '';
        }
      } else {
        return 'Day / Month / Year';
      }
    })();
    return (
      <div style={{...containerStyle, ...focusStyle}}
          ref={this.containerRef}
          onFocus={this.onFocus}
          onBlur={this.onBlur}>
        <div style={DateField.STYLE.inner}>
          <IntegerField
            min={1} max={maxDate}
            value={this.props.value.day}
            readonly={this.props.readonly}
            onChange={this.onChange.bind(this, DateUnit.DAY)}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.defaultIntegerBox}
            padding={2}/>
          <div style={DateField.STYLE.slash}>/</div>
          <IntegerField
            min={1} max={12}
            value={this.props.value.month}
            readonly={this.props.readonly}
            onChange={this.onChange.bind(this, DateUnit.MONTH)}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.defaultIntegerBox}
            padding={2}/>
          <div style={DateField.STYLE.slash}>/</div>
          <IntegerField
            min={2000} max={3000}
            value={this.props.value.year}
            readonly={this.props.readonly}
            onChange={this.onChange.bind(this, DateUnit.YEAR)}
            className={css(DateField.EXTRA_STYLE.effects)}
            style={DateField.STYLE.yearBox}
            padding={4}/>
        </div>
        <div style={DateField.STYLE.hintText}>
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
    if(this.props.displaySize === DisplaySize.SMALL && 
        this.state.componentWidth !== this.containerRef.current.clientWidth) {
      this.setState({componentWidth: this.containerRef.current.clientWidth});
    }
  }

  private onFocus() {
    if(!this.props.readonly) {
      this.setState({isFocused: true});
    }
  }

  private onBlur() {
    if(!this.props.readonly) {
      this.setState({isFocused: false});
    }
  }

  private onChange(dateUnit: DateUnit, value: number) {
    const oldValue = this.props.value;
    const newValue = (() => {
      switch(dateUnit) {
        case DateUnit.DAY:
          return new Beam.Date(oldValue.year, oldValue.month, value);
        case DateUnit.MONTH:
          return new Beam.Date(oldValue.year, value, oldValue.day);
        case DateUnit.YEAR:
          return new Beam.Date(value, oldValue.month, oldValue.day);
      }
    })();
    this.props.onChange(newValue);
  }

  private static readonly STYLE = {
    containerSmall: {
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
    containerLarge: {
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
    hintText: {
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
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    }
  });

  private containerRef: React.RefObject<HTMLDivElement>;
}
