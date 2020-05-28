import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../..';

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The first address line. */
  addressLineOne?: string;

  /** The second address line. */
  addressLineTwo?: string;

  /** The third address line. */
  addressLineThree?: string;

  /** Indicates if the component is readonly. */
  readonly?: boolean;
  
  /** Called when the value has changed.
   * @param addressLineOne - The first address line.
   * @param addressLineTwo - The second address line.
   * @param addressLineThree - The third address line.
   */
  onChange?: (addressLineOne: string,addressLineTwo: string,
    addressLineThree: string) => void;
}

interface State {
  displayValue: string,
  isEditing: boolean
}

/** Displays a address field. */
export class AddressField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    addressLineOne: '',
    addressLineTwo: '',
    addressLineThree:'',
    onChange: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      displayValue: this.props.addressLineOne + 
        '\n' + this.props.addressLineTwo + '\n' + this.props.addressLineThree,
      isEditing: false
    }
  }

  public render(): JSX.Element {
    const containerStyle = (() => {
      if(this.props.readonly) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return AddressField.STYLE.readonlySmall;
        } else if(this.props.displaySize === DisplaySize.MEDIUM) {
          return AddressField.STYLE.readonlyMedium;
        } else {
          return AddressField.STYLE.readonlyLarge;
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return AddressField.STYLE.containerSmall;
        } else if(this.props.displaySize === DisplaySize.MEDIUM) {
          return AddressField.STYLE.containerMedium;
        } else {
          return AddressField.STYLE.containerLarge;
        }
      }
    })();
    const heightOverride = (() => {
      if(this.props.readonly) {
        if(this.props.addressLineTwo === '' &&
            this.props.addressLineThree === '') {
          return AddressField.STYLE.singleLineHeight;
        } else if(this.props.addressLineThree === ''){
          return AddressField.STYLE.doubleLineHeight;
        } else {
          return AddressField.STYLE.tripleLineHeight;
        }
      } else {
        return null;
      }
    })();
    return (
      <textarea
        spellCheck={!this.props.readonly}
        rows={3}
        disabled={this.props.readonly}
        className={css(AddressField.EXTRA_STYLE.effects)}
        value={this.state.displayValue}
        style={{...containerStyle, ...heightOverride}}
        onBlur={this.onBlur}
        onFocus={this.onFocus}
        onChange={this.onChange}/>);
  }

  public onComponentDidMount() {
    this.updateDisplayValue();
  }

  public componentDidUpdate(prevProps: Properties) {
    if(!this.state.isEditing && (
        prevProps.addressLineOne !== this.props.addressLineOne ||
        prevProps.addressLineTwo !== this.props.addressLineTwo ||
        prevProps.addressLineThree !== this.props.addressLineThree)) {
      this.updateDisplayValue();
    }
  }

  private updateDisplayValue() {
    const displayText = (() => {
      if(this.props.addressLineTwo === '' &&
          this.props.addressLineThree === '') {
        return this.props.addressLineOne;
      } else if(this.props.addressLineThree === '') {
        return this.props.addressLineOne + '\n' + this.props.addressLineTwo;
      } else {
        return (this.props.addressLineOne +
          '\n' + this.props.addressLineTwo +
          '\n' + this.props.addressLineThree);
      }
    })();
    this.setState({displayValue: displayText});
  }

  private onBlur = () => {
    this.setState({isEditing: false});
    this.updateDisplayValue();
  }

  private onFocus = () => {
    this.setState({isEditing: true});
  }

  private onChange = (event: React.ChangeEvent<HTMLTextAreaElement>) => {
    const text = event.target.value.toString();
    if((text.split('\n').length <= 3)) {
      this.setState({displayValue: text});
      this.onSubmitChange(text);
    }
  }

  private onSubmitChange = (newValue: string) => {
    const addressLines = newValue.split('\n');
    const addressLineOne = addressLines[0] || '';
    const addressLineTwo = addressLines[1] || '';
    const addressLineThree = addressLines[2] || '';
    this.props.onChange(addressLineOne, addressLineTwo, addressLineThree);
  }

  public static readonly STYLE = {
    containerSmall: {
      boxSizing: 'border-box' as 'border-box',
      resize: 'none',
      height: '75px',
      font: '400 14px Roboto',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      lineHeight: '16px',
      paddingLeft: '10px',
      paddingTop: '7px',
      paddingBottom: '7px',
      backgroundColor: '#FFFFFF',
      flexGrow: 1,
      minWidth: '284px',
      width: '100%'
    } as React.CSSProperties,
    containerMedium: {
      boxSizing: 'border-box' as 'border-box',
      resize: 'none',
      height: '75px',
      minWidth: '284px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      lineHeight: '16px',
      paddingLeft: '10px',
      paddingTop: '7px',
      paddingBottom: '7px',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties,
    containerLarge: {
      boxSizing: 'border-box' as 'border-box',
      resize: 'none',
      height: '75px',
      minWidth: '350px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      lineHeight: '16px',
      paddingLeft: '10px',
      paddingTop: '7px',
      paddingBottom: '7px',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties,
    readonlySmall: {
      boxSizing: 'border-box' as 'border-box',
      resize: 'none',
      height: '75px',
      font: '400 14px Roboto',
      color: '#333333',
      border: '1px solid #FFFFFF',
      borderRadius: '1px',
      lineHeight: '16px',
      paddingLeft: '10px',
      paddingTop: '7px',
      paddingBottom: '7px',
      backgroundColor: '#FFFFFF',
      flexGrow: 1,
      minWidth: '284px',
      width: '100%'
    } as React.CSSProperties,
    readonlyMedium: {
      boxSizing: 'border-box' as 'border-box',
      resize: 'none',
      height: '75px',
      minWidth: '284px',
      border: '1px solid #FFFFFF',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      lineHeight: '16px',
      paddingLeft: '10px',
      paddingTop: '7px',
      paddingBottom: '7px',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties,
    readonlyLarge: {
      boxSizing: 'border-box' as 'border-box',
      resize: 'none',
      height: '75px',
      minWidth: '350px',
      border: '1px solid #FFFFFF',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      lineHeight: '16px',
      paddingLeft: '10px',
      paddingTop: '7px',
      paddingBottom: '7px',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties,
    singleLineHeight: {
      height: '34px'
    } as React.CSSProperties,
    doubleLineHeight: {
      height: '54px'
    } as React.CSSProperties,
    tripleLineHeight: {
      height: '75px'
    } as React.CSSProperties,
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    },
    inputStyle: {
      flexGrow: 1
    }
  });
}
