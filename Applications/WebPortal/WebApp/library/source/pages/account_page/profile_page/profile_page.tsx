import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CountrySelectionBox, DisplaySize, HBoxLayout, HLine, Padding,
  PhotoField, VBoxLayout } from '../../..';
import { CommentBox } from '../comment_box';
import { FormEntry, PhotoFieldDisplayMode, RolesField, TextField } from '.';

interface Properties {

  /** The account name. */
  account: Beam.DirectoryEntry;

  /** The date of the regestration. */
  registration: Beam.DateTime;

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The account identity to display. */
  identity: Nexus.AccountIdentity;

  /** The name of the group the account belongs to. */
  group: Beam.DirectoryEntry;

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** Whether the save changes button can be clicked. */
  isSubmitEnabled?: boolean;

  /** The status of the submission. */
  submitStatus?: string;

  /** Whether an error occurred. */
  hasError?: boolean;

  /** Indicates the profile is being submitted. */
  onSubmit?: () => void;

  /** Whether the password button can be clicked. */
  isPasswordSubmitEnabled?: boolean;

  /** The status of the password submission. */
  submitPasswordStatus?: string;

  /** Whether an error occurred submitting the password. */
  hasPasswordError?: boolean;

  /** Indicates the password has been updated. */
  onSubmitPassword?: (password: string) => void;
}

interface State {
  countryDatabase: Nexus.CountryDatabase;
}

/** Displays an account's profile page. */
export class ProfilePage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    isSubmitEnabled: false,
    submitStatus: '',
    hasError: false,
    onSubmit: () => { },
    isPasswordSubmitEnabled: false,
    submitPasswordStatus: '',
    hasPasswordError: false,
    onPasswordSubmit: () => { }
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      countryDatabase: Nexus.buildDefaultCountryDatabase()
    };
  }

  public render(): JSX.Element {
    const orientation = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return FormEntry.Orientation.VERTICAL;
      } else {
        return FormEntry.Orientation.HORIZONTAL;
      }
    })();
    const contentWidth = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return ProfilePage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return ProfilePage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return ProfilePage.STYLE.contentLarge;
      }
    })();
    const sidePanelPhoto = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return (
          <PhotoField
            displaySize={this.props.displaySize}
            displayMode={PhotoFieldDisplayMode.DISPLAY}
            imageSource={this.props.identity.photoId}
            scaling={1}/>);
      }
    })();
    const sidePanelPhotoPadding = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return 0;
        case DisplaySize.MEDIUM:
          return '30px';
        case DisplaySize.LARGE:
          return '100px';
      }
    })();
    const topPanelPhoto = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <VBoxLayout>
            <PhotoField
              displaySize={this.props.displaySize}
              displayMode={PhotoFieldDisplayMode.DISPLAY}
              imageSource={this.props.identity.photoId}
              scaling={1}/>
            <Padding size='30px'/>
          </VBoxLayout>);
      } else {
        return null;
      }
    })();
    const formFooter = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return <HLine color={ProfilePage.LINE_COLOR}/>;
      } else {
        return (null);
      }
    })();
    const formFooterPaddingSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.STANDARD_PADDING;
      } else {
        return 0;
      }
    })();
    return (
      <div style={ProfilePage.STYLE.page}>
        <div style={ProfilePage.STYLE.pagePadding}/>
        <div style={contentWidth}>
          <VBoxLayout width='100%'>
            <Padding size='18px'/>
            <div style={ProfilePage.STYLE.lastLoginBox}>
              {this.props.identity.lastLoginTime.toJson.toString()}
            </div>
            <Padding size={ProfilePage.STANDARD_PADDING}/>
            <div style={ProfilePage.STYLE.headerStyler}>
              Account Information
            </div>
            <Padding size={ProfilePage.STANDARD_PADDING}/>
            <HBoxLayout>
              {sidePanelPhoto}
              <Padding size={sidePanelPhotoPadding}/>
              <VBoxLayout width='100%'>
                {topPanelPhoto}
                <FormEntry name='First Name'
                  orientation={orientation}>
                  <TextField
                    value={this.props.identity.firstName}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Last Name'
                  orientation={orientation}>
                  <TextField
                    value={this.props.identity.lastName}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Username'
                  orientation={orientation}>
                  <TextField
                    value={this.props.account.name.toString()}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Role(s)'
                  orientation={orientation}>
                  <div id='WRAPPER' style={ProfilePage.STYLE.rolesWrapper}>
                    <RolesField roles={this.props.roles}/>
                  </div>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Groups(s)'
                  orientation={orientation}>
                  <TextField value={this.props.group.name.toString()}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Registration Date'
                  orientation={orientation}>
                  <TextField displaySize={this.props.displaySize}
                    value='04/13/2019'
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='ID Number'
                  orientation={orientation}>
                  <TextField
                    value='SR68'
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Email'
                  orientation={orientation}>
                  <TextField
                    value='frodo@bagend.nz'
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Address'
                  orientation={orientation}>
                  <TextField
                    value='56 Bag End'
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='City'
                  orientation={orientation}>
                  <TextField
                    value='Hobbiton'
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Province/State'
                  orientation={orientation}>
                  <TextField
                    value='Westfarthing'
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Country'
                  orientation={orientation}>
                  <CountrySelectionBox
                    readonly
                    displaySize={this.props.displaySize}
                    value={Nexus.DefaultCountries.AU}
                    onChange={() => { }}
                    countryDatabase={this.state.countryDatabase}/>
                </FormEntry>
                <Padding size={ProfilePage.STANDARD_PADDING}/>
                {formFooter}
                <Padding size={formFooterPaddingSize}/>
              </VBoxLayout>
            </HBoxLayout>
            <SubmitCommentBox displaySize={this.props.displaySize}
              hasError={this.props.hasError}
              submitStatus={this.props.submitStatus}
              onSubmit={() => { }}/>
            <Padding size={ProfilePage.STANDARD_PADDING}/>
            <HLine color={ProfilePage.LINE_COLOR}/>
            <Padding size={ProfilePage.STANDARD_PADDING}/>
            <ChangePasswordBox displaySize={this.props.displaySize}
              hasPasswordError={this.props.hasPasswordError}
              submitPasswordStatus={this.props.submitPasswordStatus}
              onPasswordSubmit={() => { }}/>
            <Padding size={ProfilePage.BOTTOM_PADDING}/>
          </VBoxLayout>
        </div>
        <div style={ProfilePage.STYLE.pagePadding}/>
      </div>);
  }

  private static readonly STYLE = {
    page: {
      height: '100%',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      overflowY: 'auto' as 'auto'
    },
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    headerStyler: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      wdith: '100%'
    },
    lastLoginBox: {
      color: '#000000',
      font: '400 14px Roboto'
    },
    contentSmall: {
      flexGrow: 1,
      maxWidth: '424px'
    },
    contentMedium: {
      width: '732px'
    },
    contentLarge: {
      width: '1000px'
    },
    pagePadding: {
      width: '30px'
    },
    rolesWrapper: {
      marginLeft: '11px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '34px',
      justifyContent: 'flex-start',
      alignItems: 'center'
    }
  };
  private static readonly LINE_PADDING = '14px';
  private static readonly STANDARD_PADDING = '30px';
  private static readonly BOTTOM_PADDING = '60px';
  private static readonly LINE_COLOR = '#E6E6E6';
}

interface SubmitCommentBoxProperties  {
  displaySize: DisplaySize;
  isSubmitEnabled: boolean;
  submitStatus: string;
  hasError: boolean;
  onSubmit: () => void;
}

class SubmitCommentBox extends React.Component<SubmitCommentBoxProperties > {
  public static readonly defaultProps = {
    isSubmitEnabled: false,
    submitStatus: '',
    hasError: false,
    onSubmit: () => { }
  };

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SubmitCommentBox.STYLE.stackedStatusBox;
      } else {
        return SubmitCommentBox.STYLE.inlineStatusBox;
      }
    })();
    const statusMessageInline = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SubmitCommentBox.STYLE.hidden;
      } else {
        if(this.props.hasError) {
          return SubmitCommentBox.STYLE.errorMessage;
        } else {
          return SubmitCommentBox.STYLE.statusMessage;
        }
      }
    })();
    const statusMessageFooter = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.props.hasError) {
          return SubmitCommentBox.STYLE.errorMessage;
        } else {
          return SubmitCommentBox.STYLE.statusMessage;
        }
      } else {
        return SubmitCommentBox.STYLE.hidden;
      }
    })();
    return (
      <VBoxLayout>
        <div style={SubmitCommentBox.STYLE.headerStyler}>User Notes</div>
        <Padding size={SubmitCommentBox.STANDARD_PADDING}/>
        <CommentBox comment=''/>
        <Padding size={SubmitCommentBox.STANDARD_PADDING}/>
        <div style={boxStyle}>
          <div style={SubmitCommentBox.STYLE.filler}/>
          <div style={{ ...boxStyle, ...statusMessageInline}}>
            {this.props.submitStatus}
            <div style={SubmitCommentBox.STYLE.passwordButtonPadding}/>
          </div>
          <SubmitButton label='Save Changes'
            displaySize={this.props.displaySize}/>
          <div style={statusMessageFooter}>
            <div style={SubmitCommentBox.STYLE.smallPadding}/>
            {this.props.submitStatus}
          </div>
        </div>
      </VBoxLayout>);
  }
  private static readonly STYLE = {
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    headerStyler: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      wdith: '100%'
    },
    errorMessage: {
      color: '#E63F44',
      font: '400 14px Roboto'
    },
    statusMessage: {
      color: '#36BB55',
      font: '400 14px Roboto'
    },
    filler: {
      flexGrow: 1
    },
    smallPadding: {
      width: '100%',
      height: '18px'
    },
    mediumPadding: {
      width: '100%',
      height: '30px'
    },
    inlineStatusBox: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center'
    },
    stackedStatusBox: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center'
    },
    passwordButtonPadding: {
      height: '30px',
      width: '30px'
    }
  };
  private static readonly STANDARD_PADDING = '30px';
}

interface ChangePassBoxProperties {
  displaySize: DisplaySize;
  isPasswordSubmitEnabled: boolean;
  submitPasswordStatus: string;
  hasPasswordError: boolean;
  onSubmitPassword?: (password: string) => void;
}

/** Displays a slider that changes a value. */
class ChangePasswordBox extends React.Component<ChangePassBoxProperties> {
  public static readonly defaultProps = {
    isPasswordSubmitEnabled: false,
    submitPasswordStatus: '',
    hasPasswordError: false,
    onPasswordSubmit: () => { }
  };

  public render(): JSX.Element {
    const changePasswordBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePasswordBox.STYLE.passwordBoxSmall;
      } else {
        return ChangePasswordBox.STYLE.passwordBoxLarge;
      }
    })();
    const inputBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePasswordBox.DYNAMIC_STYLE.inputBoxSmall;
      } else {
        return ChangePasswordBox.DYNAMIC_STYLE.inputBoxLarge;
      }
    })();
    const messageInline = (() => {
      if(this.props.displaySize === DisplaySize.LARGE) {
        if(this.props.hasPasswordError) {
          return ChangePasswordBox.STYLE.errorMessage;
        } else {
          return ChangePasswordBox.STYLE.statusMessage;
        }
      } else {
        return ChangePasswordBox.STYLE.hidden;
      }
    })();
    const inlineStatusBox = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.LARGE:
          return (
            <div style={{...messageInline,
                ...ChangePasswordBox.STYLE.fillerMessage}}>
              {this.props.submitPasswordStatus}
            </div>);
        case DisplaySize.MEDIUM:
          return <div style={ChangePasswordBox.STYLE.filler}/>;
        case DisplaySize.SMALL:
          return <div style={ChangePasswordBox.STYLE.filler}/>;
      }
    })();
    const messageUnderneath = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.LARGE:
          return ChangePasswordBox.STYLE.hidden;
        case DisplaySize.MEDIUM:
          if(this.props.hasPasswordError) {
            return ChangePasswordBox.STYLE.errorMessage;
          } else {
            return ChangePasswordBox.STYLE.statusMessage;
          }
        case DisplaySize.SMALL:
          if(this.props.hasPasswordError) {
            return {...ChangePasswordBox.STYLE.errorMessage,
              ...ChangePasswordBox.STYLE.passwordBoxSmall};
          } else {
            return {...ChangePasswordBox.STYLE.statusMessage,
              ...ChangePasswordBox.STYLE.passwordBoxSmall};
          }

      }
    })();
    return (
      <VBoxLayout>
        <div style={ChangePasswordBox.STYLE.headerStyler}>Change Password</div>
        <Padding size={ChangePasswordBox.STANDARD_PADDING}/>
        <div style={changePasswordBox}>
          <input type='password' placeholder='New Password'
            autoComplete='off'
            className={css(inputBoxStyle)}
            disabled={false}
            onFocus={() => this.passwordInputField.placeholder = ''}
            onBlur={() =>
              this.passwordInputField.placeholder = 'New Password'}
            ref={(ref) => this.passwordInputField = ref}/>
          <div style={ChangePasswordBox.STYLE.passwordPadding}/>
          <input type='password' placeholder='Confirm New Password'
            autoComplete='off'
            className={css(inputBoxStyle)}
            disabled={true}
            onFocus={() => this.confirmPasswordInputField.placeholder = ''}
            onBlur={() =>
              this.confirmPasswordInputField.placeholder
              = 'Confirm New Password'}
            ref={(ref) => this.confirmPasswordInputField = ref}/>
          {inlineStatusBox}
          <SubmitButton label='Save Password'
            displaySize={this.props.displaySize}/>
        </div>
        <div style={messageUnderneath}>
          <div style={ChangePasswordBox.STYLE.smallPadding}/>
          {this.props.submitPasswordStatus}
        </div>
      </VBoxLayout>);
  }
  private static readonly STYLE = {
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    headerStyler: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      width: '100%'
    },
    errorMessage: {
      color: '#E63F44',
      font: '400 14px Roboto'
    },
    statusMessage: {
      color: '#36BB55',
      font: '400 14px Roboto'
    },
    filler: {
      flexGrow: 1,
      height: '30px'
    },
    fillerMessage: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      flexGrow: 1,
      textAlign: 'right' as 'right',
      marginRight: '30px',
      justifyContent: 'flex-end' as 'flex-end'
    },
    smallPadding: {
      width: '100%',
      height: '18px'
    },
    mediumPadding: {
      width: '100%',
      height: '30px'
    },
    passwordBoxLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center'
    },
    passwordBoxSmall: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center'
    },
    passwordPadding: {
      height: '18px',
      width: '18px'
    },
    passwordButtonPadding: {
      height: '30px',
      width: '30px'
    }
  };
  private static DYNAMIC_STYLE = StyleSheet.create({
    inputBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      padding: 0,
      height: '34px',
      border: '1px solid #C8C8C8',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '300 16px Roboto',
      outline: 0,
      textAlign: 'center',
      borderRadius: 0,
      ':active': {
        border: '1px solid #684BC7'
      },
      '::placeholder': {
        color: '#8C8C8C'
      },
      '::-moz-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-input-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none'
      },
      '::-webkit-autofill': {
        backgroundColor: 'none'
      },
      '::-webkit-credentials-auto-fill-button': {
        visibility: 'hidden' as 'hidden',
        display: 'none !important',
        pointerEvents: 'none',
        height: 0,
        width: 0,
        margin: 0
      }
    },
    inputBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      width: '240px',
      padding: 0,
      height: '34px',
      border: '1px solid #C8C8C8',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '300 16px Roboto',
      outline: 0,
      textAlign: 'center',
      borderRadius: 0,
      ':active': {
        border: '1px solid #684BC7'
      },
      '::placeholder': {
        color: '#8C8C8C'
      },
      '::-moz-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-input-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none'
      },
      '::-webkit-autofill': {
        backgroundColor: 'none'
      },
      '::-webkit-credentials-auto-fill-button': {
        visibility: 'hidden' as 'hidden',
        display: 'none !important',
        pointerEvents: 'none',
        height: 0,
        width: 0,
        margin: 0
      }
    }
  });
  private static readonly STANDARD_PADDING = '30px';
  private passwordInputField: HTMLInputElement;
  private confirmPasswordInputField: HTMLInputElement;
}

interface ButtonProperties {
  onClick?: () => void;
  isSubmitEnabled?: boolean;
  label: string;
  displaySize: DisplaySize;
}

class SubmitButton extends React.Component<ButtonProperties, {}> {
  public static readonly defaultProps = {
    disabled: true,
    onSubmit: () => { }
  };

  public render(): JSX.Element {
    const buttonStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SubmitButton.DYNAMIC_STYLE.buttonSmall;
      } else {
        return SubmitButton.DYNAMIC_STYLE.buttonLarge;
      }
    })();
    return (
      <button className={css(buttonStyle)}
          disabled={!this.props.isSubmitEnabled}
          onClick={this.props.onClick}>
        {this.props.label}
      </button>);
  }
  private static DYNAMIC_STYLE = StyleSheet.create({
    buttonSmall: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C'
      }
    },
    buttonLarge: {
      margin: 0,
      padding: 0,
      boxSizing: 'border-box' as 'border-box',
      width: '200px',
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C'
      }
    }
  });
}
