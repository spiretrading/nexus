import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, PhotoField } from '../../..';
import { FormEntry, TextField } from '.';
import { RolesField } from './roles_field';
import { CommentBox } from '../comment_box';
import { HLine, CountrySelectionBox } from '../../../components';
import { VBoxLayout, Padding, HBoxLayout } from '../../../layouts';
import { DisplayMode } from './photo_field';
//import { SubmitButton } from '../submit_button';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The account identity to display. */
  identity: Nexus.AccountIdentity;

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
      if (this.props.displaySize === DisplaySize.SMALL) {
        return FormEntry.Orientation.VERTICAL;
      } else {
        return FormEntry.Orientation.HORIZONTAL;
      }
    })();
    const contentWidth = (() => {
      switch (this.props.displaySize) {
        case DisplaySize.SMALL:
          return ProfilePage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return ProfilePage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return ProfilePage.STYLE.contentLarge;
      }
    })();
    const sidePanelPhoto = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return (<PhotoField
          displaySize={this.props.displaySize}
          displayMode={DisplayMode.DISPLAY}
          imageSource={this.props.identity.photoId}
          scaling={1} />);
      }
    })();
    const sidePanelPhotoPadding = (() => {
      switch (this.props.displaySize) {
        case DisplaySize.SMALL:
          return 0;
        case DisplaySize.MEDIUM:
          return '30px';
        case DisplaySize.LARGE:
          return '100px';
      }
    })();
    const topPanelPhoto = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return (
          <VBoxLayout>
            <PhotoField
              displaySize={this.props.displaySize}
              displayMode={DisplayMode.DISPLAY}
              imageSource={this.props.identity.photoId}
              scaling={1} />
            <Padding size='30px' />
          </VBoxLayout>);
      } else {
        return null;
      }
    })();
    const formFooter = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return <HLine color={ProfilePage.LINE_COLOR} />;
      } else {
        return (null);
      }
    })();
    const formFooterPadding = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.STD_PADDING;
      } else {
        return (null);
      }
    })();
    const newPasswordBox = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.STYLE.passwordBoxSmall;
      } else {
        return ProfilePage.STYLE.passwordBoxLarge;
      }
    })();
    const submitAndStatusBox = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.STYLE.passwordBoxSmall;
      } else {
        return ProfilePage.STYLE.passwordBoxLarge;
      }
    })();
    const buttonStyle = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.DYNAMIC_STYLES.buttonSmall;
      } else {
        return ProfilePage.DYNAMIC_STYLES.buttonLarge;
      }
    })();
    const inputBoxStyle = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.DYNAMIC_STYLES.inputBoxSmall;
      } else {
        return ProfilePage.DYNAMIC_STYLES.inputBoxLarge;
      }
    })();

    const passwordStatusMessageInline = (() => {
      if (this.props.displaySize === DisplaySize.LARGE) {
        if (this.props.hasError) {
          return ProfilePage.STYLE.errorMessage;
        } else {
          return ProfilePage.STYLE.statusMessage;
        }
      } else {
        return ProfilePage.STYLE.hidden;
      }
    })();
    const passwordStatusMessageUnderneath = (() => {
      if (this.props.displaySize === DisplaySize.LARGE) {
        return ProfilePage.STYLE.hidden;
      } else {
        if (this.props.hasError) {
          return ProfilePage.STYLE.errorMessage;
        } else {
          return ProfilePage.STYLE.statusMessage;
        }
      }
    })();
    return (
      <div style={ProfilePage.STYLE.page}>
        <div style={ProfilePage.STYLE.fixedSizePadding} />
        <div style={contentWidth}>
          <VBoxLayout width='100%'>
            <Padding size='18px' />
            <div style={ProfilePage.STYLE.loginInfo}>Last Login</div>
            <Padding size='30px' />
            <div style={ProfilePage.STYLE.headerStyler}>
              Account Information
            </div>
            <Padding size='30px' />
            <HBoxLayout>
              {sidePanelPhoto}
              <Padding size={sidePanelPhotoPadding} />
              <VBoxLayout width='100%' >
                {topPanelPhoto}
                <FormEntry name='First Name'
                  orientation={orientation}>
                  <TextField
                    value='Frodo'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Last Name'
                  orientation={orientation}>
                  <TextField
                    value='Baggins'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Username'
                  orientation={orientation}>
                  <TextField
                    value='frodo_of_the_nine_fingers'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Role(s)'
                  orientation={orientation}>
                  <div id='WRAPPER' style={ProfilePage.STYLE.rolesWrapper}>
                    <RolesField roles={this.props.roles} />
                  </div>
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Groups(s)'
                  orientation={orientation}>
                  <TextField
                    value='shire_office'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Regestration Date'
                  orientation={orientation}>
                  <TextField
                    value='PUT A GOOD VALUE HERE'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='ID Number'
                  orientation={orientation}>
                  <TextField
                    value='SR68'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Email'
                  orientation={orientation}>
                  <TextField
                    value='frodo@bagend.nz'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Address'
                  orientation={orientation}>
                  <TextField
                    value='56 Bag End'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='City'
                  orientation={orientation}>
                  <TextField
                    value='Hobbiton'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Province/State'
                  orientation={orientation}>
                  <TextField
                    value='Westfarthing'
                    displaySize={this.props.displaySize}
                    disabled />
                </FormEntry>
                <Padding size={ProfilePage.LINE_PADDING} />
                <HLine color='#E6E6E6' />
                <Padding size={ProfilePage.LINE_PADDING} />
                <FormEntry name='Country'
                  orientation={orientation}>
                  <CountrySelectionBox
                    readonly
                    displaySize={this.props.displaySize}
                    value={Nexus.DefaultCountries.AU}
                    onChange={() => { }}
                    countryDatabase={this.state.countryDatabase} />
                </FormEntry>
                <Padding size={ProfilePage.STD_PADDING} />
                {formFooter}
                <Padding size={formFooterPadding} />
              </VBoxLayout>
            </HBoxLayout>
            <Padding size={ProfilePage.STD_PADDING} />
            <CommentSubmitBox displaySize={this.props.displaySize}/>
            <HLine color='#E6E6E6' />
            <Padding size={ProfilePage.STD_PADDING} />
            <ChangePasswordBox displaySize={this.props.displaySize}
              onPasswordSubmit={() => { }}
            />
            <Padding size={ProfilePage.BOTTOM_PADDING} />
          </VBoxLayout>
        </div>
        <div style={ProfilePage.STYLE.fixedSizePadding} />
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
    loginInfo: {
      color: '#000000',
      font: '400 14px Roboto'
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
    fixedSizePadding: {
      width: '30px'
    },
    tinyPadding: {
      width: '100%',
      height: '18px'
    },
    mediumPadding: {
      width: '100%',
      height: '30px'
    },
    rolesWrapper: {
      marginLeft: '11px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '34px',
      justifyContent: 'flex-start',
      alignItems: 'center'
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
  private static DYNAMIC_STYLES = StyleSheet.create({
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
    },
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
  private static readonly LINE_PADDING = '14px';
  private static readonly STD_PADDING = '30px';
  private static readonly BOTTOM_PADDING = '60px';
  private static readonly LINE_COLOR = '#E6E6E6';
  private passwordInputField: HTMLInputElement;
  private confirmPasswordInputField: HTMLInputElement;
}

interface CommentBoxProp {
  displaySize: DisplaySize;
  isSubmitEnabled: boolean;
  submitStatus: string;
  hasError: boolean;
  onSubmit: () => {};
}

class CommentSubmitBox extends React.Component<CommentBoxProp> {
  public static readonly defaultProps = {
    isSubmitEnabled: false,
    submitStatus: '',
    hasError: false,
    onSubmit: () => { }
  };

  public render(): JSX.Element {
    const boxStyle = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return CommentSubmitBox.STYLE.hidden;
      } else {
        if (this.props.hasError) {
          return CommentSubmitBox.STYLE.errorMessage;
        } else {
          return CommentSubmitBox.STYLE.statusMessage;
        }
      }
    })();
    const statusMessageInline = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return CommentSubmitBox.STYLE.hidden;
      } else {
        if (this.props.hasError) {
          return CommentSubmitBox.STYLE.errorMessage;
        } else {
          return CommentSubmitBox.STYLE.statusMessage;
        }
      }
    })();
    const statusMessageUnderneath = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        if (this.props.hasError) {
          return CommentSubmitBox.STYLE.errorMessage;
        } else {
          return CommentSubmitBox.STYLE.statusMessage;
        }
      } else {
        return CommentSubmitBox.STYLE.hidden;
      }
    })();
    return (<div>
      <div style={CommentSubmitBox.STYLE.headerStyler}>User Notes</div>
      <Padding size={CommentSubmitBox.STD_PADDING} />
      <CommentBox comment='boo' />
      <Padding size={CommentSubmitBox.STD_PADDING} />
      <div id='NEEDS STYLE!!!!!!!!!'>
        <div style={CommentSubmitBox.STYLE.filler} />
        <div style={{ ...boxStyle, ...statusMessageInline }}>
          BEEP
                <div style={CommentSubmitBox.STYLE.passwordButtonPadding} />
        </div>
        <SubmitButton label='Save Password'
          displaySize={this.props.displaySize} />
        <div style={statusMessageUnderneath}>
          <div style={CommentSubmitBox.STYLE.tinyPadding} />
          BEEP
              </div>
      </div>
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
    loginInfo: {
      color: '#000000',
      font: '400 14px Roboto'
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
    fixedSizePadding: {
      width: '30px'
    },
    tinyPadding: {
      width: '100%',
      height: '18px'
    },
    mediumPadding: {
      width: '100%',
      height: '30px'
    },
    rolesWrapper: {
      marginLeft: '11px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '34px',
      justifyContent: 'flex-start',
      alignItems: 'center'
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
  private static readonly LINE_PADDING = '14px';
  private static readonly STD_PADDING = '30px';
  private static readonly BOTTOM_PADDING = '60px';
  private static readonly LINE_COLOR = '#E6E6E6';
  private passwordInputField: HTMLInputElement;
  private confirmPasswordInputField: HTMLInputElement;
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
    const newPasswordBox = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return ChangePasswordBox.STYLE.passwordBoxSmall;
      } else {
        return ChangePasswordBox.STYLE.passwordBoxLarge;
      }
    })();
    const inputBoxStyle = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return ChangePasswordBox.DYNAMIC_STYLES.inputBoxSmall;
      } else {
        return ChangePasswordBox.DYNAMIC_STYLES.inputBoxLarge;
      }
    })();
    const passwordStatusMessageInline = (() => {
      if (this.props.displaySize === DisplaySize.LARGE) {
        if (this.props.hasPasswordError) {
          return ChangePasswordBox.STYLE.errorMessage;
        } else {
          return ChangePasswordBox.STYLE.statusMessage;
        }
      } else {
        return ChangePasswordBox.STYLE.hidden;
      }
    })();
    const passwordStatusMessageUnderneath = (() => {
      if (this.props.displaySize === DisplaySize.LARGE) {
        return ChangePasswordBox.STYLE.hidden;
      } else {
        if (this.props.hasPasswordError) {
          return ChangePasswordBox.STYLE.errorMessage;
        } else {
          return ChangePasswordBox.STYLE.statusMessage;
        }
      }
    })();
    return (<div>
      <div style={ChangePasswordBox.STYLE.headerStyler}>Change Password</div>
      <Padding size={ChangePasswordBox.STD_PADDING} />
      <div style={newPasswordBox}>
        <input type='password' placeholder='New Password'
          autoComplete='off'
          className={css(inputBoxStyle)}
          disabled={false}
          onFocus={() => this.passwordInputField.placeholder = ''}
          onBlur={() =>
            this.passwordInputField.placeholder = 'New Password'}
          ref={(ref) => this.passwordInputField = ref} />
        <div style={ChangePasswordBox.STYLE.passwordPadding} />
        <input type='password' placeholder='Confirm New Password'
          autoComplete='off'
          className={css(inputBoxStyle)}
          disabled={true}
          onFocus={() => this.confirmPasswordInputField.placeholder = ''}
          onBlur={() =>
            this.confirmPasswordInputField.placeholder
            = 'Confirm New Password'}
          ref={(ref) => this.confirmPasswordInputField = ref} />
        <div style={ChangePasswordBox.STYLE.filler} />
        <div style={passwordStatusMessageInline}>
          BEEP
            </div>
        <div style={ChangePasswordBox.STYLE.passwordButtonPadding} />
        <SubmitButton label='Save Password'
          displaySize={this.props.displaySize} />
      </div>
      <div style={ChangePasswordBox.STYLE.mediumPadding} />
      <div style={{
        ...passwordStatusMessageUnderneath,
        ...newPasswordBox
      }}>
        BEEP
      <div style={ChangePasswordBox.STYLE.filler} />
      </div>
    </div>);
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
    fixedSizePadding: {
      width: '30px'
    },
    tinyPadding: {
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
  private static DYNAMIC_STYLES = StyleSheet.create({
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
  private static readonly LINE_PADDING = '14px';
  private static readonly STD_PADDING = '30px';
  private static readonly BOTTOM_PADDING = '60px';
  private static readonly LINE_COLOR = '#E6E6E6';
  private passwordInputField: HTMLInputElement;
  private confirmPasswordInputField: HTMLInputElement;
}

//BUTTON!!!!!
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
      if (this.props.displaySize === DisplaySize.SMALL) {
        return SubmitButton.DYNAMIC_STYLES.buttonSmall;
      } else {
        return SubmitButton.DYNAMIC_STYLES.buttonLarge;
      }
    })();
    return (
      <button className={css(buttonStyle)}
        disabled={!this.props.isSubmitEnabled}
        onClick={this.props.onClick}>
        {this.props.label}
      </button>);
  }
  private static DYNAMIC_STYLES = StyleSheet.create({
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
