import { css, StyleSheet } from 'aphrodite';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HLine } from '../../';
import { CountrySelectionBox } from '../../components';
import { FormEntry, PhotoField, PhotoFieldDisplayMode, RolesField, TextField }
  from '../account_page/profile_page';


interface Properties {
  
  /** The size of the element to display. */
  displaySize: DisplaySize;
  
  /** */
  countryDatabase: Nexus.CountryDatabase;
}

interface State {
  roles: Nexus.AccountRoles;
  identity: Nexus.AccountIdentity;
}

export class CreateAccountPage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      identity: new Nexus.AccountIdentity(),
      roles: new Nexus.AccountRoles()
    };
  }

  public render(): JSX.Element {
    const contentWidth = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return CreateAccountPage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return CreateAccountPage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return CreateAccountPage.STYLE.contentLarge;
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
            imageSource={this.state.identity.photoId}
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
          <Dali.VBoxLayout>
            <PhotoField
              displaySize={this.props.displaySize}
              displayMode={PhotoFieldDisplayMode.DISPLAY}
              imageSource={this.state.identity.photoId}
              scaling={1}/>
            <Dali.Padding size={CreateAccountPage.STANDARD_PADDING}/>
          </Dali.VBoxLayout>);
      } else {
        return null;
      }
    })();
    const buttonStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
          return CreateAccountPage.DYNAMIC_STYLE.buttonSmall;
      } else {
        return CreateAccountPage.DYNAMIC_STYLE.buttonLarge;
      }
    })();
    return (
      <div style={CreateAccountPage.STYLE.page}>
        <div style={CreateAccountPage.STYLE.pagePadding}/>
        <div style={contentWidth}>
          <Dali.VBoxLayout width='100%'>
            <Dali.Padding size='18px'/>
            <div style={CreateAccountPage.STYLE.headerStyler}>
              Create Account
            </div>
            <Dali.Padding size='60px'/>
            <Dali.HBoxLayout>
              {sidePanelPhoto}
              <Dali.Padding size={sidePanelPhotoPadding}/>
              <Dali.VBoxLayout width='100%'>
                {topPanelPhoto}
                <FormEntry name='First Name'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.state.identity.firstName}
                    placeholder={'First Name'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
                <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='Last Name'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.state.identity.lastName}
                    placeholder={'Last Name'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
                <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='Username'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={''}
                    placeholder={'Username'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
                 <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                 <FormEntry name='Role(s)'
                    displaySize={this.props.displaySize}>
                  <div style={CreateAccountPage.STYLE.rolesWrapper}>
                    <RolesField
                      roles={this.state.roles}/>
                    </div>
                </FormEntry>
                <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='Groups(s)'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={null}
                    placeholder={'Find Group'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
                <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='Email'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.state.identity.emailAddress}
                    placeholder={'Email'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
                <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='Address'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.state.identity.addressLineOne}
                    placeholder={'Address'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
                <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='City'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.state.identity.city}
                    placeholder={'City'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
                <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='Province/State'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.state.identity.province}
                    placeholder={'Province/State'}
                    displaySize={this.props.displaySize}/>
                </FormEntry>
              <Dali.Padding size={CreateAccountPage.SMALL_PADDING}/>
                <FormEntry name='Country'
                    displaySize={this.props.displaySize}>
                  <CountrySelectionBox
                    displaySize={this.props.displaySize}
                    countryDatabase={this.props.countryDatabase}
                    value={}/>
                </FormEntry>

              </Dali.VBoxLayout>
            </Dali.HBoxLayout>
                <Dali.Padding size={CreateAccountPage.STANDARD_PADDING}/>
                <HLine color={'#E6E6E6'}/>
                <Dali.Padding size={CreateAccountPage.STANDARD_PADDING}/>
                <div style={CreateAccountPage.STYLE.buttonBox}>
                  <button className={css(buttonStyle)}>
                      Create Account
                  </button>
                </div>
                <Dali.Padding size={CreateAccountPage.BOTTOM_PADDING}/>
          </Dali.VBoxLayout>
        </div>
        <div style={CreateAccountPage.STYLE.pagePadding}/>
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
      color: '#333333',
      font: '400 18px Roboto',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
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
      width: '122px',
      justifyContent: 'flex-start',
      alignItems: 'center'
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
    buttonBox: {
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      textAlign: 'center' as 'center'
    }
  };
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
      borderRadius: 1,
      cursor: 'pointer' as 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default' as 'default'
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
      borderRadius: 1,
      cursor: 'pointer' as 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default' as 'default'
      }
    }
  });
  private static readonly SMALL_PADDING = '18px';
  private static readonly STANDARD_PADDING = '30px';
  private static readonly BOTTOM_PADDING = '60px';

}
