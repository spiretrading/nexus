import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../../';

enum Effects {

  /** No effects need to be applied to the component. */
  NONE,

  /** The input box is focused. */
  FOCUSED,

  /** The mouse is hovering over the component. */
  HOVER
}

interface Properties {

  /** Indicates the input field can not be interacted with. */
  readonly?: boolean;

  /** The value to display in the field. */
  value?: string;

  /** Text to show if the value is empty. */
  placeholder?: string;

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** Indicates if there is an error with the value. */
  isError?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onInput?: (value: string) => void;
}

interface State {
  effects: Effects;
}

/** Displays a single text input field. */
export class TextField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    readonly: false,
    value: '',
    placeholder: '',
    isError: false,
    onInput: (_: string) => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      effects: Effects.NONE
    };
    this.onInputBlurred = this.onInputBlurred.bind(this);
    this.onInputFocused = this.onInputFocused.bind(this);
    this.onMouseEnter = this.onMouseEnter.bind(this);
    this.onMouseLeave = this.onMouseLeave.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.readonly) {
        return TextField.STYLE.readonlyBox;
      } else if(this.props.isError) {
        return TextField.STYLE.errorBox;
      } else if(this.state.effects === Effects.HOVER) {
        return TextField.STYLE.hoveredBox;
      } else if(this.state.effects === Effects.FOCUSED) {
        return TextField.STYLE.focusedBox;
      } else {
        return TextField.STYLE.box;
      }
    })();
    const textStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TextField.STYLE.largerText;
      } else {
        return TextField.STYLE.text;
      }
    })();
    const image = (() => {
      if(this.state.effects === Effects.HOVER && !this.props.readonly) {
        return <img src='resources/account_page/edit.svg'
          className={css(TextField.STYLE.image)}/>;
      } else {
        return <img src='resources/account_page/edit.svg'
          className={css(TextField.STYLE.hidden)}/>;
      }
    })();
    const tabIndexValue = (() => {
      if(this.props.readonly) {
        return -1;
      } else {
        return 0;
      }
    })();
    return (
      <div className={css(boxStyle)}
          onMouseEnter={this.onMouseEnter}
          onMouseLeave={this.onMouseLeave}>
        <input value={this.props.value}
          placeholder={this.props.placeholder}
          tabIndex={tabIndexValue}
          readOnly={this.props.readonly}
          onFocus={this.onInputFocused}
          onBlur={this.onInputBlurred}
          onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
            this.props.onInput(event.target.value);
          }}
          className={css(textStyle)}/>
          {image}
      </div>);
  }

  private onInputFocused() {
    if(!this.props.readonly) {
      this.setState({
        effects: Effects.FOCUSED
      });
    }
  }

  private onInputBlurred() {
    if(!this.props.readonly) {
      this.setState({
        effects: Effects.NONE
      });
    }
  }

  private onMouseEnter() {
    if(!this.props.readonly && this.state.effects !== Effects.FOCUSED) {
      this.setState({
        effects: Effects.HOVER
      });
    }
  }

  private onMouseLeave() {
    if(!this.props.readonly && this.state.effects !== Effects.FOCUSED) {
      this.setState({
        effects: Effects.NONE
      });
    }
  }

  private static STYLE = StyleSheet.create({
    box: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #FFFFFF',
      borderRadius: '1px',
      ':focus': {
        ouline: 0
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    },
    hoveredBox: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px'
    },
    focusedBox: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #684BC7',
      borderRadius: '1px'
    },
    errorBox: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #E63F44',
      borderRadius: '1px'
    },
    readonlyBox: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #FFFFFF',
      borderRadius: '1px'
    },
    inputStyle: {
      flexGrow: 1
    },
    image: {
      visibility: 'visible' as 'visible',
      height: '14px',
      width: '14px',
      paddingRight: '10px',
      flex: '0, 0, auto'
    },
    hidden: {
      opacity: 0,
      height: '14px',
      width: '14px',
      paddingRight: '10px'
    },
    text: {
      font: '400 14px Roboto',
      color: '#333333',
      whiteSpace: 'nowrap',
      paddingLeft: '10px',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      flexGrow: 1,
      ':focus': {
        ouline: 0,
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::-ms-clear': {
        display: 'none' as 'none'
      },
      ':-moz-read-only': {
        color: 'transparent',
        textShadow: '0 0 0 #000000'
      },
      ':read-only': {
        color: 'transparent',
        textShadow: '0 0 0 #000000'
      }
    },
    largerText: {
      font: '400 16px Roboto',
      color: '#000000',
      whiteSpace: 'nowrap',
      paddingLeft: '10px',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      flexGrow: 1,
      ':focus': {
        ouline: 0,
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::-ms-clear': {
        display: 'none' as 'none'
      },
      ':-moz-read-only': {
        color: 'transparent',
        textShadow: '0 0 0 #000000'
      },
      ':read-only': {
        color: 'transparent',
        textShadow: '0 0 0 #000000'
      }
    }
  });
}
