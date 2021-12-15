import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';

interface Properties {

  /**
   * The type of directory entry being displayed.
   * Groups/directories have an option for whether a rule is consolidated which
   * does not appear for accounts.
   */
  entryType: Beam.DirectoryEntry.Type;

  /** Determines the size of the element. */
  displaySize: DisplaySize;

  /** The currently selected rule mode. */
  value: RuleExecutionDropDown.Mode;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /**
   * The event handler called when the selection changes. 
   * @param mode - The new mode.
   */
  onChange?: (mode: RuleExecutionDropDown.Mode) => void;
}

/* Displays a drop down box that allows the user to select a rule mode.*/
export class RuleExecutionDropDown extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

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
    if(this.props.entryType === Beam.DirectoryEntry.Type.ACCOUNT) {
      return (
        <select onChange={this.onChange} value={this.props.value.state}
            disabled={this.props.readonly}
            className={css(RuleExecutionDropDown.EXTRA_STYLE.noHighlighting)}
            style={boxStyle}>
          <option value={Nexus.ComplianceRuleEntry.State.ACTIVE}>
            Active
          </option>
          <option value={Nexus.ComplianceRuleEntry.State.PASSIVE}>
            Passive
          </option>
          <option value={Nexus.ComplianceRuleEntry.State.DISABLED}>
            Disabled
          </option>
          <option value={Nexus.ComplianceRuleEntry.State.DELETED}>
            Delete
          </option>
        </select>);
    }
    const value = (() => {
      switch(this.props.value.applicability) {
        case Nexus.ComplianceRuleSchema.Applicability.CONSOLIDATED:
          switch(this.props.value.state) {
            case Nexus.ComplianceRuleEntry.State.ACTIVE:
              return 0;
            case Nexus.ComplianceRuleEntry.State.PASSIVE:
              return 1;
            case Nexus.ComplianceRuleEntry.State.DISABLED:
              return 4;
            case Nexus.ComplianceRuleEntry.State.DELETED:
              return 5;
          }
        case Nexus.ComplianceRuleSchema.Applicability.PER_ACCOUNT:
          switch(this.props.value.state) {
            case Nexus.ComplianceRuleEntry.State.ACTIVE:
              return 2;
            case Nexus.ComplianceRuleEntry.State.PASSIVE:
              return 3;
            case Nexus.ComplianceRuleEntry.State.DISABLED:
              return 4;
            case Nexus.ComplianceRuleEntry.State.DELETED:
              return 5;
          }
      }
    })();
    return (
      <select onChange={this.onChange} value={value}
          disabled={this.props.readonly}
          className={css(RuleExecutionDropDown.EXTRA_STYLE.noHighlighting)}
          style={boxStyle}>
        <option value={0}>
          Consolidated Active
        </option>
        <option value={1}>
          Consolidated Passive
        </option>
        <option value={2}>
          Per Account Active
        </option>
        <option value={3}>
          Per Account Passive
        </option>
        <option value={4}>
          Disabled
        </option>
        <option value={5}>
          Delete
        </option>
      </select>);
  }

  private onChange = (event: React.ChangeEvent<HTMLSelectElement>): void => {
    if(this.props.entryType === Beam.DirectoryEntry.Type.ACCOUNT) {
      this.props.onChange({
        state: parseInt(event.target.value),
        applicability: Nexus.ComplianceRuleSchema.Applicability.CONSOLIDATED
      });
    } else {
      const selection = parseInt(event.target.value);
      const state = (() => {
        if(selection === 0 || selection === 2) {
          return Nexus.ComplianceRuleEntry.State.ACTIVE;
        } else if(selection === 1 || selection === 3) {
          return Nexus.ComplianceRuleEntry.State.PASSIVE;
        } else if(selection === 4) {
          return Nexus.ComplianceRuleEntry.State.DISABLED;
        } else if(selection === 5) {
          return Nexus.ComplianceRuleEntry.State.DELETED;
        }
        return Nexus.ComplianceRuleEntry.State.PASSIVE;
      })();
      const applicability = (() => {
        if(selection === 0 || selection === 1) {
          return Nexus.ComplianceRuleSchema.Applicability.CONSOLIDATED;
        } else if(selection === 2 || selection === 3) {
          return Nexus.ComplianceRuleSchema.Applicability.PER_ACCOUNT;
        }
        return Nexus.ComplianceRuleSchema.Applicability.CONSOLIDATED;
      })();
      this.props.onChange({
        state,
        applicability
      });
    }
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

export namespace RuleExecutionDropDown {

  /**
   * The mode is a pair of entry state as well as schema applicability. Allows
   * a user to specify both of them together instead of needing two independent
   * input fields.
   */
  export interface Mode {

    /** The rule's state. */
    state: Nexus.ComplianceRuleEntry.State;

    /** The rule's applicability. */
    applicability: Nexus.ComplianceRuleSchema.Applicability;
  }
}
