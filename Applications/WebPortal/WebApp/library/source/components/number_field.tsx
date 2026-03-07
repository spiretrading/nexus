import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The value to display in the field. */
  value?: number;

  /** Additional CSS styles. */
  style?: React.CSSProperties;

  /** The class name of the input field. */
  className?: string;

  /** The smallest number that the box accepts (inclusive). */
  min?: number;

  /** The largest number that the box accepts (inclusive). */
  max?: number;

  /** Determines if the component is read only. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: number) => void;
}

interface State {
  text: string;
}

/** An editable decimal number field. */
export class NumberField extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      text: (props.value ?? 0).toString()
    };
    this.inputRef = React.createRef<HTMLInputElement>();
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.readonly) {
        return NumberField.STYLE.boxReadonly;
      } else {
        return NumberField.STYLE.box;
      }
    })();
    return (
      <input type={'text'}
        ref={this.inputRef}
        value={this.state.text}
        style={{...boxStyle, ...this.props.style}}
        disabled={this.props.readonly}
        onChange={this.onChange}
        onBlur={this.onBlur}
        className={css(NumberField.EXTRA_STYLE.customHighlighting) + ' ' +
          this.props.className}/>);
  }

  public componentDidMount() {
    this.inputRef.current?.addEventListener('wheel', this.onWheel,
      {passive: false});
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({text: (this.props.value ?? 0).toString()});
    }
  }

  public componentWillUnmount() {
    this.inputRef.current?.removeEventListener('wheel', this.onWheel);
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const text = event.target.value;
    this.setState({text});
    const value = parseFloat(text);
    if(!isNaN(value) && value !== (this.props.value ?? 0)) {
      this.props.onChange?.(value);
    }
  }

  private onBlur = () => {
    let value = parseFloat(this.state.text);
    if(isNaN(value)) {
      value = this.props.value ?? 0;
    }
    if(this.props.min !== undefined) {
      value = Math.max(value, this.props.min);
    }
    if(this.props.max !== undefined) {
      value = Math.min(value, this.props.max);
    }
    this.setState({text: value.toString()});
    if(value !== (this.props.value ?? 0)) {
      this.props.onChange?.(value);
    }
  }

  private onWheel = (event: WheelEvent) => {
    if(document.activeElement !== event.currentTarget) {
      return;
    }
    event.preventDefault();
    const delta = event.deltaY < 0 ? 1 : -1;
    let next = (this.props.value ?? 0) + delta;
    if(this.props.min !== undefined) {
      next = Math.max(next, this.props.min);
    }
    if(this.props.max !== undefined) {
      next = Math.min(next, this.props.max);
    }
    if(next !== (this.props.value ?? 0)) {
      this.props.onChange?.(next);
    }
  }

  private static readonly STYLE = {
    box: {
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      flexGrow: 1,
      minWidth: '184px',
      maxWidth: '246px',
      width: '100%',
      paddingLeft: '10px',
      WebkitAppearance: 'textfield',
      appearance: 'none'
    } as React.CSSProperties,
    boxReadonly: {
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      flexGrow: 1,
      minWidth: '184px',
      maxWidth: '246px',
      width: '100%',
      paddingLeft: '10px',
      WebkitAppearance: 'textfield',
      appearance: 'none',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    customHighlighting: {
      ':focus': {
        borderColor: '#684BC7',
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

  private inputRef: React.RefObject<HTMLInputElement>;
}
