import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';

interface Properties {

  /** Determines the size of the element. */
  displaySize: DisplaySize;

  /** The currently selected rule mode. */
  value: Nexus.ComplianceRuleEntry.State;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler called when the selection changes. 
   * @param newMode - The new mode.
   */
  onChange?: (newMode: Nexus.ComplianceRuleEntry.State) => void;
}

/* Displays a drop down box that allows the user to select a rule mode.*/
export class RuleExecutionDropDown extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.readonly) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return RuleExecutionDropDown.STYLE.selectionBoxReadonlySmall;
        } else {
          return RuleExecutionDropDown.STYLE.selectionBoxReadonlyLarge;
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return RuleExecutionDropDown.STYLE.selectionBoxSmall;
        } else {
          return RuleExecutionDropDown.STYLE.selectionBoxLarge;
        }
      }
    })();
    return (
      <select onChange={this.onChange} value={this.props.value}
          disabled={this.props.readonly}
          className={css(RuleExecutionDropDown.EXTRA_STYLE.noHighlighting)}
          style={boxStyle}>
        <option value={Nexus.ComplianceRuleEntry.State.ACTIVE}>
          {'Active'}
        </option>
        <option value={Nexus.ComplianceRuleEntry.State.PASSIVE}>
          {'Passive'}
        </option>
        <option value={Nexus.ComplianceRuleEntry.State.DISABLED}>
          {'Disabled'}
        </option>
        <option value={Nexus.ComplianceRuleEntry.State.DELETED}>
          {'Delete'}
        </option>
      </select>);
  }

  private onChange(event: React.ChangeEvent<HTMLSelectElement>): void {
    this.props.onChange(parseInt(event.target.value));
  }

  private static readonly STYLE = {
    selectionBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '7px',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage:
        'url(resources/account_page/compliance_page/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none',
      cursor: 'pointer' as 'pointer',
      height: '34px',
      font: '400 14px Roboto',
      width: '100%',
      minWidth: '246px',
      maxWidth: '386px'
    },
    selectionBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '7px',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage:
        'url(resources/account_page/compliance_page/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none',
      cursor: 'pointer' as 'pointer',
      width: '200px',
      height: '34px',
      font: '400 14px Roboto'
    },
    selectionBoxReadonlySmall: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '7px',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#FFFFFF',
      backgroundImage:
        'url(resources/account_page/compliance_page/arrow-down-grey.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none',
      cursor: 'default' as 'default',
      height: '34px',
      font: '400 14px Roboto',
      width: '100%',
      minWidth: '246px',
      maxWidth: '386px'
    },
    selectionBoxReadonlyLarge: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '7px',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#FFFFFF',
      backgroundImage:
        'url(resources/account_page/compliance_page/arrow-down-grey.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none',
      cursor: 'default' as 'default',
      width: '200px',
      height: '34px',
      font: '400 14px Roboto'
    }
  };
  public static readonly EXTRA_STYLE = StyleSheet.create({
    noHighlighting: {
      ':focus': {
        outline: 0,
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
