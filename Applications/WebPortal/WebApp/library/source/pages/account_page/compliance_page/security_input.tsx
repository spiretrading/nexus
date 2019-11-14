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
            <div>{'Edit Symbols'}</div>
            <div>{'Search box'}</div>
            <div>{'Scrolly box'}</div>
            <div>{'trash and upload'}</div>
            <HLine color={'#e6e6e6'}/>
            <div>{'Submit changes~~~'}</div>
          </div>
        </div>
      </div>);
  }

  private toggleEditing(){
    console.log('TOGGLE!!!', this.state.isEditing);
    this.setState({isEditing: !this.state.isEditing});
  }

  private static readonly STYLE = {
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
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    },
    smallOptionsBox: {
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
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)',
      padding: '18px',
      marginTop: '20px',
      marginBottom: '20px'
    },
    label: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'no-wrap' as 'no-wrap'
    },
    textBox: {
      textOverflow: 'ellipsis' as 'ellipsis'
    }
  };
  private static readonly MIN_RANGE_VALUE = 0;
}