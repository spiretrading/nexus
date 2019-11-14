import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { HLine } from '../../../components';
import { ComplianceValue } from 'nexus';


interface Properties {
  displaySize: DisplaySize;

  value?: ComplianceValue.Type.SECURITY | ComplianceValue.Type.LIST;
}

interface State {
  isEditing: boolean;
}

export class SecurityInput extends React.Component<Properties, State>{
  constructor(props: Properties) {
    super(props);
    this.state = {
      isEditing: false
    }
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return SecurityInput.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const shadowBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.boxShadowSmall;
      } else {
        return SecurityInput.STYLE.boxShadowBig;
      }
    })();
    const optionsBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.smallOptionsBox;
      } else {
        return SecurityInput.STYLE.bigOptionsBox;
      }
    })();
    const selectedSecuritiesBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.STYLE.scrollBoxSmall;
      } else {
        return SecurityInput.STYLE.scrollBoxBig;
      }
    })();
    return(
      <div>
        <input
          style={SecurityInput.STYLE.textBox}
          value ={'After how many characters do you do the ...'}
          onFocus={this.toggleEditing.bind(this)}
          onClick={this.toggleEditing.bind(this)}/>
        <div style={visibility}>
          <div style={SecurityInput.STYLE.overlay}/>
          <div style={shadowBox}/>
          <div style={optionsBox}>
            <div style={SecurityInput.STYLE.header}>
              <div style={SecurityInput.STYLE.headerText}>
                {SecurityInput.MODAL_HEADER}
              </div>
              <img src={'resources/account_page/compliance_page/close.svg'}
                height='20px'
                width='20px'/>
            </div>
            <input style={SecurityInput.STYLE.findSymbolBox}
              placeholder={SecurityInput.PLACEHOLDER_TEXT}
              value={''}/>
            <div style={selectedSecuritiesBox}/>
            <div style={SecurityInput.STYLE.iconBoxBig}/>
            <HLine color={'#e6e6e6'}/>
            <div style={SecurityInput.STYLE.buttonWrapper}>
              <button style={SecurityInput.STYLE.button}>
                {'Submit Changes'}
              </button>
            </div>
          </div>
        </div>
      </div>);
  }

  private toggleEditing(){
    console.log('TOGGLE!!!', this.state.isEditing);
    this.setState({isEditing: !this.state.isEditing});
  }

  private static readonly STYLE = {
    textBox: {
      textOverflow: 'ellipsis' as 'ellipsis'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    overlay: {
      boxSizing: 'border-box' as 'border-box',
      top: '0px',
      left: '0px',
      position: 'fixed' as 'fixed',
      width: '100%',
      height: '100%',
      backgroundColor: '#FFFFFF',
      opacity: 0.9
    },
    boxShadowSmall: {
      boxSizing: 'border-box' as 'border-box', 
      opacity: 0.4,
      display: 'block',
      boxShadow: '0px 0px 6px #000000',
      position: 'absolute' as 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '282px',
      height: '100%',
      top: '0%',
      right: '0%'
    },
    boxShadowBig: {
      boxSizing: 'border-box' as 'border-box',
      opacity: 0.4,
      boxShadow: '0px 0px 6px #000000',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '559px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    },
    smallOptionsBox: { //uhhhhh as column and center the thing
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '282px',
      height: '100%',
      top: '0%',
      right: '0%',
      padding: '18px'
    },
    bigOptionsBox: {
      opacity: 1,
      boxSizing: 'border-box' as 'border-box',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '559px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)',
      padding: '18px',
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      height: '20px',
      marginBottom: '30px'
    },
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1
    },
    findSymbolBox: {
      width: '100%',
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      height: '34px',
      paddingLeft: '10px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '18px'
    },
    scrollBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '30px'
    },
    scrollBoxBig: {
      boxSizing: 'border-box' as 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '30px'
    },
    iconBoxBig: {
      height: '16px',
      width: '100%',
      marginBottom: '30px',
      backgroundColor: '#ff0090'
    },
    button: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#4B23A0',
      color: '#FFFFFF',
      border: '1px solid #4B23A0',
      borderRadius: '1px',
      marginTop: '30px',
      font: '400 16px Roboto',
    },
    buttonWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
    }
  };
  private static readonly MODAL_HEADER = 'Edit Symbols';
  private static readonly PLACEHOLDER_TEXT = 'Find symbol here';
}