import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../../../';
import {CheckMarkButton} from '.';
import {DropDownButton} from '../../../components';

interface Properties {
}

interface State {
}

export class EntitlementsPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
    };
  }

  public render(): JSX.Element {
    return (
      <div>
        <CheckMarkButton size='16px' isReadOnly={false}/>
        <DropDownButton size='16px'/>
      </div>);
  }

  private static STYLE = StyleSheet.create({
    page: {
      backgroundColor: '#4B23A0'
    },
    inputBox: {
      width: '284px',
      padding: 0,
      height: '34px',
      borderColor: '#FFFFFF',
      backgroundColor: '#4B23A0',
      borderWidth: '0px 0px 1px 0px',
      color: '#FFFFFF',
      font: '300 16px Roboto',
      outline: 0,
      textAlign: 'center',
      borderRadius: 0,
      '::placeholder': {
        color: '#FFFFFF'
      },
      '::-moz-placeholder': {
        color: '#FFFFFF',
        opacity: 1
      },
      '::-ms-input-placeholder': {
        color: '#FFFFFF',
        opacity: 1
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none'
      },
      '::-webkit-autofill': {
        backgroundColor:  'none'
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
    errorMessage: {
      width: '100%',
      textAlign: 'center',
      font: '300 14px Roboto',
      height: '20px',
      color: '#FAEB96',
    }
  });
  //private staticLogo: HTMLObjectElement;
}
